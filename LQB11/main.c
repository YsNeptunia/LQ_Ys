#include "System.h"

//��ʱ����
u8 icnt;
u8 keycnt,segcnt,ledcnt;
u16 time5s = 0;//5s��ʱ����
bit flag5s = 0;//5s��־λ

//�������
u8 mode = 1;//��ģʽ�л���1) ���ݽ���2) ��������3) ��������
float vin;//AIN3 ͨ���ϻ�ȡ�ĵ�ѹ�ź�
float vp;//��ѹ��������E2PROM��ȡ
u8 vtemp;//��ʱ�洢�ַ���
bit vflag = 1;//��ѹ������һ��־λ���ú����
u32 Ncnt = 0;//����ֵ(0~9999999)
u8 nkey;//��Ч��������


void mInfo_Process()
{

	if(icnt)	return;
	icnt = 1;

	vin = AD_Read(0x43) * (5.00 / 255);
	
	if(vflag && vp >= vin){
		vflag = 0;
		if(Ncnt != 9999999)
			Ncnt++;
	}
	else if(vp < vin)
		vflag = 1;
}


void KBD_Process()
{
	if(keycnt)	return;
	keycnt = 1;

	KBD_Read();
	
	if(pres == 0x28){//S12������Ϊ����ʾ�����л�������
		nkey = 0;
		
		if(mode == 2){
			vtemp = (u8)(vp * 10);
			EEPROM_Write(&vtemp,0,8);
		}
		
		if(mode == 3)	mode = 1;
		else	mode++;
		return;
	}
	else if(mode == 3 && pres == 0x24){//S13������Ϊ�����㡱����
		nkey = 0;
		Ncnt = 0;
		return;
	}
	else if(mode == 2 && pres == 0x18){
		nkey = 0;
		if(vp != 5.0)	vp += 0.5;
		else	vp = 0.0;
		return;
	}
	else if(mode == 2 && pres == 0x14){
		nkey = 0;
		if(vp != 0.0)	vp -= 0.5;
		else	vp = 5.0;
		return;
	}
	else{
		if(pres != 0x00)
			nkey++;
	}
}

void SEG_Process()
{
	u8 i = 1;
	u8 buf[8];//mode3ר�û���
	
	if(segcnt)	return;
	segcnt = 1;

	if(mode == 1 || mode == 2){
		seg_buf[1] = 0xff;
		seg_buf[2] = 0xff;
		seg_buf[3] = 0xff;
		seg_buf[4] = 0xff;
	}
	switch(mode){
		case 1:{
			seg_buf[0] = 0xc1;//"U"
			seg_buf[5] = tab[(u8)vin%10] & 0x7f;
			seg_buf[6] = tab[(u16)(vin*10)%10];
			seg_buf[7] = tab[(u16)(vin*100)%10];
		};break;
		
		case 2:{
			seg_buf[0] = 0x8c;//"P"
			seg_buf[5] = tab[(u8)vp %10] & 0x7f;
			seg_buf[6] = tab[(u16)(vp*10)%10];
			seg_buf[7] = tab[(u16)(vp*100)%10];
		};break;
		
		case 3:{
			seg_buf[0] = 0xc8;//"N"
			
			buf[1] = tab[(Ncnt/1000000)%10];
			buf[2] = tab[(Ncnt/100000)%10];
			buf[3] = tab[(Ncnt/10000)%10];
			buf[4] = tab[(Ncnt/1000)%10];
			buf[5] = tab[(Ncnt/100)%10];
			buf[6] = tab[(Ncnt/10)%10];
			buf[7] = tab[Ncnt%10];
			
			while(buf[i] == tab[0]){
				buf[i] = 0xff;//Ϩ���λ
				if(++i == 7)	break;//��֤���λ����
			}
			
			for(i = 1; i < 8; i++){
				seg_buf[i] = buf[i];//����ʾ�������������
			}
		};break;
	}
}

void LED_Process()
{
	static u8 led_old;
	u8 led = 0xff;
	
	if(ledcnt)	return;
	ledcnt = 1;

	if(flag5s)
		led &= 0xfe;
	
	if(Ncnt % 2)
		led &= 0xfd;
	
	if(nkey >= 3)
		led &= 0xfb;
	
	if(led != led_old){
		led_old = led;
		Device_Ctrl(0x80,led);
	}
}



void main()
{
	Timer2_Init();
	Device_Init();
	AD_Read(0x43);
	EEPROM_Read(&vtemp,0,8);
	vp = vtemp * 0.1;
	
	while(1)
	{
		mInfo_Process();
		
		KBD_Process();
		SEG_Process();
		LED_Process();
	}
}

void T2ISR() interrupt 12
{
	SEG_Display();
	
	if(++icnt >= 10)		icnt = 0;
	if(++keycnt >= 10)	keycnt = 0;
	if(++segcnt >= 150)	segcnt = 0;
	if(++ledcnt >= 10)	ledcnt = 0;
	
	if(vin < vp){
		time5s++;
		if(time5s >= 5000){
			time5s = 0;
			flag5s = 1;
		}
	}
	else{
		time5s = 0;
		flag5s = 0;
	}
}
