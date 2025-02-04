#ifndef __DS1302_H_
#define __DS1302_H_

#include "System.h"

sbit SCK = P1^7;
sbit SDA = P2^3;
sbit RST = P1^3;

void Write_Ds1302(unsigned  char temp);
void Write_Ds1302_Byte( unsigned char address,unsigned char dat );
unsigned char Read_Ds1302_Byte ( unsigned char address );

void DS1302_Set(u8* time);
void DS1302_Read(u8* time);

#endif