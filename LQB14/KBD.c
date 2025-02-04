#include "KBD.h"

u8 KBD_IO(void)
{
	u8 io = 0xff;
	
//	P3 = 0xf0;P4 = 0xff;
//	if(P44==0) io=0x70;
//	if(P42==0) io=0xb0;
//	if(P35==0) io=0xd0;
//	if(P34==0) io=0xe0;
//	
//	P3 = 0x0f;P4 = 0x00;
//	if(P33==0) io=io | 0x07;
//	if(P32==0) io=io | 0x0b;
//	if(P31==0) io=io | 0x0d;
//	if(P30==0) io=io | 0x0e;
	
	P32 = 0;P33 = 0;P42 = 1;P44 = 1;//∑¿÷π”ÎNE555≤‚∆µ¬ ≥ÂÕª
	if(P44==0) io=0x70;
	if(P42==0) io=0xb0;
	
	P32 = 1;P33 = 1;P42 = 0;P44 = 0;
	if(P33==0) io=io | 0x07;
	if(P32==0) io=io | 0x0b;
	
	return io;
}

u8 trg = 0,cont = 0;
void KBD_Read(void)
{
	u8 state = KBD_IO() ^ 0xff;
	trg = state & (state ^ cont);
	cont = state;
}
