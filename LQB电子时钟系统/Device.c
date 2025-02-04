#include "Device.h"

void Device_Ctrl(u8 p2data, u8 p0data)//锁存器控制函数
/*	Y4(LED)=0x80; Y5(BEEP)=0xA0;	 
	Y6(位选)=0xC0; Y7(段选)=0xE0;	*/
{
	P2 = P2 & 0x1f | p2data;	//P2锁存器打开，前三位先清零，后五位不动。然后再赋值
	P0 = p0data;				//P0口赋值
	P2 = P2 & 0x1f;				//P2锁存器关闭，前三位清零，后五位不动
}

void Device_Init()//初始化外设驱动，开启中断等
{
	Device_Ctrl(0xa0,0x00);
	Device_Ctrl(0x80,0xff);
	
//	IE2 |= 0x04;    //开定时器2中断
	ET0 = 1;//开定时器0中断
	ES = 1;//开串口中断
	EA = 1;//开总中断
}