//
// Created by h3_Saki on 2025/10/28.
//
#include "fftana.h"
#include "arm_const_structs_extra.h"
#include "arm_math.h"
#include "extra_ffts.h"

void process_data(const uint16_t *data_ori, fftin *data_processed) {
    float32_t dc_offset = 0.0f;
    float32_t voltage_scale = 3.3f / 4095.0f;
    
    float32_t sum = 0;
    for (uint32_t i = 0; i < FFT_N; i++) {
        sum += (float32_t)data_ori[i];
    }
    dc_offset = sum / (float32_t)FFT_N;
    for (uint32_t i = 0; i < FFT_N; i++) {
        data_processed->cmp[2 * i] = ((float32_t)data_ori[i] - dc_offset) * voltage_scale;
        data_processed->cmp[2 * i + 1] = 0.0f;
    }
}

void add_window(fftin *data_in, const float32_t *windows_func) {//加窗函数，采用取采样点的方式
    arm_cmplx_mult_real_f32(data_in->cmp, windows_func, data_in->cmp, FFT_N);
    for (uint16_t i = 0; i < FFT_N; i++) {
        data_in->cmp[2*i + 1] = 0.0f;
    }
}

arm_cfft_instance_f32_extra *get_fft_num(uint16_t fft_n) {//获取fft常量，最大支持16384点
    switch (fft_n) {
    case 16:
        return &arm_cfft_sR_f32_len16_extra;
    case 32:
        return &arm_cfft_sR_f32_len32_extra;
    case 64:
        return &arm_cfft_sR_f32_len64_extra;
    case 128:
        return &arm_cfft_sR_f32_len128_extra;
    case 256:
        return &arm_cfft_sR_f32_len256_extra;
    case 512:
        return &arm_cfft_sR_f32_len512_extra;
    case 1024:
        return &arm_cfft_sR_f32_len1024_extra;
    case 2048:
        return &arm_cfft_sR_f32_len2048_extra;
    case 4096:
        return &arm_cfft_sR_f32_len4096_extra;
    case 8192:
        return &arm_cfft_sR_f32_len8192_extra;
    case 16384:
        return &arm_cfft_sR_f32_len16384_extra;
    default:
        return &arm_cfft_sR_f32_len2048_extra;
    }
}

void fft_process(fftin *data,fftdata *output) {//对信号进行fft，返回相谱，频谱，频率坐标
    arm_cfft_f32_extra(get_fft_num(FFT_N),data -> cmp,0,1);
    for (uint16_t i = 0; i < FFT_N_2; i++) {
        float32_t sqrt_result;
        float32_t *sqrt_result_p = &sqrt_result;
        arm_sqrt_f32(((data->cmp[2 * i]) * (data->cmp[2 * i])) +
            ((data->cmp[2 * i + 1]) * (data->cmp[2 * i + 1]))
            ,sqrt_result_p);
        output -> mag[i] = sqrt_result;
    }
    for (uint16_t i = 0; i < FFT_N_2; i++) {//输出相位谱
        output -> phase[i] = atan2(data -> cmp[2*i + 1] , data -> cmp[2*i]);
    }

    freqaxis_t *freqaxis_list = get_freqaxis();
    for (uint32_t i = 0; i < FFT_N_2; i++) {//去除直流分量
         if ((freqaxis_list -> axis[i]) <= DC_SCOPE) {
             output -> mag[i] = 0;//100hz以内的频率将被当做直流分量处理归零
         }
    }
}

freqaxis_t *get_freqaxis() { //获取频率维度
    __attribute__((section(RAM_ADDR))) static freqaxis_t freqaxis; // moved to .sram_d1 NOLOAD
    for (uint32_t i = 0; i < FFT_N_2; i++) {
        float32_t temp = (float32_t)i;//把i转化为浮点数,防止后续对频率的浮点数乘法出错
        freqaxis.axis[i] = temp * freq_s / FFT_N;
    }
    return &freqaxis;
}

void regurlize_mag(fftdata *fft_result,const float32_t regurlize_num) {
    float32_t max_mag = 0;
    for (uint16_t i = 0; i < FFT_N_2; i++) {
        if (max_mag <= fft_result -> mag[i]) {
            max_mag = fft_result -> mag[i];
        }
    }
    for (uint16_t i = 0; i < FFT_N_2; i++) {
        fft_result ->mag[i] = fft_result -> mag[i] / max_mag * regurlize_num ;//fft频谱规范化
    }
}

