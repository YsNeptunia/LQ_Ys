#include "Ultra.h"
//Tx为1时发送成功，Rx为0时接收成功
//记住返回值乘常数0.017

void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}

void Ultra_Init()//超声波初始化函数，产生8个40MHz的方波
{
	u8 i;
	for(i = 0;i < 8;i++){
		Tx = 1;
		Delay12us();
		Tx = 0;
		Delay12us();
	}
}

u8 Ultra_Read()//读取距离
{
	u16 t;//声波时间
	
	TMOD &= 0x0f;//定时器1配置计时模式
	TH1 = TL1 = 0;//复位计数值，等待超声波信号发出 
	Ultra_Init();//初始化
	TR1 = 1;//开始计时
	while((Rx == 1) && (TF1 == 0));//等待接收到信号或定时器溢出
	TR1 = 0;//停止计时
	if(TF1 == 0){//非溢出，即接收到信号
		t = TH1 << 8 | TL1;//转换定时值
		return (t * 0.017);//返回距离值
	}
	else{
		TF1 = 0;//清除溢出标志位
		return 0;
	}
}
