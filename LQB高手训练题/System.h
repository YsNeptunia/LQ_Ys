#ifndef __SYSTEM_H_
#define __SYSTEM_H_

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

#define DECtoBCD(dec) (dec/10*16)+(dec%10)
#define BCDtoDEC(bcd) (bcd/16*10)+(bcd%16)

#include <STC15F2K60S2.H>
#include <intrins.h>
#include <stdio.h>
#include <string.h>

#include "iic.h"
#include "onewire.h"
#include "DS18B20.h"
#include "Uart.h"
#include "ds1302.h"
#include "Device.h"
#include "Timer.h"
#include "KBD.h"
#include "SEG.h"
#include "Ultra.h"
#include "ADDA.h"
#include "EEPROM.h"


#endif