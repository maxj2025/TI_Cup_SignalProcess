/**
 * @file    freq_measure.c
 * @brief   频率测量实现 —— 非阻塞双模式
 */

#include "freq_measure.h"
#include "app_config.h"
#include "tim.h"
#include "gpio.h"

/* ---- 内部状态 ---- */
typedef struct {
    uint8_t  mode;          // 0=测周, 1=测频
    uint64_t pulse_count;
    uint32_t gate_start_ms;
    uint8_t  measuring;
    uint8_t  first_frame;
} FreqCtx_t;

static FreqCtx_t g_freq = {0};

/* ---- 硬件切换 ---- */
static void freq_hw_switch(uint8_t target_mode) {
    GPIO_InitTypeDef gpio = {0};

    if (target_mode == FMODE_PERIOD) {
        /* 切换到 TIM2 输入捕获（测周法） */
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
        HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);

        HAL_TIM_Base_Stop(&htim2);
        HAL_TIM_IC_Stop(&htim2, TIM_CHANNEL_1);

        __HAL_RCC_TIM2_FORCE_RESET();
        for (volatile uint32_t d = 0; d < 500; d++) {}
        __HAL_RCC_TIM2_RELEASE_RESET();

        htim2.State = HAL_TIM_STATE_RESET;
        MX_TIM2_Init();
        htim2.State = HAL_TIM_STATE_READY;

        __HAL_RCC_GPIOA_CLK_ENABLE();
        gpio.Pin       = GPIO_PIN_5;
        gpio.Mode      = GPIO_MODE_AF_PP;
        gpio.Pull      = GPIO_PULLDOWN;
        gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(GPIOA, &gpio);
        (void)GPIOA->IDR;

        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1 | TIM_FLAG_UPDATE);
        HAL_TIM_Base_Start(&htim2);
        HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_1);

    } else {
        /* 切换到 EXTI 脉冲计数（测频法） */
        HAL_TIM_Base_Stop(&htim2);
        HAL_TIM_IC_Stop(&htim2, TIM_CHANNEL_1);

        __HAL_RCC_GPIOA_CLK_ENABLE();
        gpio.Pin   = GPIO_PIN_5;
        gpio.Mode  = GPIO_MODE_IT_RISING;
        gpio.Pull  = GPIO_PULLDOWN;
        HAL_GPIO_Init(GPIOA, &gpio);
        (void)GPIOA->IDR;

        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
        HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    }

    g_freq.first_frame = 1;
}

/* ---- 测周法读取 ---- */
static float freq_read_period(void) {
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC1) != RESET) {
        uint32_t cap = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);

        if (g_freq.first_frame) { g_freq.first_frame = 0; return -1.0f; }
        if (cap == 0) return 0.0f;
        return (float)TIM2_TICK_FREQ / (float)(cap + 1);
    }
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        return 0.0f;
    }
    return -1.0f;  // 等待中
}

/*============================================================================
 * 公共 API
 *============================================================================*/

void FreqMeasure_Init(void) {
    g_freq.mode = FMODE_PERIOD;
    freq_hw_switch(FMODE_PERIOD);
}

void FreqMeasure_Count_Handler(uint16_t gpio_pin) {
    if (gpio_pin == GPIO_PIN_5 && g_freq.mode == FMODE_COUNT) {
        if (g_freq.first_frame) { g_freq.first_frame = 0; return; }
        g_freq.pulse_count++;
    }
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
            g_freq.pulse_count  = 0;
            g_freq.gate_start_ms = HAL_GetTick();
            g_freq.measuring     = 1;
        } else if (HAL_GetTick() - g_freq.gate_start_ms >= GATE_TIME_MS) {
            wave->Freq = (float)g_freq.pulse_count;
            g_freq.measuring = 0;
            if (wave->Freq < SWITCH_LOW_THR) {
                g_freq.mode = FMODE_PERIOD;
                freq_hw_switch(FMODE_PERIOD);
            }
        }
    }
}
