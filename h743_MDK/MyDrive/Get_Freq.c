#include "bsp_system.h"

/* --- 私有宏定义 --- */
// H743 TIM2 频率 (假设 D2PCLK1 未分频或由倍频器补偿，通常为 240MHz)
// 务必与你的实际系统时钟树对齐，如果读数差2倍或4倍，就是这里的问题
#define TIM2_TICK_FREQ    60000000.0f  
#define SWITCH_HIGH_THR   11000.0f      // 切测频法阈值
#define SWITCH_LOW_THR    9000.0f       // 切测周法阈值
#define GATE_TIME_MS      1000          // 测频法闸门时间(ms)

typedef struct {
    uint8_t  current_mode;  
    uint64_t count;         
    uint32_t gate_start;    
    uint8_t  is_measuring;  
    uint8_t  first_frame;   // 丢弃首帧标志位
} FreqControl_t;

static FreqControl_t FreqCtrl = {0};

/**
 * @brief 动态切换硬件配置（结合了最严谨的硬件级彻底复位逻辑）
 */
static void Freq_Hardware_Switch(uint8_t target_mode) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if(target_mode == 0) { // 【切换到：测周法 (TIM2 输入捕获)】
        
        /* 1. 硬件级彻底关闭并清除外部中断 */
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
        HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);

        /* 2. 强制停止定时器，做硬件复位 (清除一切历史残留) */
        HAL_TIM_Base_Stop(&htim2);
        HAL_TIM_IC_Stop(&htim2, TIM_CHANNEL_1);
        
        __HAL_RCC_TIM2_FORCE_RESET();
        for(volatile uint32_t i = 0; i < 500; i++); // 空循环延时，等待硬件复位生效
        __HAL_RCC_TIM2_RELEASE_RESET();

        /* 3. 强制清除 HAL 库软状态机，并重新初始化 */
        htim2.State = HAL_TIM_STATE_RESET; 
        MX_TIM2_Init(); 
        htim2.State = HAL_TIM_STATE_READY;

        /* 4. 硬件级强制配置 PA5 为 TIM2_CH1 复用捕获模式 */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;        
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;          // 下拉，防止空载干扰
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; 
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;     
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        (void)GPIOA->IDR; // Dummy Read: 强制刷新 GPIO 寄存器，防止写入延迟

        /* 5. 启动定时器和捕获，带状态检查 */
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1 | TIM_FLAG_UPDATE); // 兜底清零
        
        if(HAL_TIM_Base_Start(&htim2) != HAL_OK) {
            Error_Handler();
        }
        if(HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_1) != HAL_OK) {
            Error_Handler();
        }

    } else { // 【切换到：测频法 (外部中断)】
        
        /* 1. 彻底停止定时器捕获 */
        HAL_TIM_Base_Stop(&htim2);
        HAL_TIM_IC_Stop(&htim2, TIM_CHANNEL_1);

        /* 2. 配置 PA5 为外部中断上升沿 */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN; 
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        (void)GPIOA->IDR; // Dummy Read

        /* 3. 清除残留，使能中断 */
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
        HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    }
    
    // 标记切换后的第一帧，无论是捕获还是计数，丢弃首帧杂乱数据
    FreqCtrl.first_frame = 1; 
}

/**
 * @brief 底层测周法：非阻塞获取频率值 (取代死延时的Get_Wave_Infor)
 */
static float Get_Capture_Freq(void) {
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC1) != RESET) {
        // 读取 CCR1 寄存器 (由硬件在上升沿自动锁存的值)
        uint32_t capture_val = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);
        
        // 立刻清除标志位
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

        if (FreqCtrl.first_frame) { 
            FreqCtrl.first_frame = 0;
            return -1.0f; // 丢弃切换模式后的第一条不稳定数据
        }

        if (capture_val == 0) return 0.0f;
        
        // 核心计算公式：Freq = 外部时钟 / 捕获计数值
        return (float)TIM2_TICK_FREQ / (float)(capture_val + 1);
    } 
    // 检查定时器是否溢出 (信号太慢或无信号)
    else if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        return 0.0f; 
    }

    return -1.0f; // 处于测量间隙
}

/**
 * @brief 初始化入口
 */
void FreqMeasure_Init(void) {
    FreqCtrl.current_mode = 0; 
    Freq_Hardware_Switch(0);
}

/**
 * @brief 中断计数回调 (放在 HAL_GPIO_EXTI_Callback 中)
 */
void FreqMeasure_Count_Handler(uint16_t GPIO_Pin) {
    if(GPIO_Pin == GPIO_PIN_5 && FreqCtrl.current_mode == 1) {
        if(FreqCtrl.first_frame) {
            FreqCtrl.first_frame = 0; // 丢弃刚切入中断瞬间可能存在的毛刺脉冲
            return;
        }
        FreqCtrl.count++;
    }
}

/**
 * @brief 主循环调度任务
 */
void FreqMeasure_Process(Wave_Struct* Wave_P) {
    if (FreqCtrl.current_mode == 0) {
        /* --- 测周法 --- */
        float temp = Get_Capture_Freq();
        if (temp >= 0) {
            Wave_P->Freq = temp;
            
            // 超过阈值，切到测频法
            if (Wave_P->Freq > SWITCH_HIGH_THR) {
                FreqCtrl.current_mode = 1;
                Freq_Hardware_Switch(1);
                FreqCtrl.is_measuring = 0;
            }
        }
    } 
    else {
        /* --- 测频法 (闸门计数) --- */
        if (!FreqCtrl.is_measuring) {
            FreqCtrl.count = 0;
            FreqCtrl.gate_start = HAL_GetTick();
            FreqCtrl.is_measuring = 1;
        } else {
            // 非阻塞判断 1000ms 闸门时间到达
            if (HAL_GetTick() - FreqCtrl.gate_start >= GATE_TIME_MS) {
                Wave_P->Freq = (float)FreqCtrl.count; 
                FreqCtrl.is_measuring = 0;

                // 低于阈值，切回测周法
                if (Wave_P->Freq < SWITCH_LOW_THR) {
                    FreqCtrl.current_mode = 0;
                    Freq_Hardware_Switch(0);
                }
            }
        }
    }
}