#ifndef __UART_H_
#define __UART_H_

#include "System.h"

void Uart1Init(void);
void SendByte(u8 dat);
void Uart_Send(u8* dat);

#endif