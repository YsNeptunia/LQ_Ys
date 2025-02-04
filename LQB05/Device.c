#include "Device.h"

void Device_Ctrl(u8 p2data, u8 p0data)
{
	P2 = P2 & 0x1f | p2data;
	P0 = p0data;
	P2 = P2 & 0x1f;
}

void Device_Init(void)
{
	Device_Ctrl(0xa0,0x00);
	Device_Ctrl(0x80,0xff);
	
	ET0 = 1;
	EA = 1;
}

