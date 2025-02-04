#include "System.h"

//计时参数
u8 cnt;//按键消抖
u8 mcnt;//主计时参数
u8 keycnt;//按键长按
u8 fcnt;//闪烁计时
u8 icnt;//信息读取计时
bit flash;//闪烁标志
u8 ucnt;//串口计时，超时进入
bit uflag;//串口标志位，有数据为1

u8 mode = 0;//数码管显示模式。0初始界面，1时间主界面，2时间设置，3闹钟设置，4信息显示
bit uartmode;//串口控制模式，为1时键盘失效

u8 password[8] = {1,2,3,4,5,6,7,8};//系统已储存的密码
u8 pw_input[8] = {0xbf,0xbf,0xbf,0xbf,0xbf,0xbf,0xbf,0xbf};//用户输入的密码，用于对照，无数据默认为0xbf
u8 pw_index = 8;//输入密码下标，指向8时输入密码为空

u8 time[3] = {0x23,0x59,0x55};//即时时间
u8 alarm[3] = {0x00,0x00,0x00};//闹钟
struct T{//设置的时间
	u8 d[3];//临时存储数据
	u8 i;//选定下标
};
struct T temp;
bit editflag;//启用编辑标志，用后清除
bit tempb;//选定高低位
bit editmode;//选择编辑时间或闹钟，0时间，1闹钟
//bit tflag = 1;//读时间标志，在发送串口时为0

bit light;//检测光线标志位
float temper;//温度
u8 dist;//距离
u8 str[20];//发送字符串缓存
u8 recv[10];//接收字符串缓存
u8 rindex;//接收字符串数组下标


void mPW_Shift(bit lr)//密码左移右移
{
	u8 i;
	if(lr == 0){//左移
		for(i = pw_index;i < 8;i++){
			pw_input[i-1] = pw_input[i];
		}
		pw_input[7] = 0xbf;
	}
	else{//右移
		if(pw_index == 0){//数组已满
			for(i = 7;i > 0;i--){
				pw_input[i] = pw_input[i-1];
			}
			pw_input[0] = 0xbf;
		}
		else{//数组未满
			for(i = 6;i >= pw_index;i--){
				pw_input[i+1] = pw_input[i];
			}
			pw_input[i+1] = 0xbf;
		}
	}
}

void mPW_Ins(u8 b)//输入一位密码
{
	if(pw_input[0] == 0xbf){
		mPW_Shift(0);
		pw_index--;
		pw_input[7] = b;
	}
}

void mPW_Del(void)//删除一位密码
{
	if(pw_input[7] != 0xbf){
		mPW_Shift(1);
		pw_index++;
	}
}

void mPW_Clear(void){//清除输入的密码
	u8 i;
	for(i = 0;i < 8;i++){
		pw_input[i] = 0xbf;
	}
	pw_index = 8;
}

bit mPW_IsCorrect(void)//密码判断函数
{
	u8 i;
	for(i = 0;i < 8;i++){
		if(pw_input[i] != password[i])
			return 0;
	}
	return 1;
}

void mTime_Ins(u8 dat)//输入设置的时间（逐位输入）
{
	if(tempb)
		temp.d[temp.i] = temp.d[temp.i] & 0x0f | (DECtoBCD(dat) << 4);
	else
		temp.d[temp.i] = temp.d[temp.i] & 0xf0 | DECtoBCD(dat);
	tempb = ~tempb;
}

//void mTime_IsCorrect(u8 dat)//判断输入的时间是否合法
//{
//	
//}

void mInfo_Read()//读取信息函数
{
	temper = Read_Temper();
	dist = Ultra_Read() % 100;

	if(AD_Read(0x41) > 128)	light = 1;
	else	light = 0;//有光与否
	if(light)	DA_Write(51);//有光照输出1v
	else	DA_Write(255);
}

