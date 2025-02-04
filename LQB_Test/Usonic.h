#ifndef __USONIC_H_
#define __USONIC_H_

#include "System.h"

sbit Tx = P1^0;
sbit Rx = P1^1;

void Delay12us(void);	//@12.000MHz
void Ultra_Init();
u8 Ultra_Read();

#endif