#include "ADDA.h"

u8 AD_Read(u8 addr)//AD转换，入口参数控制字
{
	u8 dat;
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();
	
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	dat = I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	return dat;
}

void DA_Write(float vol)//DA转换，入口参数模拟量(0~255对应0~5v)
{
	u8 addr = vol * (225 / 5);
	
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x40);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();
	I2CStop();
}
