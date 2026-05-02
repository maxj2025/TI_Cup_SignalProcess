/**
 * @file    freq_measure.h
 * @brief   频率测量 API —— 双模式自动切换（测周法 ↔ 测频法）
 * 
 * 用法：
 *   1. main() 中调用 FreqMeasure_Init()
 *   2. 在 HAL_GPIO_EXTI_Callback 中调用 FreqMeasure_Count_Handler()
 *   3. 主循环中周期性调用 FreqMeasure_Process(&wave_info)
 */

#ifndef __FREQ_MEASURE_H
#define __FREQ_MEASURE_H

#include "app_types.h"
#include "stm32h7xx_hal.h"

void FreqMeasure_Init(void);
void FreqMeasure_Process(Wave_Struct *wave);
void FreqMeasure_Count_Handler(uint16_t gpio_pin);

#endif
