#include "BTN.h"

u8 pres,cont,rels;//���£�����������
void BTN_Read()
{
	u8 state = P3 & 0x0f ^ 0xff;//����P34����
	pres = state & (state ^ cont);//���¼�⣬����һ��
	rels = ~state & (state ^ cont);//���ּ�⣬����һ��
	cont = state;//������⣬��������
}
