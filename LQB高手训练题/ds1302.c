/*	# 	DS1302代码片段说明
	1. 	本文件夹中提供的驱动代码供参赛选手完成程序设计参考。
	2. 	参赛选手可以自行编写相关代码或以该代码为基础，根据所选单片机类型、运行速度和试题
		中对单片机时钟频率的要求，进行代码调试和修改。
*/								
#include "ds1302.h"

void Write_Ds1302(unsigned  char temp) 
{
	unsigned char i;
	for (i=0;i<8;i++)     	
	{ 
		SCK = 0;
		SDA = temp&0x01;
		temp>>=1; 
		SCK=1;
	}
}   

void Write_Ds1302_Byte( unsigned char address,unsigned char dat )     
{
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1; 	_nop_();  
 	Write_Ds1302(address);	
 	Write_Ds1302(dat);		
 	RST=0; 
}

unsigned char Read_Ds1302_Byte ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1;	_nop_();
 	Write_Ds1302(address);
 	for (i=0;i<8;i++) 	
 	{		
		SCK=0;
		temp>>=1;	
 		if(SDA)
 		temp|=0x80;	
 		SCK=1;
	} 
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
	SCK=1;	_nop_();
	SDA=0;	_nop_();
	SDA=1;	_nop_();
	return (temp);			
}


void DS1302_Set(u8* time)//DS1302时间设置，指针返回数据
{
	Write_Ds1302_Byte(0x8e,0x00);//打开写保护
	
	Write_Ds1302_Byte(0x84,time[0]);//写时分秒
	Write_Ds1302_Byte(0x82,time[1]);
	Write_Ds1302_Byte(0x80,time[2]);
	
	Write_Ds1302_Byte(0x8e,0x80);//关闭写保护
}

void DS1302_Read(u8* time)//读取DS1302时间，指针返回数据
{
	u8 i;
	for(i = 0; i <3; i++){
		time[i] = Read_Ds1302_Byte(0x85 - i*2);//读时分秒
	}
}
