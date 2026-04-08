#ifndef __TASKS_H
#define __TASKS_H


#include "fftana.h"
#include "global_types.h"

// --- 宏定义 ---
#define TIM3_MAX 20000000    // TIM3基础时钟20MHz
#define AD_MAX  1500000     // ADC 采样率上限 1.5Msps

extern volatile uint8_t adc_dma_finish; 
extern Wave_Struct Wave_Info;
extern __attribute__((section (".AXI_SRAM"))) uint16_t adc1_buffer[FFT_N] ;
extern fftin FFT_IN;
extern fftdata FFT_OUT;
extern max_3_index Top3;



void Start_ADC_DMA(void);
 void Stop_ADC_DMA(void);
 void FFT_Task(Wave_Struct* Wave_P);//FFT处理任务，输入输出通过Wave_P结构体传递
 void Set_ADC_Speed(Wave_Struct* Wave_P);//根据频率调整ADC采样率
 void USART_Task(Wave_Struct* P_Wave);//串口处理任务，负责将结果通过串口发送给上位机显示
#endif // __TASKS_H