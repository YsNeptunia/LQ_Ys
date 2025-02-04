#include "Ds18B20.h"

float Read_Temper(void)//读取温度数据函数
{
	u8 low,high;//低位，高位
	
	init_ds18b20();//初始化
	Write_DS18B20(0xcc);//跳过rom
	Write_DS18B20(0x44);//进行温度转换
	
	init_ds18b20();//初始化
	Write_DS18B20(0xcc);//跳过rom
	Write_DS18B20(0xbe);//读取温度
	
	low = Read_DS18B20();
	high = Read_DS18B20();
	
	return (((high << 8) | low) / 16.0);//转换为十进制浮点型输出
}
