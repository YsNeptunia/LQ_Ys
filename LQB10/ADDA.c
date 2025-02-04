#include "ADDA.h"

u8 AD_Read(u8 addr)//ADת������ڲ���������
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

void DA_Write(float vol)//DAת������ڲ���ģ����(0~255��Ӧ0~5v)
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
