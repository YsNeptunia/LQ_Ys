#include "EEPROM.h"

static void I2C_Delay(unsigned char n)
{
    do{_nop_();}
    while(n--);      	
}

void EEPROM_Write(u8* dat,u8 addr,u8 num)//形参：将要写入的字符串；写入的地址（8倍数）；写入的数量
{
	I2CStart();
	I2CSendByte(0xa0);//选择AT24C02芯片，写模式
	I2CWaitAck();
	
	I2CSendByte(addr);//选择写入的地址
	I2CWaitAck();
	
	while(num--){
		I2CSendByte(*dat++);//写入数据
		I2CWaitAck();
		I2C_Delay(200);
	}
	
	I2CStop();
}

void EEPROM_Read(u8* dat,u8 addr,u8 num)//形参：将要读出的字符串；读的地址（8倍数）；读的数量
{
	I2CStart();
	I2CSendByte(0xa0);//选择AT24C02芯片，写模式
	I2CWaitAck();
	
	I2CSendByte(addr);//选择写入的地址
	I2CWaitAck();
	
	I2CStart();
	I2CSendByte(0xa1);//选择AT24C02芯片，读模式
	I2CWaitAck();
	
	while(num--){
		*dat++ = I2CReceiveByte();
		if(num)	I2CSendAck(0);//继续读，发送应答
		else	I2CSendAck(1);//不应答
	}
	
	I2CStop();
}

////函数名：写EEPROM函数
////入口参数：需要写入的字符串，写入的地址(务必为8的倍数)，写入数量
////返回值：无
////函数功能：向EERPOM的某个地址写入字符串中特定数量的字符。
//void EEPROM_Write(unsigned char* EEPROM_String, unsigned char addr, unsigned char num)
//{
//	IIC_Start();//发送开启信号
//	IIC_SendByte(0xA0);//选择EEPROM芯片，确定写的模式
//	IIC_WaitAck();//等待EEPROM反馈
//	
//	IIC_SendByte(addr);//写入要存储的数据地址
//	IIC_WaitAck();//等待EEPROM反馈		

//	while(num--)
//	{
//		IIC_SendByte(*EEPROM_String++);//将要写入的信息写入
//		IIC_WaitAck();//等待EEPROM反馈		
//		IIC_Delay(200);	
//	}
//	IIC_Stop();//停止发送	
//}


////函数名：读EEPROM函数
////入口参数：读到的数据需要存储的字符串，读取的地址(务必为8的倍数)，读取的数量
////返回值：无
////函数功能：读取EERPOM的某个地址中的数据，并存放在字符串数组中。
//void EEPROM_Read(unsigned char* EEPROM_String, unsigned char addr, unsigned char num)
//{
//	IIC_Start();//发送开启信号
//	IIC_SendByte(0xA0);//选择EEPROM芯片，确定写的模式
//	IIC_WaitAck();//等待EEPROM反馈
//	
//	IIC_SendByte(addr);//写入要读取的数据地址
//	IIC_WaitAck();//等待EEPROM反馈		

//	IIC_Start();//发送开启信号
//	IIC_SendByte(0xA1);//选择EEPROM芯片，确定读的模式
//	IIC_WaitAck();//等待EEPROM反馈	
//	
//	while(num--)
//	{
//		*EEPROM_String++ = IIC_RecByte();//将要写入的信息写入
//		if(num) IIC_SendAck(0);//发送应答
//			else IIC_SendAck(1);//不应答
//	}
//	
//	IIC_Stop();//停止发送	
//}
