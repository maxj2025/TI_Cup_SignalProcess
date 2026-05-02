/**
 * @file    freq_measure.h
 * @brief   频率测量 API —— 双模式自动切换（测周法 ↔ 测频法，H7硬件加速版）
 * * 用法：
 * 1. main() 中调用 FreqMeasure_Init()
 * 3. 主循环中周期性调用 FreqMeasure_Process(&wave_info)
 */

#ifndef __FREQ_MEASURE_H
#define __FREQ_MEASURE_H

#include "app_types.h"    
#include "stm32h7xx_hal.h"

void FreqMeasure_Init(void);
void FreqMeasure_Process(Wave_Struct *wave);

#endif /* __FREQ_MEASURE_H */