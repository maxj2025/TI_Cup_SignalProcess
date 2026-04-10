#include "bsp_system.h"

typedef struct {
    uint8_t  current_mode;  // 0: 测周法(输入捕获), 1: 测频法(外部中断)
    uint64_t count;         // 测频法脉冲计数
    uint32_t gate_start;    // 闸门起始时间 (ms)
    uint8_t  is_measuring;  // 测频法测量标志
} FreqControl_t;

static FreqControl_t FreqCtrl = {0};

/**
 * @brief 动态切换硬件配置
 * @param target_mode 0: 测周法 (TIM2), 1: 测频法 (EXTI)
 */
static void Freq_Hardware_Switch(uint8_t target_mode) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    if(target_mode == 0) { // 切换到：输入捕获 (TIM2_CH1)
        // 1. 关闭并清理外部中断
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
        HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);

        // 2. 复位并重启 TIM2 硬件
        HAL_TIM_IC_Stop(&htim2, TIM_CHANNEL_1);
        HAL_TIM_Base_Stop(&htim2);
        __HAL_RCC_TIM2_FORCE_RESET();
        for(volatile uint32_t i=0; i<500; i++); 
        __HAL_RCC_TIM2_RELEASE_RESET();

        // 3. 重新初始化定时器设置
        MX_TIM2_Init();

        // 4. 配置 PA5 为定时器复用模式，开启下拉以防空载干扰
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN; // 下拉：无信号时引脚稳定
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // 5. 开启定时器并清理初始标志
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        HAL_TIM_Base_Start(&htim2);
        HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_1);
        
    } else { // 切换到：外部中断 (EXTI)
        // 1. 释放定时器资源
        HAL_TIM_IC_Stop(&htim2, TIM_CHANNEL_1);
        HAL_TIM_Base_Stop(&htim2);

        // 2. 配置 PA5 为中断引脚，开启下拉
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN; 
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // 3. 清理并开启 NVIC 中断
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
        HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
        HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    }
}

/**
 * @brief 底层测周法获取原始频率值（支持无信号归零）
 * @return float 频率值。-1.0表示正在等待信号，0.0表示确定无信号
 */
static float Get_Capture_Freq(void) {
    // 检查是否发生了捕获事件 (硬件检测到了上升沿)
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC1) != RESET) {
        uint32_t Rise_Count = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1) + 1;
        
        // 清理标志位
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

        if (Rise_Count <= 1) return 0.0f;
        // 注意：60MHz 需要根据你的定时器实际频率修改
        return 60000000.0f / (float)Rise_Count;
    } 
    // 如果没有捕获，但定时器溢出了 (说明超过 0.4s 没信号)
    else if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        return 0.0f; // 频率太低或断开连接
    }

    return -1.0f; // 处于测量间歇期，暂不更新
}


/**
 * @brief 初始化频率测量模块
 */
void FreqMeasure_Init(void) {
    FreqCtrl.current_mode = 0; 
    Freq_Hardware_Switch(0);
}


void FreqMeasure_Count_Handler(uint16_t GPIO_Pin) {
    if(GPIO_Pin == GPIO_PIN_5 && FreqCtrl.current_mode == 1) {
        FreqCtrl.count++;
    }
}

/**
 * @brief 频率测量主逻辑处理函数
 */
void FreqMeasure_Process(Wave_Struct* Wave_P) {
    if (FreqCtrl.current_mode == 0) { // 【测周法】
        float temp = Get_Capture_Freq();
        
        if (temp >= 0) { // 只有在明确捕获到值或溢出归零时才更新
            Wave_P->Freq = temp;
        }

        // 高频切换逻辑：如果频率 > 11kHz，切换到测频法
        if (Wave_P->Freq > 11000.0f) {
            FreqCtrl.current_mode = 1;
            Freq_Hardware_Switch(1);
            FreqCtrl.is_measuring = 0;
        }
    } 
    else { // 【测频法】(闸门模式)
        if (!FreqCtrl.is_measuring) {
            FreqCtrl.count = 0;
            FreqCtrl.gate_start = HAL_GetTick();
            FreqCtrl.is_measuring = 1;
        } else {
            // 闸门时间 1000ms
            if (HAL_GetTick() - FreqCtrl.gate_start >= 1000) {
                Wave_P->Freq = (float)FreqCtrl.count; // 即使 count 为 0 也会正确赋值
                FreqCtrl.is_measuring = 0;

                // 低频/无信号切换逻辑：如果频率 < 9kHz，切回测周法
                if (Wave_P->Freq < 9000.0f) {
                    FreqCtrl.current_mode = 0;
                    Freq_Hardware_Switch(0);
                }
            }
        }
    }
}