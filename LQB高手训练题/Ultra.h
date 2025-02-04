#ifndef __ULTRA_H_
#define __ULTRA_H_

#include "System.h"

sbit Tx = P1^0;
sbit Rx = P1^1;

void Delay12us(void);
void Ultra_Init();
u8 Ultra_Read();

#endif