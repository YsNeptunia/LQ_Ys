#include "DAC.h"
//������ ����DAC��0x40 ����(AIN1)0x41��Rb2(AIN3)0x43 ͬʱ��ȡ��Ҫ������
//8591д��ַ��0x90������ַ��0x91

void DA_Write(u8 addr)
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x40);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();
	I2CStop();
}
