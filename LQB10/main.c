#include "System.h"

//��ʱ����
u8 btn_cnt,seg_cnt,led_cnt,icnt;//���ٱ���
u16 Timer_500Ms;//����Ƶ��������

//�������
u16 freq;//Ƶ��
float v;//��ȡ��ѹ
float vout = 2.0;//�����ѹ
bit mode = 1;//�������ʾģʽ��0Ƶ�ʣ�1��ѹ
bit vmode;//��ѹ���ģʽ��1Ϊ��ȡ��ѹ��0Ϊ2v
bit ledmode = 1;//led��ʾģʽ��Ϊ1ʱ����ledָʾ��
bit segmode = 1;//�������ʾģʽ��ͬ��


void mInfo_Process()
{
	if(icnt)	return;
	icnt = 1;
	
	v = AD_Read(0x43) * (5.0 / 255);//��Rb2�ĵ�ѹֵ
	if(vmode)	vout = v;
	DA_Write(vout);
}


void BTN_Process()
{
	if(btn_cnt)	return;
	btn_cnt = 1;
	
	BTN_Read();
	
	if(pres == 0x08){//S4������Ϊ����ʾ�����л�������
		mode = ~mode;
		return;
	}
	
	if(pres == 0x04){//S5������Ϊ PCF8591 DAC�����ģʽ�л�������
		if(~vmode){
			vmode = 1;
			return;
		}
		else{
			vmode = 0;
			vout = 2.0;
			return;
		}
	}
	
	if(pres == 0x02){//S6������Ϊ��LED ָʾ�ƹ��ܿ��ơ�����
		ledmode = ~ledmode;
	}
	
	if(pres == 0x01){//S7������Ϊ���������ʾ���ܿ��ơ�����
		segmode = ~segmode;
	}
}

void SEG_Process()
{
	if(seg_cnt)	return;
	seg_cnt = 1;
	
	if(mode){//��ѹ��ʾ
		seg_buf[0] = 0xc1;//"U"
		seg_buf[1] = 0xff;
		seg_buf[2] = 0xff;
		seg_buf[3] = 0xff;
		seg_buf[4] = 0xff;
		seg_buf[5] = tab[(u8)v %10] & 0x7f;//С����
		seg_buf[6] = tab[(u8)(v *10) %10];
		seg_buf[7] = tab[(u8)(v *100) %10];
	}
	else{//Ƶ����ʾ
		seg_buf[0] = tab[15];//"F"
		seg_buf[1] = 0xff;
		seg_buf[2] = (freq / 100000)?tab[(u8)(freq / 100000)%10]:0xff;//��������ʾ������Ϩ��
		seg_buf[3] = (freq / 10000)?tab[(u8)(freq / 10000)%10]:0xff;
		seg_buf[4] = (freq / 1000)?tab[(u8)(freq / 1000)%10]:0xff;
		seg_buf[5] = (freq / 100)?tab[(u8)(freq / 100)%10]:0xff;
		seg_buf[6] = (freq / 10)?tab[(u8)(freq / 10)%10]:0xff;
		seg_buf[7] = freq?tab[(u8)freq%10]:0xff;
	}
}

void LED_Process()
{
	u8 led = 0xff;
	
	if(led_cnt)	return;
	led_cnt = 1;
	
	if(ledmode){
		if(mode)	led = 0xfd;
		else	led = 0xfe;
		
		if((vout >= 1.5 && vout < 2.5) || vout > 3.5)
			led &= 0xfb;
		
		if((freq >= 1000 && freq < 5000) || freq >= 10000)
			led &= 0xf7;
		
		if(vmode)
			led &= 0xef;
	}
	
	Device_Ctrl(0x80,led);
}

void main()
{
	Timer0_Init();
	Timer1_Init();
	Device_Init();
	
	while(1)
	{
		mInfo_Process();
		BTN_Process();
		SEG_Process();
		LED_Process();
	}
}

void T1ISR() interrupt 3
{
	if(segmode)
		SEG_Display();
	else
		Device_Ctrl(0xc0,0x00);
	
	if(++btn_cnt >= 10)		btn_cnt = 0;
	if(++seg_cnt >= 200)	seg_cnt = 0;
	if(++led_cnt >= 10)		led_cnt = 0;
	if(++icnt >= 100)			icnt = 0;
	
	if(++Timer_500Ms >= 500){//0.5S
		Timer_500Ms = 0;
		freq = (TH0 << 8) | TL0;
		TL0 = 0;				//���ö�ʱ��ʼֵ
		TH0 = 0;				//���ö�ʱ��ʼֵ
		freq *= 2;
	}
	
}
