#include "Timer.h"

void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			
	TMOD |= 0x05;			//GATE = 0;设置计数器模式,非自动重装载
	TL0 = 0;				//设置定时初始值
	TH0 = 0;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}

void Timer1_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
}

void Timer2_Init(void)		//10毫秒@12.000MHz
{
	AUXR &= 0xFB;			//定时器时钟12T模式
	T2L = 0xF0;				//设置定时初始值
	T2H = 0xD8;				//设置定时初始值
	AUXR |= 0x10;			//定时器2开始计时
}