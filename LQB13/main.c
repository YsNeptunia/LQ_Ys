#include "System.h"

//定时变量
u8 icnt;
u8 kbdcnt,segcnt,ledcnt,rcnt;
bit flash = 1;//闪烁标志位

//题设变量
float temper;//当前温度
u8 tempset = 23;//温度参数
u8 rtc[3] = {0x23,0x59,0x50};//实时时间
u8 mode = 1;//数码管显示主模式，1：温度显示界面；2：时间显示界面；3：参数设置界面
bit ctrlmode;//控制工作模式切换。0：温度控制；1：时间控制
bit timemode;//时间显示模式。0：显示时分；1：显示分秒
bit relay;//继电器吸合状态标志
bit sharp;//整点标志，持续5s
bit rflag = 1;//继电器函数标志位，非触发状态为1


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
	
	if(pres == 0x28){//S12：定义为“切换”按键
		if(mode == 3)	mode = 1;
		else	mode++;
		return;
	}
	
	if(pres == 0x24)//S13：定义为“模式”按键
		ctrlmode = ~ctrlmode;
	
	if(mode == 2){
		if(cont == 0x14)
			timemode = 1;
		else
			timemode = 0;
	}
	
	if(mode == 3){
		if(pres == 0x18)//S16：定义为“加”按键
			if(tempset < 99)	tempset++;
		if(pres == 0x14)//S17：定义为“减”按键
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
	Delay750ms();//防止读到脏东西
	
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
