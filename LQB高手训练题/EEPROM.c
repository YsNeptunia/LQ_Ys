#include "EEPROM.h"

static void I2C_Delay(unsigned char n)
{
    do{_nop_();}
    while(n--);      	
}

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

////��������дEEPROM����
////��ڲ�������Ҫд����ַ�����д��ĵ�ַ(���Ϊ8�ı���)��д������
////����ֵ����
////�������ܣ���EERPOM��ĳ����ַд���ַ������ض��������ַ���
//void EEPROM_Write(unsigned char* EEPROM_String, unsigned char addr, unsigned char num)
//{
//	IIC_Start();//���Ϳ����ź�
//	IIC_SendByte(0xA0);//ѡ��EEPROMоƬ��ȷ��д��ģʽ
//	IIC_WaitAck();//�ȴ�EEPROM����
//	
//	IIC_SendByte(addr);//д��Ҫ�洢�����ݵ�ַ
//	IIC_WaitAck();//�ȴ�EEPROM����		

//	while(num--)
//	{
//		IIC_SendByte(*EEPROM_String++);//��Ҫд�����Ϣд��
//		IIC_WaitAck();//�ȴ�EEPROM����		
//		IIC_Delay(200);	
//	}
//	IIC_Stop();//ֹͣ����	
//}


////����������EEPROM����
////��ڲ�����������������Ҫ�洢���ַ�������ȡ�ĵ�ַ(���Ϊ8�ı���)����ȡ������
////����ֵ����
////�������ܣ���ȡEERPOM��ĳ����ַ�е����ݣ���������ַ��������С�
//void EEPROM_Read(unsigned char* EEPROM_String, unsigned char addr, unsigned char num)
//{
//	IIC_Start();//���Ϳ����ź�
//	IIC_SendByte(0xA0);//ѡ��EEPROMоƬ��ȷ��д��ģʽ
//	IIC_WaitAck();//�ȴ�EEPROM����
//	
//	IIC_SendByte(addr);//д��Ҫ��ȡ�����ݵ�ַ
//	IIC_WaitAck();//�ȴ�EEPROM����		

//	IIC_Start();//���Ϳ����ź�
//	IIC_SendByte(0xA1);//ѡ��EEPROMоƬ��ȷ������ģʽ
//	IIC_WaitAck();//�ȴ�EEPROM����	
//	
//	while(num--)
//	{
//		*EEPROM_String++ = IIC_RecByte();//��Ҫд�����Ϣд��
//		if(num) IIC_SendAck(0);//����Ӧ��
//			else IIC_SendAck(1);//��Ӧ��
//	}
//	
//	IIC_Stop();//ֹͣ����	
//}
