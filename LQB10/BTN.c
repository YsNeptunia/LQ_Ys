#include "BTN.h"

u8 pres,cont,rels;//按下，长按，松手
void BTN_Read()
{
	u8 state = P3 & 0x0f ^ 0xff;//屏蔽P34以上
	pres = state & (state ^ cont);//按下检测，触发一次
	rels = ~state & (state ^ cont);//松手检测，触发一次
	cont = state;//长按检测，持续触发
}