void get_max_3(const fftdata *comparein, max_3_index *top3) {
    freqaxis_t *freqaxis = get_freqaxis();
    float32_t tmp=0;
    // 找到振幅最大的频点
    for (uint16_t i = 0; i < (FFT_N / 2); i++) {
        if (tmp <= comparein->mag[i]) {
            tmp = comparein->mag[i];
            top3->index[0] = i; 
        }
    }

    tmp = 0;
    float32_t maxamp = comparein->mag[top3->index[0]];
    float32_t freq1up = freqaxis->axis[top3->index[0]] + FREQ_SCOPE;
    float32_t freq1down = freqaxis->axis[top3->index[0]] - FREQ_SCOPE;

    // 找到振幅第二大的频点
    for (uint16_t i = 0; i < FFT_N / 2; i++) {
        if (tmp <= comparein->mag[i] && comparein->mag[i] < maxamp) {
            if (freqaxis->axis[i] > freq1up || freqaxis->axis[i] < freq1down) {
                tmp = comparein->mag[i];
                top3->index[1] = i; 
            }
        }
    }

    float32_t maxamp2 = comparein->mag[top3->index[1]];
    float32_t freq2up = freqaxis->axis[top3->index[1]] + FREQ_SCOPE;
    float32_t freq2down = freqaxis->axis[top3->index[1]] - FREQ_SCOPE;

    tmp = 0;
    // 找到振幅第三大的频点
    for (uint16_t i = 0; i < FFT_N / 2; i++) {
        if (tmp <= comparein->mag[i] && comparein->mag[i] < maxamp2) {
            if (freqaxis->axis[i] > freq1up || freqaxis->axis[i] < freq1down) {
                if (freqaxis->axis[i] > freq2up || freqaxis->axis[i] < freq2down) {
                    tmp = comparein->mag[i];
                    top3->index[2] = i; 
                }
            }
        }
    }
}

float32_t findnearfreq(float32_t freq1) {
    // 1. 基础舍入逻辑
    uint32_t freq1_int = (uint32_t)freq1;
    uint32_t freq1up = FREQ_STEP - (freq1_int % (uint32_t)FREQ_STEP);
    uint32_t freq1down = freq1_int % (uint32_t)FREQ_STEP;
    
    float32_t freq1_out;
    
    if (freq1up >= freq1down) {
        freq1_out = (float32_t)(freq1_int - freq1down);
    }
    else {
        freq1_out = (float32_t)(freq1_int + freq1up);
    }

    // 2. 增加阈值限制逻辑
    if (freq1_out > FREQ_LIMIT_HIGH) {
        freq1_out = FREQ_LIMIT_HIGH;
    }
    else if (freq1_out < FREQ_LIMIT_LOW) {
        freq1_out = FREQ_LIMIT_LOW;
    }

    return freq1_out;
}
/* --- 新增的函数 --- */
float32_t FFT_PostProcess_FreqAmp(
    float32_t *fftMag, uint32_t peakIdx, float32_t freqRes, 
    uint32_t Len, float32_t winCorrFactor, float32_t *pPreciseVpp)
{
    uint32_t fftMagHalfLen = Len / 2; 
    if (peakIdx < 2 || peakIdx >= (fftMagHalfLen - 2) || fftMag == NULL) return 0.0f;

    float32_t Y_prev2 = fftMag[peakIdx - 2];
    float32_t Y_prev1 = fftMag[peakIdx - 1];
    float32_t Y_peak  = fftMag[peakIdx];
    float32_t Y_next1 = fftMag[peakIdx + 1];
    float32_t Y_next2 = fftMag[peakIdx + 2];

    // 三次多项式系数计算与δ求解
    float32_t numerator = (Y_prev2 - Y_next2) * 2.0f + (Y_next1 - Y_prev1) * 12.0f;
    float32_t denominator = (Y_prev2 - Y_prev1 * 4.0f + Y_peak * 6.0f - Y_next1 * 4.0f + Y_next2) * 12.0f;
    
    if (fabsf(denominator) < MIN_VALID_DENOM) return 0.0f;
    
    float32_t delta = fmaxf(fminf(numerator / denominator, 0.5f), -0.5f);
    float32_t preciseFreq = (peakIdx + delta) * freqRes;

    // 幅度校正计算
    float32_t a = (Y_next2 - Y_prev2 + 3.0f*(Y_prev1 - Y_next1)) / 8.0f;
    float32_t b = (Y_next2 + Y_prev2 - 2.0f*Y_peak) / 4.0f;
    float32_t c = (Y_next1 - Y_prev1) / 2.0f;
    float32_t preciseAmp = a*delta*delta*delta + b*delta*delta + c*delta + Y_peak;
    
    // 窗函数响应补偿校正
    float32_t normalizedFreq = delta * freqRes / (freqRes * 2.0f);
    float32_t windowResponse = 0.54f + 0.46f * cosf(2.0f * PI * normalizedFreq);
    preciseAmp *= (winCorrFactor / windowResponse);

    if (pPreciseVpp != NULL) *pPreciseVpp = preciseAmp * 2.0f;
    return preciseFreq;
}

