#include "HMI.h"

void HMI_send_string(char* name,char* showdata)
{
	UART3_Printf("%s=\"%s\"\xff\xff\xff",name,showdata);
}

void HMI_send_number(char* name,int num)
{
	UART3_Printf("%s=%d\xff\xff\xff",name,num);
	
}

void HMI_send_float(char* name,float num)
{
	UART3_Printf("%s=%d\xff\xff\xff",name,(int)(num*100));
}

void HMI_send_wave(char* name,int ch,int val)
{
	UART3_Printf("add %s,%d,%d\xff\xff\xff",name,ch,val);
}

void HMI_Wave_fast(char* name,int ch,int count,int* show_data)
{
	int i = 0;
	UART3_Printf("addt %s,%d,%d\xff\xff\xff",name,ch,count);
	HAL_Delay(100);
	for(i=0;i<count;i++)
	{
		UART3_Printf("%c",show_data[i]);
		UART3_Printf("\xff\xff\xff");
	}
}

void HMI_Wave_clear(char* name,int ch)
{
	UART3_Printf("cle %s,%d\xff\xff\xff",name,ch);
}


void UART3_Printf(const char *format,...)
{
	char tmp[128] = {0};  // 初始化清零，避免脏数据
	va_list argptr;
	va_start(argptr, format);

	int len = vsnprintf(tmp, 128 - 1, format, argptr);
	va_end(argptr);
	if(len <= 0) return;  // 格式化失败，直接返回
	if(len >= 128) len = 128 - 1;  // 截断保护

	if(__get_PRIMASK())
	{
		HAL_UART_Transmit(&huart3, (uint8_t *)tmp, len, 1);
	}
	else
	{
		HAL_UART_Transmit(&huart3, (uint8_t *)tmp, len, HAL_MAX_DELAY);
	}

}

void HMI_Init(void)
{
	HMI_send_string("t1.txt", "待测");
	HMI_send_string("t3.txt", "待测");
	HMI_send_string("t5.txt", "待测");
	HMI_send_string("t7.txt", "待测");
}


