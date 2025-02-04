#include "System.h"

u8 cnt;//按键消抖变量
u8 ecnt;//控制闪光变量
u8 mode = 1;//主模式选择,123为普通显示，456为设置显示

u8 time[3] = {0x23,0x59,0x55};//当前时钟，BCD码
u8 date[3] = {0x22,0x12,0x12};//当前日期
u8 alarm[3][3] = 
{
	{0x00,0x00,0x00},
	{0x00,0x01,0x00},
	{0xbf,0xbf,0xbf}
};//闹钟
u8 al_num = 0;//闹钟选择编号，第一个下标
u8 al_ring = 3;//闹钟响铃编号，响铃时变化，3为空
bit al_flag = 1;//闹钟使能标志
bit ringing;//响铃状态标志
bit al_flash;//响铃闪烁标志
u8 al_count;//响铃计数，超时自动关闭响铃
u8 edit_index = 3;//012:待修改下标 3：非修改状态
bit edit_flash = 0;//修改使能标志，使数码管0.5s闪
bit eflag;//设置长按标志
u8 temp[3];//存储时间临时变量

//struct uart{//串口数据结构体
//	u8 dat[25];//字符串数据
//	u8 i;//数组下标指针
//};
//struct uart recv;//接收字符串
//struct uart send;//发送字符串

bit mAlarm_Exist(void)//判断闹钟是否存在函数
{
	if(alarm[al_num][0] == 0xbf)	return 0;
	else	return 1;
}

void mAlarm_Clear(void)//清除当前闹钟函数
{
	u8 i;
	for(i = 0; i < 3; i++){
		alarm[al_num][i] = 0xbf;
	}
}

void mTime_IncDec(u8* t, u8 i,bit ope)//时间增减函数
{//形参为待操作的时间数组、操作下标和待操作数正负
	u8 x,max;//即将加减的数,对应上限
	x = BCDtoDEC(t[i]);
	
	//上限判断，小时的上限为23，时分的上限为59，下限均为00
	if(i == 0)	max = 0x23;
	else max = 0x59;
	
	if(~ope){
		if(t[i] == max)	t[i] = 0x00;
		else{
			x++;
			t[i] = DECtoBCD(x);
		}
	}
	else{
		if(t[i] == 0x00)	t[i] = max;
		else{
			x--;
			t[i] = DECtoBCD(x);
		}
	}
}

void mDate_IncDec(u8* d, u8 i, bit ope)//日期增减函数
{
	u8 x,max,min;//待加减的时间参数，对应上限，下限
	u16 year;
	x = BCDtoDEC(d[i]);
	year = BCDtoDEC(d[0]) + 2000;//年份，便于判断是否为闰年
	
	switch(i){//日期上下限判断
		case 0: max = 0x99;min = 0x00;break;
		case 1:	max = 0x12;min = 0x01;break;
		case 2:{
			if(d[1] == 0x02){
				if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))//闰年条件：能被4整除但是不能被100整除 或者 能被400整除
					max = 0x29;
				else	max = 0x28;
			}
			else if(d[1] == 4 || d[1] == 6 || d[1] == 9 || d[1] == 11)//小月
				max = 0x30;
			else
				max = 0x31;
			min = 0x01;
		}break;
	}
	
	
	if(~ope){//日期加减
		if(d[i] == max)	d[i] = min;
		else{
			x++;
			d[i] = DECtoBCD(x);
		}
	}
	else{
		if(d[i] == min)	d[i] = max;
		else{
			x--;
			d[i] = DECtoBCD(x);
		}
	}
}


