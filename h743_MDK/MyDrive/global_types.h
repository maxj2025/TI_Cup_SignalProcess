
#ifndef __GLOBAL_TYPES_H
#define __GLOBAL_TYPES_H

#include <stdint.h>
#include <stdarg.h>    
#include <stdio.h>     
#include "arm_math.h" 

/************* 全局宏定义 *************************/
#define RXBUFFERSIZE 500 
/************* 枚举类型定义 ***********************/
typedef enum {
    WAVE_SINE = 0,     // 正弦波
    WAVE_SQUARE,       // 方波
    WAVE_TRIANGLE,     // 三角波
    WAVE_UNKNOWN       // 未知/初始状态
} WaveType_t;

/************* 结构体类型定义 *********************/
/* 改进后的波形数据结构体 */
typedef struct {
    float32_t Freq;      // 频率
    float32_t Vpp;       // 峰峰值
    WaveType_t Wave_type; 
} Wave_Struct;

typedef struct {
    uint8_t Freq_flage;
    uint16_t Freq_time;
    uint8_t mode_flage;
} Freq_Struct;

typedef struct {
    Wave_Struct Original;   // 原始信号 A (正弦波)
    Wave_Struct Interfere;  // 干扰信号 B (正弦/方波/三角)
    float32_t Total_RMS;    // 混合信号的总有效值
} Analysis_Result_t;

typedef struct {
    uint8_t  current_mode;  // 0: 测周法(输入捕获), 1: 测频法(外部中断)
    uint64_t count;         // 测频法计数
    uint32_t gate_start;    // 闸门起始时间
    uint8_t  is_measuring;  // 测频法测量标志
} FreqControl_t;

#endif