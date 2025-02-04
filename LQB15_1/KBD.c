#include "KBD.h"

u8 KBD_IO()
{
	u8 io = 0xff;
	
	P32 = 0;P33 = 0;P42 = 1;P44 = 1;//∑¿÷π”ÎNE555≤‚∆µ¬ ≥ÂÕª
	if(P44==0) io=0x70;
	if(P42==0) io=0xb0;
	
	P32 = 1;P33 = 1;P42 = 0;P44 = 0;
	if(P33==0) io|= 0x07;
	if(P32==0) io|= 0x0b;
	
	return io;
}

u8 pres,cont,rels;
void KBD_Read()
{
	u8 state = KBD_IO() ^ 0xff;
	pres = state & (state ^ cont);
	rels = ~state & (state ^ cont);
	cont = state;
}
