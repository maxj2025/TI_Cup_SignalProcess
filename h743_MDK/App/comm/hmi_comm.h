/**
 * @file    hmi_comm.h
 * @brief   串口屏 (HMI) 通信 API —— 通过 UART3 与陶晶驰等串口屏通信
 * 
 * 协议格式：控件名="值"\xff\xff\xff
 */

#ifndef __HMI_COMM_H
#define __HMI_COMM_H

#include <stdint.h>

void HMI_Init(void);
void HMI_Send_Str(const char *ctl_name, const char *text);
void HMI_Send_Int(const char *ctl_name, int num);
void HMI_Send_Float(const char *ctl_name, float num, int decimals);
void HMI_Wave_Add(const char *ctl_name, int ch, int val);
void HMI_Wave_Fast(const char *ctl_name, int ch, int count, int *data);
void HMI_Wave_Clear(const char *ctl_name, int ch);
void UART3_Printf(const char *fmt, ...);

#endif
