#include "System.h"

//定时变量
u8 icnt;
u8 keycnt,segcnt,ledcnt;
bit flash;

//题设变量
u8 mode = 1;//数码管显示界面切换，1、测距界面；2、参数界面；3、记录界面
bit setmode;//参数界面下的输入模式切换，0：按键模式；1：旋钮模式
u8 dist;//超声波测得距离
u8 PH = 60,PL = 10;//参数上下限
u8 vin;//临时储存电压数据，在键盘函数转化为参数
u8 Ecnt = 0;//报警次数
bit warning = 0;//报警触发标志位，“参数下限≤测量的距离值≤参数上限”变为“测量的距离值>参数上限”或“测量的距离值<参数下限”时置1

void mInfo_Process()
{
	u8 v,temp;
	if(icnt)	return;
	icnt = 1;
	
	temp = Ultra_Read();
	if(temp)	dist = temp;
	if(setmode){
		v = AD_Read(0x43);
		if(v < 51)				vin = 0;
		else if(v < 102)	vin = 1;
		else if(v < 153)	vin = 2;
		else if(v < 204)	vin = 3;
		else							vin = 4;
	}
	
	if(warning == 0 && (dist < PL || dist > PH)){
		warning = 1;
		Ecnt++;
	}
	if(warning && (dist >= PL && dist <= PH))
		warning = 0;
}

void mPset(bit HL)//参数旋钮设置
{
	if(HL){
		switch(vin){
			case 0:PH=50;break;
			case 1:PH=60;break;
			case 2:PH=70;break;
			case 3:PH=80;break;
			case 4:PH=90;break;
		}
	}
	else{
		switch(vin){
			case 0:PL=0;break;
			case 1:PL=10;break;
			case 2:PL=20;break;
			case 3:PL=30;break;
			case 4:PL=40;break;
		}
	}
}

void KBD_Process()
{
	if(keycnt)	return;
	keycnt = 1;
	
	KBD_Read();
	
	if(pres == 0x88){//S4：定义为“切换”，按下S4按键，切换“测距界面”、“参数界面”和“记录界面”。
		if(mode != 3)	mode++;
		else	mode = 1;
		return;
	}
	
	if(mode == 2){
		if(pres == 0x84){//S5：在“参数界面”下，定义为“模式”，按下按键S5，切换参数调整模式。
			setmode = ~setmode;
			return;
		}
		if(~setmode){
			if(pres == 0x44){//S9:“上限调整”
				if(PH == 90)	PH = 50;
				else	PH += 10;
				return;
			}
			if(pres == 0x48){//S8:“下限调整”
				if(PL == 40)	PL = 0;
				else	PL += 10;
				return;
			}
		}
		else{
			if(pres == 0x44){//S9:“上限调整”
				mPset(1);
				return;
			}
			if(pres == 0x48){//S8:“下限调整”
				mPset(0);
				return;
			}
		}
	}
	
	if(mode == 3){
		if(pres == 0x84){//S5：在“记录界面下”，定义为“清零”，按下按键S5，清零当前的报警次数。
			Ecnt = 0;
			return;
		}
	}
}

void SEG_Process()
{
	if(segcnt)	return;
	segcnt = 1;
	
	switch(mode){
		case 1:{//测距界面
			seg_buf[0] = tab[10];//"A"
			seg_buf[1] = 0xff;
			seg_buf[2] = 0xff;
			seg_buf[3] = 0xff;
			seg_buf[4] = 0xff;
			seg_buf[5] = (dist / 100 % 10)?tab[(u8)(dist / 100 % 10)]:0xff;
			seg_buf[6] = tab[(u8)(dist / 10 % 10)];
			seg_buf[7] = tab[(u8)(dist % 10)];
		}break;
		
		case 2:{//参数界面
			seg_buf[0] = 0x8c;//"P"
			seg_buf[1] = setmode?tab[2]:tab[1];
			seg_buf[2] = 0xff;
			seg_buf[3] = tab[PL/10];
			seg_buf[4] = tab[PL%10];
			seg_buf[5] = 0xbf;//"-"
			seg_buf[6] = tab[PH/10];
			seg_buf[7] = tab[PH%10];
		}break;
		
		case 3:{//参数界面
			seg_buf[0] = tab[14];//"E"
			seg_buf[1] = 0Xff;
			seg_buf[2] = 0xff;
			seg_buf[3] = 0xff;
			seg_buf[4] = 0xff;
			seg_buf[5] = 0xff;
			seg_buf[6] = 0xff;
			seg_buf[7] = (Ecnt > 9)?0xbf:tab[Ecnt];
		}break;
	}
}

void LED_Process()
{
	u8 led = 0xff;
	
	if(ledcnt)	return;
	ledcnt = 1;
	
	switch(mode){
		case 1:led &= 0xfe;break;
		case 2:led &= 0xfd;break;
		case 3:led &= 0xfb;break;
	}
	if(~warning && flash)	led &= 0x7f;
	else if(warning)	led &= 0x7f;
	
	Device_Ctrl(0x80,led);
}

void main()
{
	Timer0_Init();
	Device_Init();
	
	while(1)
	{
		mInfo_Process();
		
		KBD_Process();
		SEG_Process();
		LED_Process();
	}
}

void T0ISR(void) interrupt 1
{
	SEG_Display();
	
	if(++icnt >= 100)	{icnt = 0;flash = ~flash;}
	if(++keycnt >= 10)	keycnt = 0;
	if(++segcnt >= 200)	segcnt = 0;
	if(++ledcnt >= 10)	ledcnt = 0;
}
