#ifndef __TASKS_H
#define __TASKS_H


#include "fftana.h"
#include "global_types.h"

// --- 宏定义 ---
#define TIM3_MAX 20000000    // TIM3基础时钟20MHz
#define AD_MAX  1500000     // ADC 采样率上限 1.5Msps

extern uint8_t adc_dma_finish; 

extern __attribute__((section (".AXI_SRAM"))) uint16_t adc1_buffer[FFT_N] ;
extern fftin FFT_IN;
extern fftdata FFT_OUT;
extern max_3_index Top3;

Wave_Struct Wave_Info = {
    .Freq = 0,
    .Vpp = 0,
    .Wave_type = 0,
};
void System_Tasks_Init(void);

void App_process(void);

#endif // __TASKS_H