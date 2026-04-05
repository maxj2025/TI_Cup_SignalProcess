#include "Get_Freq.h"

static FreqControl_t FreqCtrl = {0};


static void Freq_Hardware_Switch(uint8_t target_mode) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    if(target_mode == 0) { // 切换到：输入捕获 (TIM2_CH1)
        // 1. 清理中断
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
        HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
        EXTI->PR1 = 0xFFFFFFFF;

        // 2. 复位 TIM2
        HAL_TIM_Base_Stop(&htim2);
        HAL_TIM_IC_Stop(&htim2, TIM_CHANNEL_1);
        __HAL_RCC_TIM2_FORCE_RESET();
        for(volatile uint32_t i=0; i<500; i++); 
        __HAL_RCC_TIM2_RELEASE_RESET();

        // 3. 重新初始化并配置 PA5 为 AF1
        MX_TIM2_Init();
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // 4. 启动硬件
        HAL_TIM_Base_Start(&htim2);
        HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_1);
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
        
    } else { // 切换到：外部中断 (EXTI)
        // 1. 释放定时器
        HAL_TIM_IC_Stop(&htim2, TIM_CHANNEL_1);
        HAL_TIM_Base_Stop(&htim2);

        // 2. 配置 PA5 为中断引脚
        GPIO_InitStruct.Pin = GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // 3. 启动 NVIC
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
        HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
        HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    }
}

/**
 * @brief 底层测周法获取原始值
 */
static float Get_Capture_Freq(void) {
    uint32_t Rise_Count = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1) + 1;
    if (Rise_Count <= 1) return 0; // 防止分母为0
    return 50000000.0f / (float)Rise_Count;
}

void FreqMeasure_Init(void) {
    FreqCtrl.current_mode = 0; 
    Freq_Hardware_Switch(0);
}

void FreqMeasure_Count_Handler(uint16_t GPIO_Pin) {
    if(GPIO_Pin == GPIO_PIN_5 && FreqCtrl.current_mode == 1) {
        FreqCtrl.count++;
    }
}

void FreqMeasure_Process(float *pFreq) {
    if (FreqCtrl.current_mode == 0) { // 测周法
        float temp = Get_Capture_Freq();
        if (temp > 0) *pFreq = temp;

        // 模式切换判断：高频阈值 11kHz
        if (*pFreq > 11000.0f) {
            FreqCtrl.current_mode = 1;
            Freq_Hardware_Switch(1);
            FreqCtrl.is_measuring = 0;
        }
    } 
    else { // 测频法 (闸门模式)
        if (!FreqCtrl.is_measuring) {
            FreqCtrl.count = 0;
            FreqCtrl.gate_start = HAL_GetTick();
            FreqCtrl.is_measuring = 1;
        } else {
            if (HAL_GetTick() - FreqCtrl.gate_start >= 1000) { // 1秒闸门
                *pFreq = (float)FreqCtrl.count;
                FreqCtrl.is_measuring = 0;

                // 模式切换判断：低频迟滞阈值 9kHz
                if (*pFreq < 9000.0f) {
                    FreqCtrl.current_mode = 0;
                    Freq_Hardware_Switch(0);
                }
            }
        }
    }
}