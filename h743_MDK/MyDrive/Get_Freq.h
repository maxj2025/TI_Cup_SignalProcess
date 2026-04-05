#ifndef __FREQUENCY_MEASURE_H
#define __FREQUENCY_MEASURE_H

#include "stm32h7xx_hal.h"

typedef struct {
    uint8_t  current_mode;  // 0: 测周法(输入捕获), 1: 测频法(外部中断)
    uint64_t count;         // 测频法计数
    uint32_t gate_start;    // 闸门起始时间
    uint8_t  is_measuring;  // 测频法测量标志
} FreqControl_t;


void FreqMeasure_Init(void);

void FreqMeasure_Process(float *pFreq);

void FreqMeasure_Count_Handler(uint16_t GPIO_Pin);

#endif