/*
 * 设计原则：
 *   - 仅包含 <stdint.h>，不引入 HAL/DSP 等重型头文件
 *   - 所有结构体按功能分组，便于查找
 *   - 使用 float 而非 float32_t，避免绑定 ARM DSP 类型
*/

#ifndef __APP_TYPES_H
#define __APP_TYPES_H

#include <stdint.h>

/*===========================================================================
 * 一、波形与信号分析类型
 *===========================================================================*/

/** 波形类型枚举 */
typedef enum {
    WAVE_SINE = 0,      // 正弦波
    WAVE_SQUARE,        // 方波
    WAVE_TRIANGLE,      // 三角波
    WAVE_UNKNOWN        // 未知 / 初始状态
} WaveType_t;

/** 波形测量结果（频率 + 峰峰值 + 波形类型） */
typedef struct {
    float   Freq;       // 频率 (Hz)
    float   Vpp;        // 峰峰值 (V)
    WaveType_t Wave_type;
} Wave_Struct;

/*===========================================================================
 * 二、FFT 专用类型
 *===========================================================================*/
#include "app_config.h"  // 仅用于获取 FFT_N

#define FFT_2N  (FFT_N * 2)

/** FFT 输入（复数交错: real[0], imag[0], real[1], imag[1]...） */
typedef struct {
    float cmp[FFT_2N];
} fftin_t;

/** FFT 输出（幅度谱 + 相位谱） */
typedef struct {
    float phase[FFT_2N];
    float mag[FFT_N_2];
} fftout_t;

/** 前三大峰值索引 */
typedef struct {
    uint16_t index[3];
} peak3_t;

/** 频率轴 */
typedef struct {
    float axis[FFT_N_2];
} freqaxis_t;

/*===========================================================================
 * 三、频率测量状态类型
 *===========================================================================*/

/** 测频模式 */
typedef enum {
    FMODE_PERIOD = 0,   // 测周法（低频高精度）
    FMODE_COUNT  = 1    // 测频法（高频）
} FreqMode_t;

/*===========================================================================
 * 四、AD9910 波形枚举
 *===========================================================================*/

typedef enum {
    AD9910_WAVE_TRI  = 0,
    AD9910_WAVE_SQR  = 1,
    AD9910_WAVE_SINC = 2,
} AD9910_Wave_t;

#endif /* __APP_TYPES_H */
