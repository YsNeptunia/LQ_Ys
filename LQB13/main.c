#include "System.h"

//��ʱ����
u8 icnt;
u8 kbdcnt,segcnt,ledcnt,rcnt;
bit flash = 1;//��˸��־λ

//�������
float temper;//��ǰ�¶�
u8 tempset = 23;//�¶Ȳ���
u8 rtc[3] = {0x23,0x59,0x50};//ʵʱʱ��
u8 mode = 1;//�������ʾ��ģʽ��1���¶���ʾ���棻2��ʱ����ʾ���棻3���������ý���
bit ctrlmode;//���ƹ���ģʽ�л���0���¶ȿ��ƣ�1��ʱ�����
bit timemode;//ʱ����ʾģʽ��0����ʾʱ�֣�1����ʾ����
bit relay;//�̵�������״̬��־
bit sharp;//�����־������5s
bit rflag = 1;//�̵���������־λ���Ǵ���״̬Ϊ1


void mInfo_Process()
{
	if(icnt)	return;
	icnt = 1;
	
	DS1302_Read(rtc);
	temper = Read_Temper();
	
	if(~sharp && rtc[1] == 0 && rtc[2] < 0x05)
		sharp = 1;
	else if(sharp && rtc[2] >= 0x05)
		sharp = 0;
	
	if(ctrlmode){
		if(~relay && rtc[1] == 0 && rtc[2] < 0x05)
			relay = 1;
		else if(relay && rtc[2] >= 0x05)
			relay = 0;
	}
	else{
		if(~relay && temper > tempset)
			relay = 1;
		else if(relay && temper <= tempset)
			relay = 0;
	}
}


void KBD_Process()
{
	if(kbdcnt)	return;
	kbdcnt = 1;
	
	KBD_Read();
	
	if(pres == 0x28){//S12������Ϊ���л�������
		if(mode == 3)	mode = 1;
		else	mode++;
		return;
	}
	
	if(pres == 0x24)//S13������Ϊ��ģʽ������
		ctrlmode = ~ctrlmode;
	
	if(mode == 2){
		if(cont == 0x14)
			timemode = 1;
		else
			timemode = 0;
	}
	
	if(mode == 3){
		if(pres == 0x18)//S16������Ϊ���ӡ�����
			if(tempset < 99)	tempset++;
		if(pres == 0x14)//S17������Ϊ����������
			if(tempset > 10)	tempset--;
	}
	
}

void SEG_Process()
{
	if(segcnt)	return;
	segcnt = 1;
	
	seg_buf[0] = 0xc1;//"U"
	seg_buf[1] = tab[mode];
	switch(mode){
		case 1:{
			seg_buf[2] = 0xff;
			seg_buf[3] = 0xff;
			seg_buf[4] = 0xff;
			seg_buf[5] = tab[(u8)(temper /10) %10];
			seg_buf[6] = tab[(u8)temper %10] & 0x7f;
			seg_buf[7] = tab[(u8)(temper *10) %10];
		}break;
		case 2:{
			seg_buf[2] = 0xff;
			if(~timemode){
				seg_buf[3] = tab[rtc[0] / 16];
				seg_buf[4] = tab[rtc[0] % 16];
				seg_buf[5] = 0xbf;//"-"
				seg_buf[6] = tab[rtc[1] / 16];
				seg_buf[7] = tab[rtc[1] % 16];
			}
			else{
				seg_buf[3] = tab[rtc[1] / 16];
				seg_buf[4] = tab[rtc[1] % 16];
				seg_buf[5] = 0xbf;//"-"
				seg_buf[6] = tab[rtc[2] / 16];
				seg_buf[7] = tab[rtc[2] % 16];
			}
		}break;
		case 3:{
			seg_buf[2] = 0xff;
			seg_buf[3] = 0xff;
			seg_buf[4] = 0xff;
			seg_buf[5] = 0xff;
			seg_buf[6] = tab[tempset /10];
			seg_buf[7] = tab[tempset %10];
		}break;
	}
}

void LED_Process()
{
	u8 led = 0xff;
	
	if(ledcnt)	return;
	ledcnt = 1;
	
	if(sharp)			led &= 0xfe;
	if(~ctrlmode)	led &= 0xfd;
	if(relay && flash)
		led &= 0xfb;
	
	Device_Ctrl(0x80,led);
}

void Relay_Process()
{
	if(rcnt)	return;
	rcnt = 1;
	
	if(relay && rflag){
		Device_Ctrl(0xa0,0x10);
		rflag = 0;
	}
	else if(~relay && ~rflag){
		Device_Ctrl(0xa0,0x00);
		rflag = 1;
	}
}


void Delay750ms(void)	//@12.000MHz
{
	unsigned char data i, j, k;

	_nop_();
	_nop_();
	i = 35;
	j = 51;
	k = 182;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
void main()
{
	Read_Temper();
	Delay750ms();//��ֹ�����ණ��
	
	Timer2_Init();
	Device_Init();
	DS1302_Set(rtc);
	while(1)
	{
		mInfo_Process();
		
		KBD_Process();
		SEG_Process();
		LED_Process();
		Relay_Process();
	}
}

void I2ISR()	interrupt 12
{
	static u8 cnt;
	SEG_Display();
	
	if(++icnt >= 50)		icnt = 0;
	if(++kbdcnt >= 10)	kbdcnt = 0;
	if(++segcnt >= 200)	segcnt = 0;
	if(++ledcnt >= 10)	ledcnt = 0;
	if(++rcnt >= 10)		rcnt = 0;
	
	if(relay){
		if(++cnt >= 100){
			flash = ~flash;
			cnt = 0;
		}
	}
	else if(cnt)
		cnt = 0;
}
