#include "bsp_system.h"


void Set_ADC_Speed(Wave_Struct* Wave_P)
{
    HAL_TIM_Base_Stop(&htim3);
    uint32_t Sampling_speed = ((uint32_t)(Wave_P->Freq)) * 100;
    
    if (Sampling_speed > AD_MAX) {
        Sampling_speed = AD_MAX;
    }
    else if (Sampling_speed == 0) { // 防止除以 0
        Sampling_speed = 1000;      // 给一个默认的最低采样率1000hz
    }
    uint32_t Actual_Arr = TIM3_MAX / Sampling_speed - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim3, Actual_Arr);
    HAL_TIM_Base_Init(&htim3);
    HAL_Delay(1);
}

void Start_ADC_DMA(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc1_buffer, FFT_N);
    HAL_TIM_Base_Start(&htim3);
}
 void Stop_ADC_DMA(void)
{
    HAL_ADC_Stop_DMA(&hadc1);
    HAL_TIM_Base_Stop(&htim3);
}

 void FFT_Task(Wave_Struct* Wave_P)
{
    //  测量 Vpp
    process_data(adc1_buffer, &FFT_IN); 
    Wave_P->Vpp = Find_Vpp(&FFT_IN);

    // FFT 数据处理
    // add_window(&FFT_IN, Hanning_Window_2048);
    fft_process(&FFT_IN, &FFT_OUT);
    regurlize_mag(&FFT_OUT, 1);
    get_max_3(&FFT_OUT, &Top3);

    // VOFA+ 发波形 (通过串口3打印)
    for(int i = 0; i < FFT_N; i++) {
        UART3_Printf("%.3f\n", adc1_buffer[i] * 3.3f / 4095.0f);
    }
    // 5. 清空数组准备下一次
    memset(adc1_buffer, 0, sizeof(adc1_buffer));
}

 void USART_Task(Wave_Struct* P_Wave)
{
    char* Char_temp = malloc(sizeof(char) * 64);
    if (Char_temp == NULL) return;

    float64_t Freq_temp = 0.0f;

    // 处理频率显示
    if (P_Wave->Freq < 1000) {
        Freq_temp = (float64_t)P_Wave->Freq;
        sprintf(Char_temp, "%.4f Hz", Freq_temp);
    } else if (P_Wave->Freq < 1000000) {
        Freq_temp = (float64_t)P_Wave->Freq / 1000.0f;
        sprintf(Char_temp, "%.4f KHz", Freq_temp);
    } else {
        Freq_temp = (float64_t)P_Wave->Freq / 1000000.0f;
        sprintf(Char_temp, "%.5f MHz", Freq_temp);
    }
    HMI_send_string("t1.txt", Char_temp);
    memset(Char_temp, 0, 64);

    // 峰峰值发送
    sprintf(Char_temp, "%.2f mv", P_Wave->Vpp);
    HMI_send_string("t3.txt", Char_temp);
    memset(Char_temp, 0, 64);

    free(Char_temp);
    Char_temp = NULL; 
}