void KBD_Process()//矩阵键盘处理函数
{
	static bit mflag;//模式选择标志
	static u8 pcnt,ecnt;//长按计时,设置长按计时
	
	if(cnt >= 10){//按键消抖处理，操作函数写在大括号内
		KBD_Read();
		cnt = 0;
		
		if(trg == 0x18 && edit_index == 3)//S16抬手长按判断
			mflag = 1;
		if(mflag && cont == 0x00){//S16主模式选择，抬手切换
			mflag = 0;
			pcnt = 0;
			mode++;
			if(mode >= 4)	mode = 1;
			if(mode == 3)	al_num = 0;
		}
		if(mflag && cont == 0x18){//S16闹钟使能，长按切换
			pcnt++;
			if(pcnt >= 100){//长按1s进入
				mflag = 0;
				pcnt = 0;
				al_flag = ~al_flag;
			}
		}
		
		if(trg == 0x14 && mode == 3){//S17闹钟编号切换
			al_num++;
			al_num %= 3;
		}
		
		if(cont == 0x24 && mode == 3 && edit_index == 3){//S13闹钟清除按键
			pcnt++;
			if(pcnt >= 100){
				pcnt = 0;
				mAlarm_Clear();
			}
		}
		
		if(trg == 0x28){//S12进入修改菜单
			u8 i;
			bit tempflag;
			if(edit_index >= 2){
				if(mode <= 3){
					mode += 3;//将模式切换入对应设置模式
					tempflag = al_flag;//临时储存闹钟使能模式
					al_flag = 0;//防止进入闹钟响铃
					if(mode == 4){//时间设置
						for(i = 0; i<3; i++)
							temp[i] = time[i];
					}
					else if(mode == 5){//日期设置
						for(i = 0; i<3; i++)
							temp[i] = date[i];
					}
					else{
						if(mAlarm_Exist()){
							for(i = 0; i<3; i++)
								temp[i] = alarm[al_num][i];//已有闹钟
						}
						else{
							for(i = 0; i<3; i++)
								temp[i] = 0x00;//新闹钟，从0修改
						}
					}
					edit_index = 0;
				}
				else{
					mode -= 3;
					if(mode == 1)
						DS1302_Set(temp,date);
					else if(mode == 2)
						DS1302_Set(time,temp);
					else{
						for(i = 0; i<3; i++)
							alarm[al_num][i] = temp[i];
					}
					al_flag = tempflag;//闹钟使能模式恢复为设置前的
					edit_index = 3;
				}
			}
			else
				edit_index++;
		}

		if((cont == 0x81 || cont == 0x88) && edit_index != 3){//长按快速加减，在下面的函数每0.1s触发一次加减
			pcnt++;
			ecnt++;
			if(pcnt >= 150){//1.5s进入
				eflag = 1;
				pcnt = 0;
			}
		}
		if(((trg == 0x81 || trg == 0x88) && edit_index != 3) || (eflag && ecnt >= 10)){//S6设置加,S5设置减
			bit flag;//增减判断
			if(cont == 0x88)	flag = 1;
			else	flag = 0;
			if(mode == 4)
				mTime_IncDec(temp,edit_index,flag);
			else if(mode == 5)
				mDate_IncDec(temp,edit_index,flag);
			else{
				mTime_IncDec(temp,edit_index,flag);	
			}
			ecnt = 0;
		}
		
		if(ringing == 1 && cont != 0x00){//任意键关闭闹钟
			ringing = 0;
			al_count = 0;
			al_ring = 3;
			Device_Ctrl(0xa0,0x00);
		}
		
		if(cont == 0x00){//松手长按计数清零
			pcnt = 0;
			ecnt = 0;
			eflag = 0;
		}
		
	}
}

void SEG_Process()//数码管处理函数
{
	seg_buf[2] = 0xbf;//"-"
	seg_buf[5] = 0xbf;
	
	switch(mode){
		case 1:{//显示当前时间
		seg_buf[0] = tab[time[0] / 16];
		seg_buf[1] = tab[time[0] % 16];
		seg_buf[3] = tab[time[1] / 16];
		seg_buf[4] = tab[time[1] % 16];
		seg_buf[6] = tab[time[2] / 16];
		seg_buf[7] = tab[time[2] % 16];
		}break;
		
		case 2:{//显示当前日期
		seg_buf[0] = tab[date[0] / 16];
		seg_buf[1] = tab[date[0] % 16];
		seg_buf[3] = tab[date[1] / 16];
		seg_buf[4] = tab[date[1] % 16];
		seg_buf[6] = tab[date[2] / 16];
		seg_buf[7] = tab[date[2] % 16];		
		}break;
		
		case 3:{//显示闹钟
		if(mAlarm_Exist()){
			seg_buf[0] = tab[alarm[al_num][0] / 16];
			seg_buf[1] = tab[alarm[al_num][0] % 16];
			seg_buf[3] = tab[alarm[al_num][1] / 16];
			seg_buf[4] = tab[alarm[al_num][1] % 16];
			seg_buf[6] = tab[alarm[al_num][2] / 16];
			seg_buf[7] = tab[alarm[al_num][2] % 16];
		}
		else{
			seg_buf[0] = alarm[al_num][0];
			seg_buf[1] = alarm[al_num][0];
			seg_buf[3] = alarm[al_num][0];
			seg_buf[4] = alarm[al_num][0];
			seg_buf[6] = alarm[al_num][0];
			seg_buf[7] = alarm[al_num][0];					
		}
		}break;
		
		default:{//编辑模式
			u8 lbit = edit_index * 3;
			u8 lbit1 = (lbit+3)%9;
			u8 lbit2 = (lbit+6)%9;
			u8 edit_index1 = (edit_index+1)%3;
			u8 edit_index2 = (edit_index+2)%3;
			if(edit_flash || eflag){//待编辑位闪烁(为长按状态下不闪烁)
				seg_buf[lbit] = tab[temp[edit_index] / 16];
				seg_buf[lbit + 1] = tab[temp[edit_index] % 16];
			}
			else{//熄灭
				seg_buf[lbit] = 0xff;
				seg_buf[lbit + 1] = 0xff;
			}
			seg_buf[lbit1] = tab[temp[edit_index1] / 16];//其他位正常显示
			seg_buf[lbit1+1] = tab[temp[edit_index1] % 16];
			seg_buf[lbit2] = tab[temp[edit_index2] / 16];
			seg_buf[lbit2+1] = tab[temp[edit_index2] % 16];
		}break;
	}
}

