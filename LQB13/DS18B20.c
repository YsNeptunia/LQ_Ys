#include "DS18B20.h"

float Read_Temper(void)//��ȡ�¶����ݺ���
{
	u8 low,high;//��λ����λ
	
	init_ds18b20();//��ʼ��
	Write_DS18B20(0xcc);//����rom
	Write_DS18B20(0x44);//�����¶�ת��
	
	init_ds18b20();//��ʼ��
	Write_DS18B20(0xcc);//����rom
	Write_DS18B20(0xbe);//��ȡ�¶�
	
	low = Read_DS18B20();
	high = Read_DS18B20();
	
	return (((high << 8) | low) / 16.0);//ת��Ϊʮ���Ƹ��������
}
