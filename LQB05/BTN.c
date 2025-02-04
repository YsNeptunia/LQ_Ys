#include "BTN.h"

u8 pres,cont,rels;//按下，长按，松手
void BTN_Read()
{
	u8 state = P3 & 0x0f ^ 0xff;
	pres = state & (state ^cont);
	rels = ~state & (state ^cont);
	cont = state;
}
