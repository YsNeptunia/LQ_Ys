#include "System.h"

//��ʱ����
u8 cnt;//��������
u8 mcnt;//����ʱ����
u8 keycnt;//��������
u8 fcnt;//��˸��ʱ
u8 icnt;//��Ϣ��ȡ��ʱ
bit flash;//��˸��־
u8 ucnt;//���ڼ�ʱ����ʱ����
bit uflag;//���ڱ�־λ��������Ϊ1

u8 mode = 0;//�������ʾģʽ��0��ʼ���棬1ʱ�������棬2ʱ�����ã�3�������ã�4��Ϣ��ʾ
bit uartmode;//���ڿ���ģʽ��Ϊ1ʱ����ʧЧ

u8 password[8] = {1,2,3,4,5,6,7,8};//ϵͳ�Ѵ��������
u8 pw_input[8] = {0xbf,0xbf,0xbf,0xbf,0xbf,0xbf,0xbf,0xbf};//�û���������룬���ڶ��գ�������Ĭ��Ϊ0xbf
u8 pw_index = 8;//���������±ָ꣬��8ʱ��������Ϊ��

u8 time[3] = {0x23,0x59,0x55};//��ʱʱ��
u8 alarm[3] = {0x00,0x00,0x00};//����
struct T{//���õ�ʱ��
	u8 d[3];//��ʱ�洢����
	u8 i;//ѡ���±�
};
struct T temp;
bit editflag;//���ñ༭��־���ú����
bit tempb;//ѡ���ߵ�λ
bit editmode;//ѡ��༭ʱ������ӣ�0ʱ�䣬1����
//bit tflag = 1;//��ʱ���־���ڷ��ʹ���ʱΪ0

bit light;//�����߱�־λ
float temper;//�¶�
u8 dist;//����
u8 str[20];//�����ַ�������
u8 recv[10];//�����ַ�������
u8 rindex;//�����ַ��������±�


void mPW_Shift(bit lr)//������������
{
	u8 i;
	if(lr == 0){//����
		for(i = pw_index;i < 8;i++){
			pw_input[i-1] = pw_input[i];
		}
		pw_input[7] = 0xbf;
	}
	else{//����
		if(pw_index == 0){//��������
			for(i = 7;i > 0;i--){
				pw_input[i] = pw_input[i-1];
			}
			pw_input[0] = 0xbf;
		}
		else{//����δ��
			for(i = 6;i >= pw_index;i--){
				pw_input[i+1] = pw_input[i];
			}
			pw_input[i+1] = 0xbf;
		}
	}
}

void mPW_Ins(u8 b)//����һλ����
{
	if(pw_input[0] == 0xbf){
		mPW_Shift(0);
		pw_index--;
		pw_input[7] = b;
	}
}

void mPW_Del(void)//ɾ��һλ����
{
	if(pw_input[7] != 0xbf){
		mPW_Shift(1);
		pw_index++;
	}
}

void mPW_Clear(void){//������������
	u8 i;
	for(i = 0;i < 8;i++){
		pw_input[i] = 0xbf;
	}
	pw_index = 8;
}

bit mPW_IsCorrect(void)//�����жϺ���
{
	u8 i;
	for(i = 0;i < 8;i++){
		if(pw_input[i] != password[i])
			return 0;
	}
	return 1;
}

void mTime_Ins(u8 dat)//�������õ�ʱ�䣨��λ���룩
{
	if(tempb)
		temp.d[temp.i] = temp.d[temp.i] & 0x0f | (DECtoBCD(dat) << 4);
	else
		temp.d[temp.i] = temp.d[temp.i] & 0xf0 | DECtoBCD(dat);
	tempb = ~tempb;
}

//void mTime_IsCorrect(u8 dat)//�ж������ʱ���Ƿ�Ϸ�
//{
//	
//}

void mInfo_Read()//��ȡ��Ϣ����
{
	temper = Read_Temper();
	dist = Ultra_Read() % 100;

	if(AD_Read(0x41) > 128)	light = 1;
	else	light = 0;//�й����
	if(light)	DA_Write(51);//�й������1v
	else	DA_Write(255);
}

