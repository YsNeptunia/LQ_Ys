#include "KBD.h"
/*Keyboard �������*/

u8 trg,cont;//˲ʱ״̬������״̬

u8 KBD_IO()//�������ɨ�躯��
{
	u8 io = 0xff;
	
	//��ɨ��
	P3 = 0xf0;P4 = 0xff;
	if(P44 == 0)	io = 0x70;
	if(P42 == 0)	io = 0xb0;
	if(P35 == 0)	io = 0xd0;
	if(P34 == 0)	io = 0xe0;
	
	//��ɨ��
	P3 = 0x0f;P4 = 0x00;
	if(P33 == 0)	io = io | 0x07;
	if(P32 == 0)	io = io | 0x0b;
	if(P31 == 0)	io = io | 0x0d;
	if(P30 == 0)	io = io | 0x0e;
	
	return io;
}

void KBD_Read()//��ȡ�������״̬����
{
	u8 state = KBD_IO() ^ 0xff;
	trg = state & (state ^ cont);
	cont = state;
}