#include "Device.h"

void Device_Ctrl(u8 p2data, u8 p0data)//���������ƺ���
/*	Y4(LED)=0x80; Y5(BEEP)=0xA0;	 
	Y6(λѡ)=0xC0; Y7(��ѡ)=0xE0;	*/
{
	P2 = P2 & 0x1f | p2data;	//P2�������򿪣�ǰ��λ�����㣬����λ������Ȼ���ٸ�ֵ
	P0 = p0data;				//P0�ڸ�ֵ
	P2 = P2 & 0x1f;				//P2�������رգ�ǰ��λ���㣬����λ����
}

void Device_Init()//��ʼ�����������������жϵ�
{
	Device_Ctrl(0xa0,0x00);
	Device_Ctrl(0x80,0xff);
	
//	IE2 |= 0x04;    //����ʱ��2�ж�
	ET0 = 1;//����ʱ��0�ж�
	ES = 1;//�������ж�
	EA = 1;//�����ж�
}