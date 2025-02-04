#include "System.h"

//计时变量
u8 keycnt;
u8 icnt;

//题设变量
u8 rtc[3] = {0x23,0x59,0x55};
float temper;
float AIN1,AIN3;
u8 x = 30;
u8 sx;
float dist;

void mInfo_Process()
{
	if(icnt)	return;
	icnt = 1;

//	DS1302_Read(rtc);
	
//	temper = Read_Temper();
	
	DA_Write(255);
	AIN3 = AD_Read(0x41) * 1.0 / 51;
	AIN1 = AD_Read(0x43) * 1.0 / 51;
	
	dist = Ultra_Read();
}


void KBD_Process()
{
	if(keycnt)	return;
	keycnt = 1;

	KBD_Read();
	
	if(pres == 0x88)	x--;
	if(pres == 0x84)	x++;
	if(pres == 0x44){
		EEPROM_Write(&x,0,8);
		EEPROM_Read(&sx,0,8);
	}
}

void SEG_Process()
{
	u8 i;
	
	for(i = 0;i < 4; i++){
		seg_buf[i] = 0xff;
	}
	seg_buf[5] = tab[(u8)dist/100%10];
	seg_buf[6] = tab[(u8)dist/10%10];
	seg_buf[7] = tab[(u8)dist%10];
	
//	u8 i,j,buf[3];

//	for(i = 4;i < 8; i++){
//		seg_buf[i] = 0xff;
//	}
//	buf[0] = tab[(u8)AIN3%10] & 0x7f;
//	buf[1] = tab[(u16)(AIN3*10)%10];
//	buf[2] = tab[(u16)(AIN3*100)%10];
//	buf[3] = tab[(u16)(AIN3*1000)%10];
//	
//	i = 3;
//	while(buf[i] == tab[0]){
//		for(j = 3; buf[j] != 0xff && j > 0; j--){
//			buf[j] = buf[j - 1];
//		}
//		buf[0] = 0xff;
//		if(i != 0)	i--;
//	}
//	for(i = 0;i < 4; i++){
//		seg_buf[i] = buf[i];
//	}
	
//	seg_buf[0] = tab[(u8)(temper /10)%10];
//	seg_buf[1] = tab[(u8)temper %10] & 0x7f;
//	seg_buf[2] = tab[(u8)(temper *10) %10];
//	seg_buf[3] = tab[(u8)(temper *100) %10];
//	seg_buf[4] = tab[(u8)(temper *1000) %10];
//	seg_buf[5] = tab[(u8)(temper *10000) %10];
//	seg_buf[6] = 0xff;
//	seg_buf[7] = 0xff;
	
//	seg_buf[0] = tab[rtc[0] / 16];
//	seg_buf[1] = tab[rtc[0] % 16];
//	seg_buf[2] = 0xbf;
//	seg_buf[3] = tab[rtc[1] / 16];
//	seg_buf[4] = tab[rtc[1] % 16];
//	seg_buf[5] = 0xbf;
//	seg_buf[6] = tab[rtc[2] / 16];
//	seg_buf[7] = tab[rtc[2] % 16];
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
	Timer2_Init();
	Device_Init();
	Read_Temper();
	Delay750ms();
	
	DS1302_Set(rtc);
	DS1302_Read(rtc);
	
	EEPROM_Read(&sx,0,8);
	
	while(1)
	{
		mInfo_Process();
		
		KBD_Process();
		SEG_Process();
	}
}

void T2ISR() interrupt 12
{
	SEG_Display();
	
	if(++keycnt >= 10)	keycnt = 0;
	if(++icnt >= 200)		icnt = 0;
}
