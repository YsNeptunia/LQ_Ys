#include "System.h"

//计时变量
u8 icnt;
u8 keycnt,segcnt,ledcnt;
u16 Timer_1000ms;//1s计时变量
u8 pcnt = 0;//长按计时变量
bit keyflag;//长按标志位，长按超过1s后置1

//题设变量
u8 mode = 1;//主模式。1) 频率显示界面,2) 周期显示界面,3) 电压显示界面
u16 freq;//频率
u32 T;//周期
bit chl;//电压通道切换标志位。光敏电阻0，电位器1。
float U1,U3;//采集到的电压值。光敏电阻通道编号为 1，电位器通道编号为 3。
float utemp;//电压缓存
u16 ftemp;//频率缓存
bit ledflag;//led使能标志位，置1时led全熄灭
u8 led_old = 0xff;//上一led状态


void mInfo_Process()
{
	if(icnt)	return;
	icnt = 1;

	U1 = AD_Read(0x43) * 1.0/ 51;//同时读取需调换
	U3 = AD_Read(0x41) * 1.0/ 51;
}



void BTN_Process()
{
	if(keycnt)	return;
	keycnt = 1;

	BTN_Read();
	
	if(pres == 0x08){//S4：定义为“界面”按键
		if(mode == 2)	chl = 0;
		if(mode == 3)	mode = 1;
		else	mode++;
		return;
	}
	
	if(mode == 3 && pres == 0x04){//S5：定义为电压通道切换按键
		chl = ~chl;
		return;
	}
	
	if(pres == 0x02){//S6：定义为通道 3 电压“缓存”按键
		utemp = U3;
		return;
	}
	
	if(pres == 0x01){//S7：定义为频率“缓存”按键
		ftemp = freq;
		return;
	}
	
	if(cont == 0x01){//S7 长按键功能，禁用 LED指示灯功能
		if(++pcnt >= 100){
			pcnt = 0;
			keyflag = 1;
		}
	}
	else	pcnt = 0;
	
	if(keyflag && rels == 0x01){//长按 S7 按键超过 1 秒后松开按键
		keyflag = 0;
		ledflag = ~ledflag;
	}
}

void SEG_Process()
{
	u8 i;
	u8 buf[8];//暂存，处理后输入segbuf
	
	if(segcnt)	return;
	segcnt = 1;

	switch(mode){
		case 1:{
			seg_buf[0] = tab[15];//"F"
			seg_buf[1] = 0xff;
			seg_buf[2] = 0xff;//最大值65535，不可能超过五位故熄灭
			
			buf[3] = tab[(u8)(freq/10000%10)];
			buf[4] = tab[(u8)(freq/1000%10)];
			buf[5] = tab[(u8)(freq/100%10)];
			buf[6] = tab[(u8)(freq/10%10)];
			buf[7] = tab[(u8)(freq%10)];
			
			i = 3;
			while(buf[i] == tab[0]){
				buf[i] = 0xff;//熄灭无效位
				if(++i == 7)	break;//保证最后一位点亮
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
				buf[i] = 0xff;//熄灭无效位
				if(++i == 7)	break;//保证最后一位点亮
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
	
	if(++Timer_1000ms >= 1000){//读频率，周期
		Timer_1000ms = 0;
		TR0 = 0;
		freq = (TH0 << 8) | TL0;
		T = (u32)((float)(1.0/freq) * 1000000);
		TH0 = 0;
		TL0 = 0;
		TR0 = 1;
	}
}