void KBD_Process()//����������
{
	if(cnt >= 10){
		KBD_Read();
		cnt = 0;
		
		//mode0������
		if(mode == 0){
			//��������
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
			
			if(trg == 0x81){//ȷ������
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
			if(trg == 0x82)	mPW_Del();//ɾ������
			
			if(cont == 0x84){//�����������
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
			if(cont == 0x18){//S16�������ص�¼����
				keycnt++;
				if(keycnt >= 100){
					mode = 0;
				}
			}
			if(keycnt && cont == 0x00){//������ʱ����
				keycnt = 0;
			}
			
			if(trg == 0x82){//ʱ������
				mode = 2;
				editflag = 1;
				editmode = 0;
				return;
			}
			if(trg == 0x84){//��������
				mode = 3;
				editflag = 1;
				editmode = 1;
				return;
			}
			
			if(trg == 0x81){//��Ϣ��ʾ
				mode = 4;
				ES = 0;
				return;
			}
			
			if(trg == 0x41){//���ڿ���
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
				temp.i = 0;//��һ�ν����Сʱ�༭
				tempb = 1;//��λ
				editflag = 0;
			}
			
			switch(trg){//������
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
				
				case 0x41:temp.i = 0;break;//Сʱ����
				case 0x21:temp.i = 1;break;//��������
				case 0x11:temp.i = 2;break;//��������
				
				case 0x81:{//ȷ�ϱ���
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
				case 0x82:mode = 1;break;//ȡ������
				case 0x84:uartmode = 1;break;//����ģʽ
			}
		}
		
		if(mode == 4){//��Ϣ��ʾģʽ
			if(cont != 0x81){
				mode = 1;
				ES = 1;
				return;
			}
		}
		
	}
}

void SEG_Porcess()//����ܴ�����
{
	switch(mode){
		case 0:{//ģʽ0����������
			u8 i;
			for(i = 0;i < 8;i++){
				if(pw_input[i] != 0xbf)
					seg_buf[i] = tab[pw_input[i]];
				else
					seg_buf[i] = 0xbf;
			}
		}break;
		
		case 1:{//ģʽ1����ʾʱ��		
			seg_buf[0] = tab[time[0] / 16];
			seg_buf[1] = tab[time[0] % 16];
			seg_buf[3] = tab[time[1] / 16];
			seg_buf[4] = tab[time[1] % 16];
			seg_buf[6] = tab[time[2] / 16];
			seg_buf[7] = tab[time[2] % 16];
			
			if(flash){//�ָ�����˸
				seg_buf[2] = 0xbf;
				seg_buf[5] = 0xbf;
			}
			else{
				seg_buf[2] = 0xff;
				seg_buf[5] = 0xff;
			}
		}break;
		
		case 2://ģʽ2��ʱ������	
		case 3:{//ģʽ3����������
			seg_buf[0] = tab[temp.d[0] / 16];
			seg_buf[1] = tab[temp.d[0] % 16];
			seg_buf[3] = tab[temp.d[1] / 16];
			seg_buf[4] = tab[temp.d[1] % 16];
			seg_buf[6] = tab[temp.d[2] / 16];
			seg_buf[7] = tab[temp.d[2] % 16];
			
			if(flash){//�ָ�����˸
				seg_buf[2] = 0xbf;
				seg_buf[5] = 0xbf;
			}
			else{
				seg_buf[2] = 0xff;
				seg_buf[5] = 0xff;
			}
		}break;
		
		case 4:{
			seg_buf[0] = tab[(dist/10 %10)];//������������
			seg_buf[1] = tab[(dist %10)];
			seg_buf[2] = tab[light];//�Ƿ��й�
			seg_buf[3] = 0xbf;//�ָ���"-"
			seg_buf[4] = tab[((u8)(temper /10) %10)];//�������¶�
			seg_buf[5] = tab[((u8)temper %10)] & 0x7f;//��С����
			seg_buf[6] = tab[((u8)(temper *10) %10)];
			seg_buf[7] = tab[12];//"C"
		}break;
	}
}

void LED_Process()//LED������
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

void Uart_Process()//�������ݴ�����
{
	if(ucnt >= 20){
		ucnt = uflag = 0;
		if(uartmode){
			if(rindex == 6){
				if((mode == 2 || mode == 3) && recv[1] == 'S' && recv[2] == 'e' && recv[3] == 't'){//��������ʱ��
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
					Uart_Send("ʱ�������ã�");
				}
				rindex = 0;
			}
			else if(rindex == 4){
				if(recv[0] == 'C' && recv[1] == 'K' && recv[2] == 'G' && recv[3] == 'B'){
					uartmode = 0;
					Uart_Send("����ģʽ�رգ�");
					rindex = 0;
				}
			}
			else if(recv[0] == 'X' && recv[1] == 'G'){
				u8 i;
				for(i = 0;i < 8;i++){
					password[i] = recv[i + 2];
				}
				Uart_Send("�����޸ĳɹ���");
			}
			memset(recv,0,rindex);//������������
			rindex = 0;
		}
	}
}

void mInfo_Process()//��Ϣ�����ͺ���
{
	static bit flag1,flag2;//��־λ
	
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
			sprintf(str,"%02d:%02d-%02dCM-%03.1f��\r\n",hour,min,dist,temper);//-%02dCM-%3.1f��	,dist,temper
			Uart_Send(str);
//			sprintf(str,"-%02dCM\r\n",dist);//-%03.1f��  ,temper
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
	Read_Temper();//�ϵ����ֹ���ݴ���
	Ultra_Read();
	Delay750ms();
	Timer0_Init();
	Uart1Init();
	Device_Init();
	DS1302_Set(time);
	
	EEPROM_Write(alarm,8,3);
	EEPROM_Read(&alarm,8,3);
	Uart_Send("�����ʼ���ɹ���");
	while(1)
	{
		if(~uartmode)
			KBD_Process();
		LED_Process();
		SEG_Porcess();
		Uart_Process();
		
		mInfo_Process();
		
		if(icnt >= 50){//ÿ0.5s����һ�η�����
			icnt = 0;
			mInfo_Read();
		}
	}
}

void T0ISR(void) interrupt 1
{//��ʱ��0�жϴ�����
	cnt++;
	mcnt++;
	SEG_Display();
	
	if(mcnt >= 10){
		DS1302_Read(time);
		fcnt++;
		icnt++;
		if(uflag)//������ʱ�����ۼ�
			ucnt++;
		mcnt = 0;
	}
	if((mode == 1 && fcnt >= 50) || ((mode == 2 || mode == 3) && fcnt >= 70)){
		flash = ~flash;
		fcnt = 0;
	}
}

void U1ISR(void) interrupt 4
{//����1�жϴ�����
	if(RI == 1){//�����ж�
		recv[rindex] = SBUF;
		rindex++;
		uflag = 1;//�����ݣ����߱�־λ
		RI = 0;
	}
}
