#ifndef __USONIC_H_
#define __USONIC_H_

#include "System.h"

sbit Tx = P1^0;
sbit Rx = P1^1;

void Delay12us(void);	//@12.000MHz
void Ultra_Init();//超声波初始化函数，产生8个40MHz的方波
u8 Ultra_Read();//读取距离

#endif