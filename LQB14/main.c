#include "System.h"

//计时变量
u8 kbd_cnt,seg_cnt,led_cnt,time_cnt,mcnt,mcd1,mcd2,fcnt;//减速变量
bit icnt = 0;//1s触发处理数据
bit cflag = 0;//触发采集标志位，超时后清除
u8 Timer_1000Ms = 0;//1s计时变量
bit flash = 0;//0.1s报警闪烁
bit newtemp;//更新温度标志位

bit up;//相较上升标志
u8 led = 0xff;
//题设变量
u8 mode = 1;//模式1时间模式，模式234回显模式，模式5参数模式，模式6温湿度界面（采集功能触发后，数码管立刻切换到温湿度界面）
u8 modetemp;//触发前模式
bit light = 0;//光照变化标志位，从“亮”状态切换到“暗”状态时变1
u8 time[3] = {0x13,0x03,0x05};//实时时间
u8 ptemp = 30;//温度参数
struct E{
	u8 tem;//温度
	u8 wet;//湿度
};
struct E edata[99];//采集到的数据
u8 eindex = 0;//触发次数，数组下标
u16 freq = 0;//检测到T0频率
float tem_aver = 0.0;//温度平均数
float wet_aver = 0.0;//湿度平均数
u8 tem_max = 0;//温度最大值
u8 wet_max = 0;//湿度最大值
u8 etime[2];//最近一次采集到的时间（小时分钟）


void mTime_Process()//读取时间
{
	if(time_cnt)	return;
	time_cnt = 1;
	
	DS1302_Read(time);
}

u8 mWet_Process()//频率转换为湿度
{
	float w;//湿度（十进制）
	if((freq >= 200) && (freq <= 2000)){
		w = (80 / 1800.0) * (freq - 200) + 10.0;
		return (u8)w;
	}
	else
		return 0xaa;//数据无效
}

void mCollect_Process()//采集处理函数
{
	if(mcnt)	return;
	mcnt = 1;
	
	if(~light)
		if(AD_Read(0x41) >= 128){
			light = 1;
			return;
		}
	
	if(light && AD_Read(0x41) < 128){
		light = 0;
		cflag = 1;
		modetemp = mode;
		mode = 6;
		
		edata[eindex].tem = (u8)Read_Temper();
		edata[eindex].wet = mWet_Process();
		eindex++;
		etime[0] = time[0];
		etime[1] = time[1];

	}
}

void mInfo_Process()//数据处理函数，处理平均值等
{
	static bit iflag;//判断是否更新数据
	u8 i;
	u8 max1 = 0;//循环数，打擂最大值
	u8 max2 = 0;
	u8 ninfo = 0;//无效数据
	u32 sum1 = 0;//累加值，用于计算平均数
	u32 sum2 = 0;
	
	if(cflag && ~iflag){
		iflag = 1;
		
		for(i = 0;i < eindex;i++){
			if(edata[i].wet == 0xaa){
				ninfo++;
				continue;
			}
			sum1 += edata[i].tem;
			if(edata[i].tem > max1)
				max1 = edata[i].tem;
			sum2 += edata[i].wet;
			if(edata[i].wet > max2)
				max2 = edata[i].wet;
		}
		if((eindex - ninfo) != 0){
			tem_aver = sum1 * 1.0 / (eindex - ninfo) ;
			tem_max = max1;
			wet_aver = sum2 * 1.0 / (eindex - ninfo) ;
			wet_max = max2;
		}
		else{
			tem_aver = 0.0;
			tem_max = 0;
			wet_aver = 0.0;
			wet_max = 0;
		}
	}
	else if(~cflag && iflag)	iflag = 0;
	else{return;}
}


void KBD_Process()
{
	static u8 pcnt = 0;//长按计时
	static bit flag;//长按标志位
	if(kbd_cnt)	return;
	kbd_cnt = 1;
	
	KBD_Read();
	
	if(trg == 0x88){//切换时间、回显、参数
		switch(mode){
			case 1:mode = 2;break;
			case 2:
			case 3:
			case 4:mode = 5;break;
			case 5:mode = 1;break;
		}
		return;
	}
	
	if((mode == 2|| mode == 3|| mode == 4) && trg == 0x84){
		if(mode == 4)	mode = 2;
		else	mode++;
		return;
	}
	
	if(mode == 5 && trg == 0x44)	ptemp++;
	if(mode == 5 && trg == 0x48)	ptemp--;
	
	if(mode == 1 && cont == 0x48){
		pcnt++;
		if(pcnt >= 200){
			pcnt = 0;
			flag = 1;
		}
	}
	else	pcnt = 0;
	if(flag){
		flag = 0;
		eindex = 0;
	}
}

