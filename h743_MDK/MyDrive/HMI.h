#ifndef __HMI_H
#define __HMI_H

#include "global_types.h"

void HMI_send_string(char* name, char* showdata);     // 向屏幕指定控件发送字符串
void HMI_send_number(char* name, int num);            // 向屏幕指定控件发送整数
void HMI_send_float(char* name, float num);           // 向屏幕指定控件发送浮点数
void HMI_send_wave(char* name, int ch, int val);      // 向波形控件指定通道添加一个数据点
void HMI_Wave_fast(char* name, int ch, int count, int* show_data); // 批量发送数组数据绘制波形
void HMI_Wave_clear(char* name, int ch);              // 清除指定波形通道的所有数据
void UART3_Printf(const char *format,...);
void HMI_Init(void);
#endif