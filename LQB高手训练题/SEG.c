#include "SEG.h"
/*(Nixie tubes) segment 数码管*/

u8 seg_buf[8];//数码管缓存
code unsigned char tab[] =
{
0xc0, //0
0xf9, //1
0xa4, //2
0xb0, //3
0x99, //4
0x92, //5
0x82, //6
0xf8, //7
0x80, //8
0x90, //9
0x88, //A
0x83, //b
0xc6, //C
0xa1, //d
0x86, //E
0x8e //F
};

void SEG_Display()
{
	static u8 i = 0;
	Device_Ctrl(0xc0,0x00);//消影，关闭所有位选
	Device_Ctrl(0xe0,seg_buf[i]);//段选
	Device_Ctrl(0xc0,0x01 << i);//位选
	i = (i + 1) % 8;
}