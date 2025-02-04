#include "System.h"

//��ʱ����
u8 icnt;
u8 keycnt,segcnt,ledcnt;
u16 Timer_1000ms;//1s��ʱ����
u8 pcnt = 0;//������ʱ����
bit keyflag;//������־λ����������1s����1

//�������
u8 mode = 1;//��ģʽ��1) Ƶ����ʾ����,2) ������ʾ����,3) ��ѹ��ʾ����
u16 freq;//Ƶ��
u32 T;//����
bit chl;//��ѹͨ���л���־λ����������0����λ��1��
float U1,U3;//�ɼ����ĵ�ѹֵ����������ͨ�����Ϊ 1����λ��ͨ�����Ϊ 3��
float utemp;//��ѹ����
u16 ftemp;//Ƶ�ʻ���
bit ledflag;//ledʹ�ܱ�־λ����1ʱledȫϨ��
u8 led_old = 0xff;//��һled״̬


void mInfo_Process()
{
	if(icnt)	return;
	icnt = 1;

	U1 = AD_Read(0x43) * 1.0/ 51;//ͬʱ��ȡ�����
	U3 = AD_Read(0x41) * 1.0/ 51;
}



void BTN_Process()
{
	if(keycnt)	return;
	keycnt = 1;

	BTN_Read();
	
	if(pres == 0x08){//S4������Ϊ�����桱����
		if(mode == 2)	chl = 0;
		if(mode == 3)	mode = 1;
		else	mode++;
		return;
	}
	
	if(mode == 3 && pres == 0x04){//S5������Ϊ��ѹͨ���л�����
		chl = ~chl;
		return;
	}
	
	if(pres == 0x02){//S6������Ϊͨ�� 3 ��ѹ�����桱����
		utemp = U3;
		return;
	}
	
	if(pres == 0x01){//S7������ΪƵ�ʡ����桱����
		ftemp = freq;
		return;
	}
	
	if(cont == 0x01){//S7 ���������ܣ����� LEDָʾ�ƹ���
		if(++pcnt >= 100){
			pcnt = 0;
			keyflag = 1;
		}
	}
	else	pcnt = 0;
	
	if(keyflag && rels == 0x01){//���� S7 �������� 1 ����ɿ�����
		keyflag = 0;
		ledflag = ~ledflag;
	}
}

void SEG_Process()
{
	u8 i;
	u8 buf[8];//�ݴ棬���������segbuf
	
	if(segcnt)	return;
	segcnt = 1;

	switch(mode){
		case 1:{
			seg_buf[0] = tab[15];//"F"
			seg_buf[1] = 0xff;
			seg_buf[2] = 0xff;//���ֵ65535�������ܳ�����λ��Ϩ��
			
			buf[3] = tab[(u8)(freq/10000%10)];
			buf[4] = tab[(u8)(freq/1000%10)];
			buf[5] = tab[(u8)(freq/100%10)];
			buf[6] = tab[(u8)(freq/10%10)];
			buf[7] = tab[(u8)(freq%10)];
			
			i = 3;
			while(buf[i] == tab[0]){
				buf[i] = 0xff;//Ϩ����Чλ
				if(++i == 7)	break;//��֤���һλ����
			}
			for(i = 3; i < 8; i++){
				seg_buf[i] = buf[i];
			}
		}break;
		
		case 2:{
			seg_buf[0] = 0xc8;//"N"
			
			buf[1] = tab[(u8)(T/1000000%10)];
			buf[2] = tab[(u8)(T/100000%10)];
			buf[3] = tab[(u8)(T/10000%10)];
			buf[4] = tab[(u8)(T/1000%10)];
			buf[5] = tab[(u8)(T/100%10)];
			buf[6] = tab[(u8)(T/10%10)];
			buf[7] = tab[(u8)(T%10)];
			
			i = 1;
			while(buf[i] == tab[0]){
				buf[i] = 0xff;//Ϩ����Чλ
				if(++i == 7)	break;//��֤���һλ����
			}
			for(i = 3; i < 8; i++){
				seg_buf[i] = buf[i];
			}
		}break;
		
		case 3:{
			seg_buf[0] = 0xc1;//"U"
			seg_buf[1] = 0xbf;//"-"
			seg_buf[3] = seg_buf[4] = 0xff;
			
			if(~chl){
				seg_buf[2] = tab[1];
				seg_buf[5] = tab[(u8)U1 %10] & 0x7f;
				seg_buf[6] = tab[(u16)(U1 *10) %10];
				seg_buf[7] = tab[(u16)(U1 *100) %10];
			}
			else{
				seg_buf[2] = tab[3];
				seg_buf[5] = tab[(u8)U3] & 0x7f;
				seg_buf[6] = tab[(u16)(U3 *10) %10];
				seg_buf[7] = tab[(u16)(U3 *100) %10];
			}
		}break;
	}
}

void LED_Process()
{
	u8 led = 0xff;
	
	if(ledcnt)	return;
	ledcnt = 1;

	
	if(utemp && U3 > utemp)		led &= 0xfe;
	if(ftemp && freq > ftemp)	led &= 0xfd;
	switch(mode){
		case 1:led &= 0xfb;break;
		case 2:led &= 0xf7;break;
		case 3:led &= 0xef;break;
	}
	
	if(ledflag)	led = 0xff;
	
	if(led != led_old){
		led_old = led;
		Device_Ctrl(0x80,led);
	}
}



void Delay1000ms(void)	//@12.000MHz
{
	unsigned char data i, j, k;

	_nop_();
	_nop_();
	i = 46;
	j = 153;
	k = 245;
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
	Timer0_Init();
	Timer2_Init();
	Device_Init();
	AD_Read(0x41);
	Delay1000ms();
	
	while(1)
	{
		mInfo_Process();
		
		BTN_Process();
		SEG_Process();
		LED_Process();
	}
}

void T2ISR(void) interrupt 12
{
	SEG_Display();
	
	if(++icnt >= 200)		icnt = 0;
	if(++keycnt >= 10)	keycnt = 0;
	if(++segcnt >= 200)	segcnt = 0;
	if(++ledcnt >= 1)	ledcnt = 0;
	
	if(++Timer_1000ms >= 1000){//��Ƶ�ʣ�����
		Timer_1000ms = 0;
		TR0 = 0;
		freq = (TH0 << 8) | TL0;
		T = (u32)((float)(1.0/freq) * 1000000);
		TH0 = 0;
		TL0 = 0;
		TR0 = 1;
	}
}