#ifndef __EEPROM_H_
#define __EEPROM_H_

#include "System.h"

void EEPROM_Write(u8* dat, u8 addr, u8 num);
void EEPROM_Read(u8* dat, u8 addr, u8 num);

#endif