/**
 * @file    adc_task.c
 * @brief   ADC+DMA 采集与 FFT 任务实现
 */

#include "adc_task.h"
#include "app_config.h"
#include "fft_analyzer.h"
#include "hmi_comm.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "stm32h7xx_hal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief 根据待测频率动态调节 ADC 采样率（默认100 倍频采样，自行修改）
 */
void Set_ADC_Speed(Wave_Struct *wave) {
    HAL_TIM_Base_Stop(&htim3);

    uint32_t rate = (uint32_t)(wave->Freq) * 100;
    if (rate > AD_MAX)  rate = AD_MAX;
    if (rate == 0)      rate = 1000;

    uint32_t arr = TIM3_MAX / rate - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    HAL_TIM_Base_Init(&htim3);
    HAL_Delay(1);
}

void Start_ADC_DMA(void) {
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_adc_buffer, FFT_N);
    HAL_TIM_Base_Start(&htim3);
}

void Stop_ADC_DMA(void) {
    HAL_ADC_Stop_DMA(&hadc1);
    HAL_TIM_Base_Stop(&htim3);
}

/**
 * @brief 完整 FFT 分析流水线
 */
void FFT_Task(Wave_Struct *wave) {
    fft_prepare(g_adc_buffer, &g_fft_in);
    wave->Vpp = fft_find_vpp(&g_fft_in);

    fft_process(&g_fft_in, &g_fft_out);
    fft_normalize(&g_fft_out, 1.0f);
    fft_find_peaks(&g_fft_out, &g_peaks);

    /* 通过串口发送原始波形到 VOFA+调试 */
//    for (int i = 0; i < FFT_N; i++) {
//        UART3_Printf("%.3f\n", g_adc_buffer[i] * 3.3f / 4095.0f);
//    }
	
    memset(g_adc_buffer, 0, FFT_N * sizeof(uint16_t));
}

/**
 * @brief 串口任务 —— 将测量结果发送到 HMI 串口屏
 */
void USART_Task(Wave_Struct *wave) {
    char buf[64];

    /* 格式化频率 */
    if (wave->Freq < 1000.0f) {
        snprintf(buf, sizeof(buf), "%.4f Hz", (double)wave->Freq);
    } else if (wave->Freq < 1000000.0f) {
        snprintf(buf, sizeof(buf), "%.4f KHz", (double)wave->Freq / 1000.0);
    } else {
        snprintf(buf, sizeof(buf), "%.5f MHz", (double)wave->Freq / 1000000.0);
    }
    HMI_Send_Str("t1.txt", buf);

    /* 峰峰值 */
    snprintf(buf, sizeof(buf), "%.2f mv", wave->Vpp);
    HMI_Send_Str("t3.txt", buf);
}
