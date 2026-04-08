#ifndef __FREQUENCY_MEASURE_H
#define __FREQUENCY_MEASURE_H

#include "stm32h7xx_hal.h"


void FreqMeasure_Init(void);

void FreqMeasure_Process(Wave_Struct* Wave_P);

void FreqMeasure_Count_Handler(uint16_t GPIO_Pin);

#endif