void KBD_Process()//按键处理函数
{
	if(cnt >= 10){
		KBD_Read();
		cnt = 0;
		
		//mode0键盘区
		if(mode == 0){
			//输入数字
			if(trg == 0x88)		mPW_Ins(0);
			if(trg == 0x48)		mPW_Ins(1);
			if(trg == 0x28)		mPW_Ins(2);
			if(trg == 0x18)		mPW_Ins(3);
			if(trg == 0x44)		mPW_Ins(4);
			if(trg == 0x24)		mPW_Ins(5);
			if(trg == 0x14)		mPW_Ins(6);
			if(trg == 0x42)		mPW_Ins(7);
			if(trg == 0x22)		mPW_Ins(8);
			if(trg == 0x12)		mPW_Ins(9);
			
			if(trg == 0x81){//确认密码
				if(mPW_IsCorrect()){
					mode = 1;
					Device_Ctrl(0xa0,0x00);
					mPW_Clear();
					return;
				}
				else{
					mPW_Clear();
					Device_Ctrl(0xa0,0x40);
				}
			}
			if(trg == 0x82)	mPW_Del();//删除密码
			
			if(cont == 0x84){//长按清空密码
				keycnt++;
				if(keycnt >= 100){
					mPW_Clear();
				}
			}
			if(keycnt && cont == 0x00){
				keycnt = 0;
			}
		}
		
		
		if(mode == 1){
			if(cont == 0x18){//S16长按返回登录界面
				keycnt++;
				if(keycnt >= 100){
					mode = 0;
				}
			}
			if(keycnt && cont == 0x00){//长按计时清零
				keycnt = 0;
			}
			
			if(trg == 0x82){//时钟设置
				mode = 2;
				editflag = 1;
				editmode = 0;
				return;
			}
			if(trg == 0x84){//闹钟设置
				mode = 3;
				editflag = 1;
				editmode = 1;
				return;
			}
			
			if(trg == 0x81){//信息显示
				mode = 4;
				ES = 0;
				return;
			}
			
			if(trg == 0x41){//串口控制
				uartmode = 1;
				return;
			}
		}
		
		if(mode == 2 || mode == 3){
			if(editflag){
				u8 i;
				if(~editmode){
					for(i = 0;i < 3;i++){
						temp.d[i] = time[i];
					}
				}
				else{
					EEPROM_Read(&alarm,8,3);
					for(i = 0;i < 3;i++){
						temp.d[i] = alarm[i];
					}
				}
				temp.i = 0;//第一次进入从小时编辑
				tempb = 1;//高位
				editflag = 0;
			}
			
			switch(trg){//键盘区
				case 0x88:mTime_Ins(0);break;
				case 0x48:mTime_Ins(1);break;
				case 0x28:mTime_Ins(2);break;
				case 0x18:mTime_Ins(3);break;
				case 0x44:mTime_Ins(4);break;
				case 0x24:mTime_Ins(5);break;
				case 0x14:mTime_Ins(6);break;
				case 0x42:mTime_Ins(7);break;
				case 0x22:mTime_Ins(8);break;
				case 0x12:mTime_Ins(9);break;
				
				case 0x41:temp.i = 0;break;//小时设置
				case 0x21:temp.i = 1;break;//分钟设置
				case 0x11:temp.i = 2;break;//秒钟设置
				
				case 0x81:{//确认保存
					if(~editmode)
						DS1302_Set(temp.d);
					else{
						u8 i;
						for(i = 0;i < 3;i++){
							temp.d[i] = alarm[i];
						}
						EEPROM_Write(alarm,8,3);
					}
					mode = 1;
				}break;
				case 0x82:mode = 1;break;//取消保存
				case 0x84:uartmode = 1;break;//串口模式
			}
		}
		
		if(mode == 4){//信息显示模式
			if(cont != 0x81){
				mode = 1;
				ES = 1;
				return;
			}
		}
		
	}
}

void SEG_Porcess()//数码管处理函数
{
	switch(mode){
		case 0:{//模式0：输入密码
			u8 i;
			for(i = 0;i < 8;i++){
				if(pw_input[i] != 0xbf)
					seg_buf[i] = tab[pw_input[i]];
				else
					seg_buf[i] = 0xbf;
			}
		}break;
		
		case 1:{//模式1：显示时间		
			seg_buf[0] = tab[time[0] / 16];
			seg_buf[1] = tab[time[0] % 16];
			seg_buf[3] = tab[time[1] / 16];
			seg_buf[4] = tab[time[1] % 16];
			seg_buf[6] = tab[time[2] / 16];
			seg_buf[7] = tab[time[2] % 16];
			
			if(flash){//分隔符闪烁
				seg_buf[2] = 0xbf;
				seg_buf[5] = 0xbf;
			}
			else{
				seg_buf[2] = 0xff;
				seg_buf[5] = 0xff;
			}
		}break;
		
		case 2://模式2：时间设置	
		case 3:{//模式3：闹钟设置
			seg_buf[0] = tab[temp.d[0] / 16];
			seg_buf[1] = tab[temp.d[0] % 16];
			seg_buf[3] = tab[temp.d[1] / 16];
			seg_buf[4] = tab[temp.d[1] % 16];
			seg_buf[6] = tab[temp.d[2] / 16];
			seg_buf[7] = tab[temp.d[2] % 16];
			
			if(flash){//分隔符闪烁
				seg_buf[2] = 0xbf;
				seg_buf[5] = 0xbf;
			}
			else{
				seg_buf[2] = 0xff;
				seg_buf[5] = 0xff;
			}
		}break;
		
		case 4:{
			seg_buf[0] = tab[(dist/10 %10)];//读超声波距离
			seg_buf[1] = tab[(dist %10)];
			seg_buf[2] = tab[light];//是否有光
			seg_buf[3] = 0xbf;//分隔符"-"
			seg_buf[4] = tab[((u8)(temper /10) %10)];//读环境温度
			seg_buf[5] = tab[((u8)temper %10)] & 0x7f;//带小数点
			seg_buf[6] = tab[((u8)(temper *10) %10)];
			seg_buf[7] = tab[12];//"C"
		}break;
	}
}