void SEG_Process()
{
	if(seg_cnt)	return;
	seg_cnt = 1;
	
	switch(mode){
		case 1:{
			seg_buf[0] = tab[time[0] / 16];
			seg_buf[1] = tab[time[0] % 16];
			seg_buf[2] = 0xbf;//"-"
			seg_buf[3] = tab[time[1] / 16];
			seg_buf[4] = tab[time[1] % 16];
			seg_buf[5] = 0xbf;//"-"
			seg_buf[6] = tab[time[2] / 16];
			seg_buf[7] = tab[time[2] % 16];
		}break;
		
		case 2:{
			u8 i;
			seg_buf[0] = tab[12];//"C"
			if(eindex == 0){
				for(i = 1; i < 8; i++){
					seg_buf[i] = 0xff;
				}
			}
			else{
				seg_buf[1] = 0xff;
				seg_buf[2] = tab[tem_max /10 %10];
				seg_buf[3] = tab[tem_max %10];
				seg_buf[4] = 0xbf;//"-"
				seg_buf[5] = tab[(u8)tem_aver /10 %10];
				seg_buf[6] = tab[(u8)tem_aver %10] & 0x7f;//"."
				seg_buf[7] = tab[(u8)(tem_aver *10) %10];
			}
		}break;
		
		case 3:{
			u8 i;
			seg_buf[0] = 0x89;//"H"
			if(eindex == 0){
				for(i = 1; i < 8; i++){
					seg_buf[i] = 0xff;
				}
			}
			else{
				seg_buf[1] = 0xff;
				seg_buf[2] = tab[wet_max /10 %10];
				seg_buf[3] = tab[wet_max %10];
				seg_buf[4] = 0xbf;//"-"
				seg_buf[5] = tab[(u8)wet_aver /10 %10];
				seg_buf[6] = tab[(u8)wet_aver %10] & 0x7f;//"."
				seg_buf[7] = tab[(u8)(wet_aver *10) %10];
			}
		}break;
		
		case 4:{
			u8 i;
			seg_buf[0] = tab[15];//"F"
			seg_buf[1] = tab[eindex/10%10];
			seg_buf[2] = tab[eindex%10];
			if(eindex == 0){
				for(i = 3;i<8;i++){
					seg_buf[i] = 0xff;
				}
			}
			else{
				seg_buf[3] = tab[etime[0] / 16];
				seg_buf[4] = tab[etime[0] % 16];
				seg_buf[5] = 0xbf;//"-"
				seg_buf[6] = tab[etime[1] / 16];
				seg_buf[7] = tab[etime[1] % 16];
			}
		}break;
		
		case 5:{
			seg_buf[0] = 0x8c;//"P"
			seg_buf[1] = 0xff;
			seg_buf[2] = 0xff;
			seg_buf[3] = 0xff;
			seg_buf[4] = 0xff;
			seg_buf[5] = 0xff;
			seg_buf[6] = tab[ptemp /10 %10];
			seg_buf[7] = tab[ptemp %10];
		}break;
		
		case 6:{
			seg_buf[0] = tab[14];//"E"
			seg_buf[1] = 0xff;
			seg_buf[2] = 0xff;
			seg_buf[3] = tab[(edata[eindex-1].tem) /10 %10];
			seg_buf[4] = tab[(edata[eindex-1].tem) %10];
			seg_buf[5] = 0xbf;//"-"
			if(edata[eindex-1].wet != 0xaa){
				seg_buf[6] = tab[(edata[eindex-1].wet) /10 %10];
				seg_buf[7] = tab[(edata[eindex-1].wet) %10];
			}
			else{
				seg_buf[6] = tab[(edata[eindex-1].wet) /16];//"A"
				seg_buf[7] = tab[(edata[eindex-1].wet) %16];
			}
		}break;
		
	}
}

void LED_Process()
{
	if(led_cnt)	return;
	led_cnt = 1;
	
	
	switch(mode){
		case 1:led = 0xfe;break;
		case 2:
		case 3:
		case 4:led = 0xfd;break;
		case 5:led = 0xfb;break;
		case 6:led = 0xff;break;
	}
	
	if(flash)
		led &= 0xf7;//L4闪烁
	else
		led &= 0xff;
	
	if(edata[eindex].wet == 0xaa)
		led &= 0xef;//L5点亮	
	
	if(up)
		led &= 0xdf;//L6点亮
	
	Device_Ctrl(0x80,led);
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
	mWet_Process();
	Delay750ms();//防止读到脏东西
	
	Timer0_Init();
	Timer1_Init();
	Timer2_Init();
	Device_Init();
	DS1302_Set(time);
	
	while(1)
	{	
		KBD_Process();
		SEG_Process();
		LED_Process();
		
		mTime_Process();
		if(~cflag)
			mCollect_Process();
		if(icnt){
			mInfo_Process();
			icnt = 0;
		}
		
		if(newtemp){
			newtemp = 0;
			Read_Temper();//更新温度
		}
	}
}

void T1ISR() interrupt 3
{
	SEG_Display();
	
	if(++kbd_cnt >= 10)		kbd_cnt = 0;
	if(++seg_cnt >= 200)	seg_cnt = 0;
	if(++time_cnt >= 25)	time_cnt = 0;
	if(++led_cnt >= 10)	led_cnt = 0;
	
	if(++mcnt >= 10)	mcnt = 0;
	if(cflag){
		mcd1++;
		if(mcd1 >= 100){
			mcd1 = 0;
			mcd2++;
			if(mcd2 >= 30){//3s内不触发
				mcd2 = 0;
				if(edata[eindex-1].wet == 0xaa)	--eindex;
				cflag = 0;
				mode = modetemp;
			}
		}
	}
}

void T2ISR() interrupt 12
{
	if((edata[eindex-1].tem > ptemp) && eindex){
		fcnt++;
		if(fcnt >= 10){
			fcnt = 0;
			flash = ~flash;
		}
	}
	else if(edata[eindex-1].tem <= ptemp) flash = 0;
	
	if(++Timer_1000Ms >= 100){//读取频率
		Timer_1000Ms = 0;
		freq = (TH0 << 8) | TL0;
		TL0 = 0;
		TH0 = 0;
		icnt = 1;
		newtemp = 1;
		if(eindex >= 2){
			if((edata[eindex - 1].tem > edata[eindex - 2].tem) && (edata[eindex - 1].wet > edata[eindex - 2].wet))
				up = 1;
			else
				up = 0;
		}
	}
}

