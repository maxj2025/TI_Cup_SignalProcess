/**
 * @file    adc_task.h
 * @brief   ADC + DMA 采集控制 API
 * 
 * 用法：
 *   1. Start_ADC_DMA()  — 启动触发采样
 *   2. Stop_ADC_DMA()   — 停止采样
 *   3. Set_ADC_Speed()  — 根据目标频率动态调整采样率
 *   4. FFT_Task()       — 完整 FFT 分析任务
 *   5. USART_Task()     — 通过串口发送结果到串口屏
 */

#ifndef __ADC_TASK_H
#define __ADC_TASK_H

#include "app_types.h"
#include <stdint.h>

/* 全局变量（定义在 main.c，此处声明） */
extern volatile uint8_t  adc_dma_finish;
extern Wave_Struct       g_wave_info;
extern uint16_t          g_adc_buffer[];
extern fftin_t           g_fft_in;
extern fftout_t          g_fft_out;
extern peak3_t           g_peaks;

void Start_ADC_DMA(void);
void Stop_ADC_DMA(void);
void Set_ADC_Speed(Wave_Struct *wave);
void FFT_Task(Wave_Struct *wave);
void USART_Task(Wave_Struct *wave);

#endif
