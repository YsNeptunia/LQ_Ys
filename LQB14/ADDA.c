#include "ADDA.h"
//控制字 启用DAC：0x40 光敏(AIN1)0x41，Rb2(AIN3)0x43 同时获取需要调换！
//8591写地址：0x90；读地址：0x91

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

//void DA_Write(u8 addr)//DA转换，入口参数模拟量(0~255对应0~5v)
//{
//	I2CStart();
//	I2CSendByte(0x90);
//	I2CWaitAck();
//	I2CSendByte(0x40);
//	I2CWaitAck();
//	I2CSendByte(addr);
//	I2CWaitAck();
//	I2CStop();
//}
