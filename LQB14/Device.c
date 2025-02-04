#include "Device.h"

void Device_Ctrl(u8 p2data, u8 p0data)
{
	P2 = P2 & 0x1f | p2data;
	P0 = p0data;
	P2 = P2 & 0x1f;
}

void Device_Init()
{
	Device_Ctrl(0xa0,0x00);
	Device_Ctrl(0x80,0xff);
	
	IE2 |= 0x04;        //����ʱ��2�ж�
	PT1 = 0;//��ʱ��1������ȼ�
	IE = 0x88;//����ʱ��1�жϣ������ж�
//	EA = 1;//�����ж�
}
