#include "Timer.h"

void Timer2_Init(void)		//1����@12.000MHz
{
	AUXR |= 0x04;			//��ʱ��ʱ��1Tģʽ
	T2L = 0x20;				//���ö�ʱ��ʼֵ
	T2H = 0xD1;				//���ö�ʱ��ʼֵ
	AUXR |= 0x10;			//��ʱ��2��ʼ��ʱ
}

