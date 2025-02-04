#include "KBD.h"

u8 KBD_IO()
{
	u8 io = 0xff;
	
	P34 = 1;P35 = 1;P42 = 1;P44 = 1;
	P30 = 0;P31 = 0;P32 = 0;P33 = 0;
	if(P44 == 0)	io = 0x70;
	if(P42 == 0)	io = 0xb0;
	if(P35 == 0)	io = 0xd0;
	if(P34 == 0)	io = 0xe0;
	
	P30 = 1;P31 = 1;P32 = 1;P33 = 1;
	P34 = 0;P35 = 0;P42 = 0;P44 = 0;
	if(P33 == 0)	io |= 0x07;
	if(P32 == 0)	io |= 0x0b;
	if(P31 == 0)	io |= 0x0d;
	if(P30 == 0)	io |= 0x0e;
	
	return io;
}

u8 pres,cont;
void KBD_Read()
{
	u8 state = KBD_IO() ^ 0xff;
	pres = state & (state ^ cont);
	cont = state;
}
