#include "EEPROM.h"

void EEPROM_Write(u8* dat,u8 addr,u8 num)//�βΣ���Ҫд����ַ�����д��ĵ�ַ��8��������д�������
{
	I2CStart();
	I2CSendByte(0xa0);//ѡ��AT24C02оƬ��дģʽ
	I2CWaitAck();
	
	I2CSendByte(addr);//ѡ��д��ĵ�ַ
	I2CWaitAck();
	
	while(num--){
		I2CSendByte(*dat++);//д������
		I2CWaitAck();
		I2C_Delay(200);
	}
	
	I2CStop();
}

void EEPROM_Read(u8* dat,u8 addr,u8 num)//�βΣ���Ҫ�������ַ��������ĵ�ַ��8����������������
{
	I2CStart();
	I2CSendByte(0xa0);//ѡ��AT24C02оƬ��дģʽ
	I2CWaitAck();
	
	I2CSendByte(addr);//ѡ��д��ĵ�ַ
	I2CWaitAck();
	
	I2CStart();
	I2CSendByte(0xa1);//ѡ��AT24C02оƬ����ģʽ
	I2CWaitAck();
	
	while(num--){
		*dat++ = I2CReceiveByte();
		if(num)	I2CSendAck(0);//������������Ӧ��
		else	I2CSendAck(1);//��Ӧ��
	}
	
	I2CStop();
}
