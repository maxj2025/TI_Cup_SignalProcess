
#include "bsp_system.h"

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
   
    for (uint16_t i = 0; i < FFT_N; i++) {
			  data_in->cmp[2*i] *= windows_func[i];
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

WaveType_t Rec_wavetype(fftdata *freqin, uint16_t idx) {

    if (idx >= FFT_N_2 || idx == 0) {
        return WAVE_UNKNOWN;
    }

    float32_t MAX_Val = freqin->mag[idx];

    if (MAX_Val < 0.02f) {
        return WAVE_SINE; 
    }

    // 2. 获取三次谐波（使用之前实现的邻域搜索函数）
    float32_t Three_Temp = Max_Three_Find(freqin->mag, idx);

    // 3. 计算比值 (Ratio = 三次谐波 / 基波)
    float32_t ratio = Three_Temp / MAX_Val;

    // 4. 阈值判定逻辑
    // 三角波理论值 1/9 ≈ 0.111
    if (ratio >= 0.06f && ratio <= 0.18f) {
        return WAVE_TRIANGLE;
    }
    // 方波理论值 1/3 ≈ 0.333
    else if (ratio >= 0.22f && ratio <= 0.45f) {
        return WAVE_SQUARE;
    }
    // 比值极小或不在上述区间，判定为正弦波
    else {
        return WAVE_SINE;
    }
}

void Phase_atan(float32_t* FFT_In_Complex, uint32_t Index, float32_t* Phase) {
    *Phase = atan2f(FFT_In_Complex[2*Index+1], FFT_In_Complex[2*Index]) * 180.0f / PI;
}

float32_t Find_Vpp(fftin *input)
{
    // 根据项目规范使用 FFT_N，此处按你原逻辑取一半的数据进行时域分析
    int num_samples = FFT_N / 2; 
    
    if (num_samples <= 0) return 0.0f;

    // 1. 计算信号的 DC 直流偏置 (平均值)
    float32_t dc_offset = 0.0f;
    for(int i = 0; i < num_samples; i++) {
        dc_offset += input->cmp[2 * i];
    }
    dc_offset /= (float32_t)num_samples;

    // 2. 过零检测与周期极值抓取
    float32_t AD_sum_max = 0.0f;
    float32_t AD_sum_min = 0.0f;
    uint32_t Max_count = 0;
    uint32_t Min_count = 0;

    float32_t current_cycle_max = dc_offset;
    float32_t current_cycle_min = dc_offset;
    
    // 状态机：0=寻找起始点, 1=处于正半周, -1=处于负半周
    int state = 0; 

    for(int i = 0; i < num_samples; i++) {
        float32_t val = input->cmp[2 * i];

        if (val > dc_offset) {
            // 从负半周跨越到正半周：结算上一个负半周的最小值
            if (state == -1) {
                AD_sum_min += current_cycle_min;
                Min_count++;
                current_cycle_max = val; // 重置最大值记录器
            } 
            else {
                // 持续在正半周：不断刷新当前周期的最大值
                if (val > current_cycle_max) {
                    current_cycle_max = val;
                }
            }
            state = 1;
        }
        else if (val < dc_offset) {
            // 从正半周跨越到负半周：结算上一个正半周的最大值
            if (state == 1) {
                AD_sum_max += current_cycle_max;
                Max_count++;
                current_cycle_min = val; // 重置最小值记录器
            } 
            else {
                // 持续在负半周：不断刷新当前周期的最小值
                if (val < current_cycle_min) {
                    current_cycle_min = val;
                }
            }
            state = -1;
        }
    }

    // 容错：处理最后一段未闭合的半周期
    if (state == 1 && current_cycle_max > dc_offset + 0.05f) {
        AD_sum_max += current_cycle_max; 
        Max_count++;
    } else if (state == -1 && current_cycle_min < dc_offset - 0.05f) {
        AD_sum_min += current_cycle_min; 
        Min_count++;
    }

    // 3. 计算最终平均峰峰值
    if(Max_count == 0 || Min_count == 0) return 0.0f;
    
    float32_t Vpp_AD_Max = AD_sum_max / (float32_t)Max_count;
    float32_t Vpp_AD_Min = AD_sum_min / (float32_t)Min_count;

    return (Vpp_AD_Max - Vpp_AD_Min);
}


float32_t Max_Harmonic_Find(float32_t* Input, uint16_t Base_Index, uint8_t Harmonic_N) {
    
	uint32_t target = (uint32_t)Base_Index * Harmonic_N;
    
    if (target >= FFT_N_2) {
        return 0.0f;
    }

    float32_t max_val = 0.0f;
    
    // 3. 定义搜索邻域：通常取 +/- 2 或 +/- 3 个频点
    // 因为不加窗的矩形窗主瓣较窄，+/- 2 足够覆盖能量集中的区域
    int8_t search_range = 2; 

    for (int8_t offset = -search_range; offset <= search_range; offset++) {
        int32_t current_idx = (int32_t)target + offset;
        
        // 4. 确保搜索索引不越界 (不能小于0，不能超过 FFT_N_2)
        if (current_idx > 0 && current_idx < FFT_N_2) {
            if (Input[current_idx] > max_val) {
                max_val = Input[current_idx];
            }
        }
    }

    return max_val;
}

