/**
 * @file    hmi_comm.c
 * @brief   串口屏通信实现
 */

#include "hmi_comm.h"
#include "usart.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdarg.h>

#define HMI_TAIL "\xff\xff\xff"

/* ---- 格式化打印到 UART3 ---- */
void UART3_Printf(const char *fmt, ...) {
    char buf[128] = {0};
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);
    if (len <= 0) return;
    if (len >= (int)sizeof(buf)) len = (int)sizeof(buf) - 1;

    if (__get_PRIMASK()) {
        HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 1);
    } else {
        HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, HAL_MAX_DELAY);
    }
}

/* ---- HMI 控件操作 ---- */
void HMI_Send_Str(const char *ctl, const char *text) {
    UART3_Printf("%s=\"%s\"" HMI_TAIL, ctl, text);
}

void HMI_Send_Int(const char *ctl, int num) {
    UART3_Printf("%s=%d" HMI_TAIL, ctl, num);
}

void HMI_Send_Float(const char *ctl, float num, int decimals) {
    long scaled = (long)(num * (decimals == 2 ? 100.0f : 1000.0f) + 0.5f);
    UART3_Printf("%s=%ld" HMI_TAIL, ctl, scaled);
}

void HMI_Wave_Add(const char *ctl, int ch, int val) {
    UART3_Printf("add %s,%d,%d" HMI_TAIL, ctl, ch, val);
}

void HMI_Wave_Fast(const char *ctl, int ch, int count, int *data) {
    UART3_Printf("addt %s,%d,%d" HMI_TAIL, ctl, ch, count);
    HAL_Delay(100);
    for (int i = 0; i < count; i++) {
        UART3_Printf("%c" HMI_TAIL, (char)data[i]);
    }
}

void HMI_Wave_Clear(const char *ctl, int ch) {
    UART3_Printf("cle %s,%d" HMI_TAIL, ctl, ch);
}

void HMI_Init(void) {
    HMI_Send_Str("t1.txt", "待测");
    HMI_Send_Str("t3.txt", "待测");
    HMI_Send_Str("t5.txt", "待测");
    HMI_Send_Str("t7.txt", "待测");
}
