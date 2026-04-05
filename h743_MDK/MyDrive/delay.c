#include "delay.h"
void delay_us(uint32_t nus)
{
  uint32_t ticks;
	uint32_t told,tnow,tcnt = 0;
	uint32_t reload = SysTick->LOAD+1;
	ticks = nus*(SystemCoreClock/1000000);
	told = SysTick->VAL;
	while(1)
	{
		tnow = SysTick->VAL;
		if(tnow!=told)
		{
			if(tnow<told)tcnt+=told-tnow;
			else tcnt+=reload-tnow+told;
			told = tnow;
			if(tcnt>=ticks)break;		
		}
		
		
	}
}

void delay_ms(uint32_t t)
{
	for(int i=0;i<t;i++)
	{
		delay_us(1000);
	}
	
}