void LED_Process()//LED处理函数
{
	u8 led;
	switch(mode){
		case 0:led = 0xfe;break;
		case 1:led = 0xfd;break;
		case 2:led = 0xfb;break;
		case 3:led = 0xf7;break;
		case 4:led = 0xef;break;
	}
	Device_Ctrl(0x80,led);
}

void Uart_Process()//串口数据处理函数
{
	if(ucnt >= 20){
		ucnt = uflag = 0;
		if(uartmode){
			if(rindex == 6){
				if((mode == 2 || mode == 3) && recv[1] == 'S' && recv[2] == 'e' && recv[3] == 't'){//串口设置时间
					switch(recv[0]){
						case 'H':temp.d[0] = ((recv[4]-'0') << 4) | (recv[5]-'0');break;
						case 'M':temp.d[1] = ((recv[4]-'0') << 4) | (recv[5]-'0');break;
						case 'S':temp.d[2] = ((recv[4]-'0') << 4) | (recv[5]-'0');break;
					}
					if(mode == 2)
						DS1302_Set(temp.d);
					else{
						u8 i;
						for(i = 0;i < 3;i++){
							temp.d[i] = alarm[i];
						}
					}
					Uart_Send("时间已设置！");
				}
				rindex = 0;
			}
			else if(rindex == 4){
				if(recv[0] == 'C' && recv[1] == 'K' && recv[2] == 'G' && recv[3] == 'B'){
					uartmode = 0;
					Uart_Send("串口模式关闭！");
					rindex = 0;
				}
			}
			else if(recv[0] == 'X' && recv[1] == 'G'){
				u8 i;
				for(i = 0;i < 8;i++){
					password[i] = recv[i + 2];
				}
				Uart_Send("密码修改成功！");
			}
			memset(recv,0,rindex);//接收数据清零
			rindex = 0;
		}
	}
}

void mInfo_Process()//信息处理发送函数
{
	static bit flag1,flag2;//标志位
	
	u8 hour, min, sec;
	hour = BCDtoDEC(time[0]);
	min = BCDtoDEC(time[1]);
	sec = BCDtoDEC(time[2]);
	
	if(~flag1 && min == 0 && sec == 0){
		sprintf(str,"Time = %02d:00:00\r\n",hour);
		Uart_Send(str);
		flag1 = 1;
	}
	if(flag1 && (min || sec))
		flag1 = 0;
	
	if(mode == 1){
		if(~flag2 && sec % 3 == 0){
			sprintf(str,"%02d:%02d-%02dCM-%03.1f℃\r\n",hour,min,dist,temper);//-%02dCM-%3.1f℃	,dist,temper
			Uart_Send(str);
//			sprintf(str,"-%02dCM\r\n",dist);//-%03.1f℃  ,temper
//			Uart_Send(str);
			flag2 = 1;
		}
		if(flag2 && sec % 3 != 0)	flag2 = 0;	
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
	Read_Temper();//上电读防止数据错乱
	Ultra_Read();
	Delay750ms();
	Timer0_Init();
	Uart1Init();
	Device_Init();
	DS1302_Set(time);
	
	EEPROM_Write(alarm,8,3);
	EEPROM_Read(&alarm,8,3);
	Uart_Send("程序初始化成功！");
	while(1)
	{
		if(~uartmode)
			KBD_Process();
		LED_Process();
		SEG_Porcess();
		Uart_Process();
		
		mInfo_Process();
		
		if(icnt >= 50){//每0.5s更新一次防堵塞
			icnt = 0;
			mInfo_Read();
		}
	}
}

void T0ISR(void) interrupt 1
{//定时器0中断处理函数
	cnt++;
	mcnt++;
	SEG_Display();
	
	if(mcnt >= 10){
		DS1302_Read(time);
		fcnt++;
		icnt++;
		if(uflag)//有数据时参数累加
			ucnt++;
		mcnt = 0;
	}
	if((mode == 1 && fcnt >= 50) || ((mode == 2 || mode == 3) && fcnt >= 70)){
		flash = ~flash;
		fcnt = 0;
	}
}

void U1ISR(void) interrupt 4
{//串口1中断处理函数
	if(RI == 1){//接收中断
		recv[rindex] = SBUF;
		rindex++;
		uflag = 1;//有数据，拉高标志位
		RI = 0;
	}
}
