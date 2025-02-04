#include "Uart.h"

void Uart1Init(void)		//9600bps@12.000MHz
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x01;		//����1ѡ��ʱ��2Ϊ�����ʷ�����
	AUXR &= 0xFB;		//��ʱ��ʱ��12Tģʽ
	T2L = 0xE6;			//���ö�ʱ��ʼֵ
	T2H = 0xFF;			//���ö�ʱ��ʼֵ
	AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
}

void SendByte(u8 dat)//�����ͳ�һ�ֽ�����
{
	SBUF = dat;
	while(TI == 0);
	TI = 0;
}

void Uart_Send(u8* dat)//�����ͳ��ַ���
{
	while(*dat != '\0'){
		SendByte(*dat++);
	}
}