void LED_Process()//LED处理函数
{
	u8 led;
	switch(mode){//模式切换L1~L3
		case 1:led = 0xfe;break;
		case 2:led = 0xfd;break;
		case 3:led = 0xfb;break;
		case 4:led = 0xfe;break;
		case 5:led = 0xfd;break;
		case 6:led = 0xfb;break;
	}
	if(mode == 3 || mode == 6){
		switch(al_num){//闹钟选择切换L4~L6
			case 0:led &= 0xf7;break;
			case 1:led &= 0xef;break;
			case 2:led &= 0xdf;break;
		}
	}
	if(al_flag)	led &= 0xbf;//闹钟使能标志L7
	if(edit_index != 3 && edit_flash)	led &= 0x7f;//设置模式L8
	
	if(ringing){
		if(mode == 3){//处于闹钟模式时
			switch(al_ring){//先熄灭对应闹钟
				case 0:led |= 0x08;break;
				case 1:led |= 0x10;break;
				case 2:led |= 0x20;break;
			}	
		}	
	}
	if(al_flash){
		switch(al_ring){//响铃对应闹钟闪烁
			case 0:led &= 0xf7;break;
			case 1:led &= 0xef;break;
			case 2:led &= 0xdf;break;
		}
	}
	
	Device_Ctrl(0x80,led);
}

void BEEP_Process()//闹钟进程函数
{
	u8 i;
	static bit flag;//标志位
	if(~ringing){
		for(i = 0; i < 3; i++){
			if(alarm[i][2] == time[2] && alarm[i][1] == time[1] && alarm[i][0] == time[0]){
					ringing = 1;
					flag = 1;
					al_ring = i;
					break;
				}
		}
	}
	
	
	if(ringing){
		if(al_flash && flag){
			Device_Ctrl(0xa0,0x40);
			flag = ~flag;
			al_count++;
		}
		else if(~al_flash && ~flag){
			Device_Ctrl(0xa0,0x00);
			flag = ~flag;
		}
	}
	if(al_count > 20){//响铃十秒后自动结束
		Device_Ctrl(0xa0,0x00);
		al_ring = 3;
		ringing = 0;
		flag = 0;
		al_count = 0;
	}
}

void main()
{
	Timer0_Init();
	Uart1Init();
	Device_Init();
	DS1302_Set(time,date);
	Uart_Send("程序初始化成功！");
	
	while(1)
	{
		KBD_Process();
		SEG_Process();
		LED_Process();
		if(al_flag && edit_index == 3)
			BEEP_Process();
	}
}

void T0ISR(void) interrupt 1
{
	static u8 rcnt,acnt;//读取时间计数，每0.1s一读;闹钟频响计数
	SEG_Display();
	cnt++;
	if(cnt>=10){
		rcnt++;
		if(rcnt >= 10){
			DS1302_Read(time,date);//读取实时时间
			rcnt = 0;
		}
		if(ringing){
			acnt++;
			if(acnt<=10)	al_flash = 1;
			else if(acnt <= 20)	al_flash = 0;
			else if(acnt <= 30)	al_flash = 1;
			else if(acnt <= 40)	al_flash = 0;
			if(acnt >= 100){
				al_flash = 0;
				acnt = 0;
			}
		}
		if(edit_index != 3){
			ecnt++;
			if(ecnt >= 30){
				edit_flash = ~edit_flash;
				ecnt = 0;
			}
		}
	}
}

void U1ISR(void) interrupt 4
{
//	if(RI == 1){//串口接收数据
//		recv.dat[recv.i++] = SBUF;
//		RI = 0;
//	}
}

