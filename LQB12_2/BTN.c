#include "BTN.h"

u8 pres,cont,rels;//按下，长按，松手
void BTN_Read()
{
	u8 state = P3 & 0x0f ^ 0xff;//屏蔽高四位防止读到脏东西
	state &= 0x0f;//将高四位清零（异或操作会置1）
	pres = state & (state ^cont);
	rels = ~state & (state ^cont);
	cont = state;
}
