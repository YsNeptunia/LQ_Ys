#include "Timer.h"

void Timer2_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xFB;			//定时器时钟12T模式
	T2L = 0x18;				//设置定时初始值
	T2H = 0xFC;				//设置定时初始值
	AUXR |= 0x10;			//定时器2开始计时
}
