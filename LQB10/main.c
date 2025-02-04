#include "System.h"

//计时变量
u8 btn_cnt,seg_cnt,led_cnt,icnt;//减速变量
u16 Timer_500Ms;//测量频率用周期

//题设变量
u16 freq;//频率
float v;//读取电压
float vout = 2.0;//输出电压
bit mode = 1;//数码管显示模式，0频率，1电压
bit vmode;//电压输出模式，1为读取电压，0为2v
bit ledmode = 1;//led显示模式，为1时启用led指示灯
bit segmode = 1;//数码管显示模式。同上


void mInfo_Process()
{
	if(icnt)	return;
	icnt = 1;
	
	v = AD_Read(0x43) * (5.0 / 255);//读Rb2的电压值
	if(vmode)	vout = v;
	DA_Write(vout);
}


void BTN_Process()
{
	if(btn_cnt)	return;
	btn_cnt = 1;
	
	BTN_Read();
	
	if(pres == 0x08){//S4：定义为“显示界面切换”按键
		mode = ~mode;
		return;
	}
	
	if(pres == 0x04){//S5：定义为 PCF8591 DAC“输出模式切换”按键
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
	
	if(pres == 0x02){//S6：定义为“LED 指示灯功能控制”按键
		ledmode = ~ledmode;
	}
	
	if(pres == 0x01){//S7：定义为“数码管显示功能控制”按键
		segmode = ~segmode;
	}
}

void SEG_Process()
{
	if(seg_cnt)	return;
	seg_cnt = 1;
	
	if(mode){//电压显示
		seg_buf[0] = 0xc1;//"U"
		seg_buf[1] = 0xff;
		seg_buf[2] = 0xff;
		seg_buf[3] = 0xff;
		seg_buf[4] = 0xff;
		seg_buf[5] = tab[(u8)v %10] & 0x7f;//小数点
		seg_buf[6] = tab[(u8)(v *10) %10];
		seg_buf[7] = tab[(u8)(v *100) %10];
	}
	else{//频率显示
		seg_buf[0] = tab[15];//"F"
		seg_buf[1] = 0xff;
		seg_buf[2] = (freq / 100000)?tab[(u8)(freq / 100000)%10]:0xff;//存在则显示，否则熄灭
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
		TL0 = 0;				//设置定时初始值
		TH0 = 0;				//设置定时初始值
		freq *= 2;
	}
	
}
