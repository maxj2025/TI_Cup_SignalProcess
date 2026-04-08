//
// Created by h3_Saki on 2025/10/28.
//

#ifndef PROJECT_FREQANA_H
#define PROJECT_FREQANA_H

#include "global_types.h"

#define FFT_N 2048//采样点数
#define FFT_2N 4096
#define FFT_N_2 1024
#define freq_s 1024000 //采样率

#define FREQ_RES ((float)freq_s / (float)FFT_N)//频率分辨率
#define DC_SCOPE 80 //0-100hz的频段将被视作直流分量
#define FREQ_SCOPE 0 //某一频率附近正负2570Hz的点将会被是做同一频率
#define FREQ_STEP 10 //频率步长，对应题目中的波形频率为5Khz的整数倍
#define RAM_ADDR ".SRAM1" //指定变量存放在SRAM_D1区域（使用链接脚本中的 .sram_d1 NOLOAD 段）
#define HAMMING_WIN_CORR 1.85185185185f // 汉明窗幅度恢复系数（补偿加窗带来的幅值衰减）
#define MIN_VALID_DENOM 1e-12f           // 除法安全阈值（防止分母过小导致浮点运算异常）
#define FREQ_LIMIT_HIGH 150000//频率上限150khz
#define FREQ_LIMIT_LOW 5000//频率下限5000hz

typedef struct __attribute__((packed)) {
    float32_t cmp[FFT_2N];
} fftin;

typedef struct {//定义几个函数的输出结构体
    float32_t phase[FFT_2N];
    float32_t mag[FFT_N_2];

} fftdata;

typedef struct {
    uint16_t index[3];
} max_3_index;

typedef struct {
    float32_t axis[FFT_N_2];
} freqaxis_t;

void process_data(const uint16_t *data_ori,fftin *data_processed);//adc信号处理转化成一实一虚，3.3v,去直流偏置

 void add_window(fftin *data_in, const float32_t *windows_func);//加窗函数

void fft_process(fftin *data,fftdata *output);//fft算法

freqaxis_t *get_freqaxis();//获取频率维度

void regurlize_mag(fftdata *fft_result,float32_t regurlize_num);//规范化fft结果

void get_max_3(const fftdata *comparein, max_3_index *top3);//获取幅度谱最大的三个索引

float32_t findnearfreq(float32_t freq1);//规范化频率

void Phase_atan(float32_t* FFT_In_Complex, uint32_t Index, float32_t* Phase); //相位与基础频率提取

float32_t Find_Vpp(fftin *input);//时域vpp提取（滑动窗口法）

WaveType_t Rec_wavetype(fftdata *freqin, uint16_t idx);//波形识别

float32_t Max_Harmonic_Find(float32_t* Input, uint16_t Base_Index, uint8_t Harmonic_N) ;


#endif //PROJECT_FREQANA_H