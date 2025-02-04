#include "EEPROM.h"

void EEPROM_Write(u8* dat, u8 addr, u8 num)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	
	I2CSendByte(addr);
	I2CWaitAck();
	
	while(num--){
		I2CSendByte(*dat++);
		I2CWaitAck();
		I2C_Delay(200);
	}
	
	I2CStop();
}

void EEPROM_Read(u8* dat, u8 addr, u8 num)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	
	I2CSendByte(addr);
	I2CWaitAck();

	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	
	while(num--){
		*dat++ = I2CReceiveByte();
		if(num)	I2CSendAck(0);
		else	I2CSendAck(1);
	}
	
	I2CStop();
}
