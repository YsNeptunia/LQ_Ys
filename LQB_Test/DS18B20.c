#include "DS18B20.h"

float Read_Temper()
{
	u8 l,h;
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	l = Read_DS18B20();
	h = Read_DS18B20();
	
	return (((h << 8) | l) / 16.0);
}
