#include "System.h"

//计时变量
u8 icnt;
u8 keycnt,segcnt;
u16 Timer_1000ms;
u8 Timer_200ms;
bit flash;

//题设变量
u8 mode = 1;//显示模式切换。1~4：频率、参数、时间和回显四个界面
bit pmode = 0;//参数显示切换
bit echo = 0;//回显模式切换。0~1：频率回显和时间回显两个子界面
u16 freq;//原始频率
long f;//显示频率
u16 p1 = 2000;//超限参数
int p2 = 0;//校准值参数
u8 rtc[3] = {0x13,0x03,0x05};//实时时间
long fmax = 0;//回显最大频率
u8 tmax[3];//回显最大频率发生时间
float v;//DAC输出电压

void mInfo_Process()
{
	u8 i;
	
	if(icnt)	return;
	icnt = 1;

	for(i = 0; i < 3; i++){
		DS1302_Read(rtc);
	}
	
	if(f < 0)	DA_Write(0x00);
	else{
		if(f < 500)	v = 1.0;
		else if(f < p1)	v = (4.0/(p1-500)) * f;
		else	v = 5.0;
		DA_Write((u8)(v * 51));
	}
	
	if(f > fmax){
		fmax = f;
		for(i = 0; i < 3; i++){
			tmax[i] = rtc[i];
		}
	}
}



void KBD_Process()
{
	if(keycnt)	return;
	keycnt = 1;

	KBD_Read();
	
	if(pres == 0x88){
		if(++mode > 4)
			mode = 1;
		pmode = 0;
		echo = 0;
		return;
	}
	
	if(pres == 0x84){
		if(mode == 2)
			pmode = ~pmode;
		if(mode == 4)
			echo = ~echo;
		return;
	}
	
	if(mode == 2){
		if(pres == 0x48){
			if(pmode){
				if(p2 != 900)
					p2 += 100;
			}
			else{
				if(p1 != 9000)
					p1 += 1000;
			}
			return;
		}
		if(pres == 0x44){
			if(pmode){
				if(p2 != -900)
					p2 -= 100;
			}
			else{
				if(p1 != 1000)
					p1 -= 1000;
			}
			return;
		}
	}
}

void SEG_Process()
{
	u8 i;
	u8 buf[8];//暂存
	
	if(segcnt)	return;
	segcnt = 1;

	switch(mode)
	{
		case 1:{
			seg_buf[0] = tab[15];
			seg_buf[1] = 0xff;
			seg_buf[2] = 0xff;
			
			if(f >= 0){
				buf[3] = tab[(u8)(f / 10000 % 10)];
				buf[4] = tab[(u8)(f / 1000 % 10)];
				buf[5] = tab[(u8)(f / 100 % 10)];
				buf[6] = tab[(u8)(f / 10 % 10)];
				buf[7] = tab[(u8)(f % 10)];
				i = 3;
				while(buf[i] == tab[0]){
					buf[i] = 0xff;//熄灭无效位
					if(++i == 7)	break;//保证最后一位点亮
				}
				for(i = 3; i < 8; i++){
					seg_buf[i] = buf[i];
				}
			}
			else{
				seg_buf[3] = 0xff;
				seg_buf[4] = 0xff;
				seg_buf[5] = 0xff;
				seg_buf[6] = 0xc7;//"L"
				seg_buf[7] = 0xc7;//"L"
			}
		}break;
		
		case 2:{
			seg_buf[0] = 0x8c;//"P"
			seg_buf[1] = pmode?tab[2]:tab[1];
			seg_buf[2] = 0xff;
			seg_buf[3] = 0xff;
			if(~pmode){
				seg_buf[4] = tab[(u8)(p1 /1000 %10)];
				seg_buf[5] = tab[(u8)(p1 /100 %10)];
				seg_buf[6] = tab[(u8)(p1 /10 %10)];
				seg_buf[7] = tab[(u8)(p1 %10)];
			}
			else{
				seg_buf[4] = (p2 >= 0)?0xff:0xbf;//显示负号与否
				if(p2){
					seg_buf[5] = tab[(u8)(abs(p2) /100 %10)];
					seg_buf[6] = tab[(u8)(abs(p2) /10 %10)];
					seg_buf[7] = tab[(u8)(abs(p2) %10)];
					}
				else{
					seg_buf[5] = 0xff;
					seg_buf[6] = 0xff;
					seg_buf[7] = tab[0];
				}
			}
		}break;
		
		case 3:{
			seg_buf[0] = tab[rtc[0]/16];
			seg_buf[1] = tab[rtc[0]%16];
			seg_buf[2] = 0xbf;// "-"
			seg_buf[3] = tab[rtc[1]/16];
			seg_buf[4] = tab[rtc[1]%16];
			seg_buf[5] = 0xbf;// "-"
			seg_buf[6] = tab[rtc[2]/16];
			seg_buf[7] = tab[rtc[2]%16];
		}break;
		
		case 4:{
			seg_buf[0] = 0x89;//"H"
			seg_buf[1] = echo?tab[10]:tab[15];
			if(~echo){
				seg_buf[2] = 0xff;
				buf[3] = tab[(u8)(fmax / 10000 % 10)];
				buf[4] = tab[(u8)(fmax / 1000 % 10)];
				buf[5] = tab[(u8)(fmax / 100 % 10)];
				buf[6] = tab[(u8)(fmax / 10 % 10)];
				buf[7] = tab[(u8)(fmax % 10)];
				i = 3;
				while(buf[i] == tab[0]){
					buf[i] = 0xff;//熄灭无效位
					if(++i == 7)	break;//保证最后一位点亮
				}
				for(i = 3; i < 8; i++){
					seg_buf[i] = buf[i];
				}
			}
			else{
			seg_buf[2] = tab[tmax[0]/16];
			seg_buf[3] = tab[tmax[0]%16];
			seg_buf[4] = tab[tmax[1]/16];
			seg_buf[5] = tab[tmax[1]%16];
			seg_buf[6] = tab[tmax[2]/16];
			seg_buf[7] = tab[tmax[2]%16];
			}
		}break;
	}
}

u8 led_old;
void LED_Process()
{
	u8 led = 0xff;
	
	if(mode == 1 && flash) led &= 0xfe;
	if(f > p1 && flash) led &= 0xfd;
	if(f < 0)	led &= 0xfd;
	
	if(led_old != led){
		led_old = led;
		Device_Ctrl(0x80,led);
	}
}

void main()
{
	Timer0_Init();
	Timer2_Init();
	DS1302_Set(rtc);
	Device_Init();
	
	while(1)
	{
		mInfo_Process();
		KBD_Process();
		SEG_Process();
		LED_Process();
	}
}

void T2ISR(void) interrupt 12
{
	SEG_Display();
	
	if(++icnt >= 200)		icnt = 0;
	if(++keycnt >= 10)	keycnt = 0;
	if(++segcnt >= 50)	segcnt = 0;
	
	if(++Timer_1000ms >= 1000){//读频率，周期
		Timer_1000ms = 0;
		TR0 = 0;
		freq = (TH0 << 8) | TL0;
		f = (long)(freq + (long)p2);
		TH0 = 0;
		TL0 = 0;
		TR0 = 1;
	}
	
	if(++Timer_200ms >= 200){
		Timer_200ms = 0;
		if((mode == 1) || (f > p1))
			flash = ~flash;
	}
}
