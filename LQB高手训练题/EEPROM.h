#ifndef __EEPROM_H_
#define __EEPROM_H_

#include "System.h"

static void I2C_Delay(unsigned char n);
void EEPROM_Write(u8* dat,u8 addr,u8 num);
void EEPROM_Read(u8* dat,u8 addr,u8 num);

#endif