#include "KBD.h"

u8 KBD_IO(void)
{
	u8 io = 0xff;
	
	P32 = 0;P33 = 0;P42 = 1;P44 = 1;//Ö»É¨ÃèËÄ¼ü·ÀÖ¹³åÍ»
	if(P44==0) io=0x70;
	if(P42==0) io=0xb0;
	
	P32 = 1;P33 = 1;P42 = 0;P44 = 0;
	if(P33==0) io=io | 0x07;
	if(P32==0) io=io | 0x0b;
	
	return io;
}

u8 pres = 0,cont = 0;
void KBD_Read(void)
{
	u8 state = KBD_IO() ^ 0xff;
	pres = state & (state ^ cont);
	cont = state;
}
