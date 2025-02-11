#include "Device.h"

void Device_Ctrl(u8 p2data, u8 p0data)
{
	P2 = P2 & 0x1f | p2data;
	P0 = p0data;
	P2 = P2 & 0x1f;
}

void Device_Init()
{
	Device_Ctrl(0xa0,0x00);
	Device_Ctrl(0x80,0xff);
	
	IE2 |= 0x04;        //开定时器2中断
	PT1 = 0;//定时器1最低优先级
	IE = 0x88;//开定时器1中断，开总中断
//	EA = 1;//开总中断
}
