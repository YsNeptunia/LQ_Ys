#include "System.h"

//��ʱ����
u8 icnt;
u8 keycnt,segcnt,ledcnt,bcnt;

//�������
bit mainmode = 0;//�������ʾ�����л���0:ʱ�估ʪ��������ʾ;	1:ʪ����ֵ�趨����
bit automode = 1;//�Զ��ֶ�״̬�л���1ʱ����
u8 rtc[3] = {0x08,0x30,0x00};//ʵʱʱ��
u8 wet,value = 50;//ʪ�ȣ���ֵʪ��
u8 temp;//����ʱ����ֵ�¶�
bit beepflag = 1;//������ʹ�ܱ�־λ
bit watering = 0;//���ʹ�ܱ�־λ
bit warning = 0;//������־λ
u8 ledstate = 0xff;//led״̬�������
u8 ctrlstate = 0x00;//�������״̬������������仯ʱ����������

/*�������������������*/

void mInfo_Process()//ϵͳ���ݴ�����
{
	u8 v;
	
	if(icnt)	return;
	icnt = 1;
	
	DS1302_Read(rtc);//ʵʱʱ��
	v = AD_Read(0x43);//��ȡRb2��ѹ
	wet = (v / 255.0) * 99;//��ѹ����ת��Ϊʪ��
	
	if(automode){//�Զ�ģʽ
		if(wet < value)	watering = 1;
		else	watering = 0;
	}
	else{//�ֶ�ģʽ
		if(wet < value)	warning = 1;
		else	warning = 0;
	}
}


/*����Ӳ��������������*/

void BTN_Process()
{
	if(keycnt)	return;
	keycnt = 1;
	
	BTN_Read();
	
	if(pres == 0x01)	automode = ~automode;//���� S7 �趨Ϊϵͳ����״̬�л�������
	if(automode){//�Զ�ģʽ
		if(pres == 0x02){
			mainmode = ~mainmode;//���� S6 �󣬽���ʪ����ֵ��������
			if(mainmode){ 
				EEPROM_Read(&value,0,2);
				temp = value;
			}
			else{
				EEPROM_Write(&temp,0,2);
				value = temp;
			}
		}
		if(mainmode){//���� S5 Ϊʪ����ֵ�� 1������ S4 ʪ����ֵ�� 1��
			if(pres == 0x04)	temp++;
			if(pres == 0x08)	temp--;
		}
	}
	else{//�ֶ�ģʽ
		if(pres == 0x02)	beepflag = ~beepflag;//���� S6 �رշ��������ѹ��ܣ��ٴΰ��� S6 �򿪷��������ѹ���
		if(pres == 0x04)	watering = 1;//S5 �����趨Ϊ�򿪹��ϵͳ
		if(pres == 0x08)	watering = 0;//S4 �����趨Ϊ�رչ��ϵͳ
	}
	
}

void SEG_Process()
{
	if(segcnt)	return;
	segcnt = 1;
	
	if(~mainmode){
		seg_buf[0] = tab[rtc[0] / 16];
		seg_buf[1] = tab[rtc[0] % 16];
		seg_buf[2] = 0xbf;//"-"
		seg_buf[3] = tab[rtc[1] / 16];
		seg_buf[4] = tab[rtc[1] % 16];
		seg_buf[5] = 0xff;//Ϩ��
		seg_buf[6] = tab[wet / 10 % 10];
		seg_buf[7] = tab[wet % 10];
	}
	else{
		seg_buf[0] = 0xbf;//"-"
		seg_buf[1] = 0xbf;//"-"
		seg_buf[2] = 0xbf;//"-"
		seg_buf[3] = 0xff;//Ϩ��
		seg_buf[4] = 0xff;//Ϩ��
		seg_buf[5] = 0xff;//Ϩ��
		seg_buf[6] = tab[temp / 10 % 10];
		seg_buf[7] = tab[temp % 10];
	}
}

void LED_Process()
{
	u8 led;
	
	if(ledcnt)	return;
	ledcnt = 1;
	
	if(automode)	led = 0xfe;
	else led = 0xfd;
	
	if(ledstate != led){
		ledstate = led;
		Device_Ctrl(0x80,led);
	}
}

void BEEP_Process()
{
	u8 ctrl = 0x00;//�̵�������������
	
	if(bcnt)	return;
	bcnt = 1;
	
	if(watering)	ctrl |= 0x10;
	if(~automode && beepflag && warning)	ctrl |= 0x40;
	
	if(ctrl != ctrlstate){//�б仯ʱ����
		ctrlstate = ctrl;
		Device_Ctrl(0xa0,ctrl);
	}
}

//������
void main()
{
	Timer0_Init();
	Device_Init();
	DS1302_Set(rtc);
//	EEPROM_Write(&value,0,2);
	
	while(1)
	{
		mInfo_Process();
		
		BTN_Process();
		SEG_Process();
		LED_Process();
		BEEP_Process();
	}
}

//�ж���
void T0ISR(void) interrupt 1
{
	if(++ledcnt >= 10)	ledcnt = 0;
	if(++keycnt >= 10)	keycnt = 0;
	if(++icnt >= 100)		icnt = 0;
	if(++segcnt >= 200)	segcnt = 0;
	if(++bcnt >= 200)		bcnt = 0;
	
	SEG_Display();
}
