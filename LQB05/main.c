#include "System.h"

//计时变量
u8 icnt;
u8 keycnt,segcnt,ledcnt,bcnt;

//题设变量
bit mainmode = 0;//数码管显示界面切换。0:时间及湿度数据显示;	1:湿度阈值设定界面
bit automode = 1;//自动手动状态切换，1时启用
u8 rtc[3] = {0x08,0x30,0x00};//实时时间
u8 wet,value = 50;//湿度，阈值湿度
u8 temp;//设置时的阈值温度
bit beepflag = 1;//蜂鸣器使能标志位
bit watering = 0;//灌溉使能标志位
bit warning = 0;//警报标志位
u8 ledstate = 0xff;//led状态保存变量
u8 ctrlstate = 0x00;//外设控制状态保存变量，不变化时不触发控制

/*——软件处理函数区——*/

void mInfo_Process()//系统数据处理函数
{
	u8 v;
	
	if(icnt)	return;
	icnt = 1;
	
	DS1302_Read(rtc);//实时时间
	v = AD_Read(0x43);//读取Rb2电压
	wet = (v / 255.0) * 99;//电压计算转化为湿度
	
	if(automode){//自动模式
		if(wet < value)	watering = 1;
		else	watering = 0;
	}
	else{//手动模式
		if(wet < value)	warning = 1;
		else	warning = 0;
	}
}


/*——硬件处理函数区——*/

void BTN_Process()
{
	if(keycnt)	return;
	keycnt = 1;
	
	BTN_Read();
	
	if(pres == 0x01)	automode = ~automode;//按键 S7 设定为系统工作状态切换按键；
	if(automode){//自动模式
		if(pres == 0x02){
			mainmode = ~mainmode;//按下 S6 后，进入湿度阈值调整界面
			if(mainmode){ 
				EEPROM_Read(&value,0,2);
				temp = value;
			}
			else{
				EEPROM_Write(&temp,0,2);
				value = temp;
			}
		}
		if(mainmode){//按下 S5 为湿度阈值加 1，按下 S4 湿度阈值减 1，
			if(pres == 0x04)	temp++;
			if(pres == 0x08)	temp--;
		}
	}
	else{//手动模式
		if(pres == 0x02)	beepflag = ~beepflag;//按下 S6 关闭蜂鸣器提醒功能，再次按下 S6 打开蜂鸣器提醒功能
		if(pres == 0x04)	watering = 1;//S5 功能设定为打开灌溉系统
		if(pres == 0x08)	watering = 0;//S4 功能设定为关闭灌溉系统
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
		seg_buf[5] = 0xff;//熄灭
		seg_buf[6] = tab[wet / 10 % 10];
		seg_buf[7] = tab[wet % 10];
	}
	else{
		seg_buf[0] = 0xbf;//"-"
		seg_buf[1] = 0xbf;//"-"
		seg_buf[2] = 0xbf;//"-"
		seg_buf[3] = 0xff;//熄灭
		seg_buf[4] = 0xff;//熄灭
		seg_buf[5] = 0xff;//熄灭
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
	u8 ctrl = 0x00;//继电器蜂鸣器控制
	
	if(bcnt)	return;
	bcnt = 1;
	
	if(watering)	ctrl |= 0x10;
	if(~automode && beepflag && warning)	ctrl |= 0x40;
	
	if(ctrl != ctrlstate){//有变化时进入
		ctrlstate = ctrl;
		Device_Ctrl(0xa0,ctrl);
	}
}

//主函数
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

//中断区
void T0ISR(void) interrupt 1
{
	if(++ledcnt >= 10)	ledcnt = 0;
	if(++keycnt >= 10)	keycnt = 0;
	if(++icnt >= 100)		icnt = 0;
	if(++segcnt >= 200)	segcnt = 0;
	if(++bcnt >= 200)		bcnt = 0;
	
	SEG_Display();
}