// 在基波3倍频附近寻找最大值点，应对频谱泄露
float32_t Max_Three_Find(float32_t* Input, uint16_t Max_Index) {
    uint32_t target = 3 * Max_Index;
    if (target >= FFT_N_2) return 0;
    float32_t max_temp = Input[target];
    // 检查邻域 +/- 2 个频点
    for(int offset = -2; offset <= 2; offset++) {
        if((target + offset < FFT_N_2) && (Input[target + offset] > max_temp))
            max_temp = Input[target + offset];
    }
    return max_temp;
}

uint8_t Rec_wavetype(float32_t* Input, uint16_t Len) {
    uint16_t Max_pose = 0;
    float32_t MAX_Val = 0;
    
    // 寻找主频点
    for(int i = 3; i < (Len/2); i++) {
        if(Input[i] > MAX_Val) { MAX_Val = Input[i]; Max_pose = i; }
    }
    
    float32_t Three_Temp = Max_Three_Find(Input, Max_pose);
    float32_t base = Input[Max_pose];
    
    // 根据比值系数识别
    if (base >= 7.0f * Three_Temp && base <= 18.0f * Three_Temp) 
        return 1; // TRIANGLE
    else if (base >= 0.5f * Three_Temp && base <= 4.8f * Three_Temp) 
        return 2; // SQUARE
    else 
        return 0; // SINE
}

void Phase_atan(float32_t* FFT_In_Complex, uint32_t Index, float32_t* Phase) {
    *Phase = atan2f(FFT_In_Complex[2*Index+1], FFT_In_Complex[2*Index]) * 180.0f / PI;
}

void calc_psd_from_fft_mag_float(float32_t fft_mag[], float32_t psd[], float32_t Fs, uint32_t N_FFT, float32_t win_power_coeff) {
    float32_t psd_scale = win_power_coeff / (Fs * N_FFT); //
    for (uint32_t k = 0; k < N_FFT / 2; k++) {
        psd[k] = (fft_mag[k] * fft_mag[k]) * psd_scale;
    }
}


float32_t Find_Vpp(fftin *input)
{
    float32_t Vpp = 0.0f;
    uint32_t Max_count = 0;
    uint32_t Min_count = 0;
    
    float32_t AD_sum_max = 0.0f;
    float32_t AD_sum_min = 0.0f;
    

    int num_samples = FFT_N_2;

    const float32_t VOLT_THRESHOLD = 0.05f; 
    const float32_t MIN_DIFF_THRESHOLD = 0.02f;

    for(int i = 10; i < num_samples - 10; i++)
    {
        uint8_t num_count_max = 0;
        float32_t current_R = input->cmp[2 * i]; 

        for(int j = 1; j < 6; j++)
        {
            if((current_R >= input->cmp[2 * (i - j)]) && 
               (current_R >= input->cmp[2 * (i + j)]) && 
               (current_R >= VOLT_THRESHOLD))
            {
                num_count_max++;
            }
        }
        
        if(num_count_max >= 4)
        {
            AD_sum_max += current_R;
            Max_count++;    
        }   
    }

    if(Max_count == 0) return 0.0f;
    float32_t Vpp_AD_Max = AD_sum_max / (float32_t)Max_count;

    for(int i = 5; i < num_samples - 5; i++)
    {
        uint8_t num_count_min = 0;
        float32_t current_R = input->cmp[2 * i];

        for(int j = 1; j < 6; j++)
        {
            if((current_R <= input->cmp[2 * (i - j)]) && 
               (current_R <= input->cmp[2 * (i + j)]) && 
               (current_R + MIN_DIFF_THRESHOLD < Vpp_AD_Max))
            {
                num_count_min++;
            }
        }
        
        if(num_count_min >= 4)
        {
            AD_sum_min += current_R;
            Min_count++;    
        }   
    }

    if(Min_count == 0) return 0.0f;
    float32_t Vpp_AD_Min = AD_sum_min / (float32_t)Min_count;

    Vpp = Vpp_AD_Max - Vpp_AD_Min; 
    
    return Vpp;
}