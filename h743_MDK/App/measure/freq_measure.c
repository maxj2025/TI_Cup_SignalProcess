/**
 * @file    freq_measure.c
 * @brief   频率测量实现 —— 基于 STM32H743 TIM2 纯硬件双模式
 * @note    引脚: PA5 (TIM2_CH1)
 * 低频: 测周法 (TIM2 输入捕获 + 从模式复位)
 * 高频: 测频法 (TIM2 外部时钟模式1 ETR计数，零中断)
 */

#include "freq_measure.h"
#include "tim.h"   
#include "gpio.h"


/* ================= 内部状态与宏 ================= */
#define FMODE_PERIOD 0
#define FMODE_COUNT  1

typedef struct {
    uint8_t  mode;          // 当前模式: 0=测周, 1=测频
    uint32_t gate_start_ms; // 测频闸门起始时间
    uint8_t  measuring;     // 测频过程标志位
    uint8_t  first_frame;   // 丢弃刚切换时的第一帧脏数据
} FreqCtx_t;

static FreqCtx_t g_freq = {0};

static void freq_hw_switch(uint8_t target_mode) {
    GPIO_InitTypeDef gpio = {0};
    g_freq.first_frame = 1;

    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);      
    HAL_TIM_Base_Stop(&htim2);
    HAL_TIM_IC_Stop(&htim2, TIM_CHANNEL_1);
    HAL_TIM_Base_DeInit(&htim2);

    __HAL_RCC_GPIOA_CLK_ENABLE();
    gpio.Pin       = GPIO_PIN_5;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_PULLDOWN;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &gpio);

    if (target_mode == FMODE_PERIOD) {
        MX_TIM2_Init(); 
        
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1 | TIM_FLAG_UPDATE);
        HAL_TIM_Base_Start(&htim2);
        HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_1);

    } else {
        htim2.Instance = TIM2;
        htim2.Init.Prescaler = 0;
        htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim2.Init.Period = 0xFFFFFFFF;
        htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        HAL_TIM_Base_Init(&htim2);

        TIM_SlaveConfigTypeDef sSlaveConfig = {0};
        sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
        sSlaveConfig.InputTrigger = TIM_TS_TI1FP1; 
        sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
        sSlaveConfig.TriggerFilter = 0;
        HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig);

        __HAL_TIM_SET_COUNTER(&htim2, 0); // 计数值清零
        HAL_TIM_Base_Start(&htim2);       // 开启硬件默默数脉冲
    }
}


static float freq_read_period(void) {
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC1) != RESET) {
        uint32_t cap = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

        if (g_freq.first_frame) { 
            g_freq.first_frame = 0; 
            return -1.0f; 
        }
        
        if (cap == 0) return 0.0f;
        return (float)((double)TIM2_TICK_FREQ / (double)(cap + 1));
    }
    return -1.0f;  
}

/* ================= 公共 API 接口 ================= */

void FreqMeasure_Init(void) {
    g_freq.mode = FMODE_PERIOD;
    freq_hw_switch(FMODE_PERIOD);
}


void FreqMeasure_Process(Wave_Struct *wave) {
    if (g_freq.mode == FMODE_PERIOD) {
        float f = freq_read_period();
        if (f >= 0.0f) {
            wave->Freq = f;
           
            if (f > SWITCH_HIGH_THR) {
                g_freq.mode = FMODE_COUNT;
                freq_hw_switch(FMODE_COUNT);
                g_freq.measuring = 0;
            }
        }
    } else {
        if (!g_freq.measuring) {
            __HAL_TIM_SET_COUNTER(&htim2, 0);   
            g_freq.gate_start_ms = HAL_GetTick(); 
            g_freq.measuring     = 1;
        } else {
            uint32_t current_ms = HAL_GetTick();
            uint32_t delta_ms   = current_ms - g_freq.gate_start_ms;

            if (delta_ms >= GATE_TIME_MS) {
                uint32_t total_pulses = __HAL_TIM_GET_COUNTER(&htim2);

                if (delta_ms > 0) {
                    wave->Freq = (float)(((double)total_pulses * 1000.0) / (double)delta_ms);
                }

                g_freq.measuring = 0;
             
                if (wave->Freq < SWITCH_LOW_THR) {
                    g_freq.mode = FMODE_PERIOD;
                    freq_hw_switch(FMODE_PERIOD);
                }
            }
        }
    }
}