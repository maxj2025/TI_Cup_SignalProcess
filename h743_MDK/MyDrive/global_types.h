
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



#endif