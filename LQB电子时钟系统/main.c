#include "System.h"

u8 cnt;//������������
u8 ecnt;//�����������
u8 mode = 1;//��ģʽѡ��,123Ϊ��ͨ��ʾ��456Ϊ������ʾ

u8 time[3] = {0x23,0x59,0x55};//��ǰʱ�ӣ�BCD��
u8 date[3] = {0x22,0x12,0x12};//��ǰ����
u8 alarm[3][3] = 
{
	{0x00,0x00,0x00},
	{0x00,0x01,0x00},
	{0xbf,0xbf,0xbf}
};//����
u8 al_num = 0;//����ѡ���ţ���һ���±�
u8 al_ring = 3;//���������ţ�����ʱ�仯��3Ϊ��
bit al_flag = 1;//����ʹ�ܱ�־
bit ringing;//����״̬��־
bit al_flash;//������˸��־
u8 al_count;//�����������ʱ�Զ��ر�����
u8 edit_index = 3;//012:���޸��±� 3�����޸�״̬
bit edit_flash = 0;//�޸�ʹ�ܱ�־��ʹ�����0.5s��
bit eflag;//���ó�����־
u8 temp[3];//�洢ʱ����ʱ����

//struct uart{//�������ݽṹ��
//	u8 dat[25];//�ַ�������
//	u8 i;//�����±�ָ��
//};
//struct uart recv;//�����ַ���
//struct uart send;//�����ַ���

bit mAlarm_Exist(void)//�ж������Ƿ���ں���
{
	if(alarm[al_num][0] == 0xbf)	return 0;
	else	return 1;
}

void mAlarm_Clear(void)//�����ǰ���Ӻ���
{
	u8 i;
	for(i = 0; i < 3; i++){
		alarm[al_num][i] = 0xbf;
	}
}

void mTime_IncDec(u8* t, u8 i,bit ope)//ʱ����������
{//�β�Ϊ��������ʱ�����顢�����±�ʹ�����������
	u8 x,max;//�����Ӽ�����,��Ӧ����
	x = BCDtoDEC(t[i]);
	
	//�����жϣ�Сʱ������Ϊ23��ʱ�ֵ�����Ϊ59�����޾�Ϊ00
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

void mDate_IncDec(u8* d, u8 i, bit ope)//������������
{
	u8 x,max,min;//���Ӽ���ʱ���������Ӧ���ޣ�����
	u16 year;
	x = BCDtoDEC(d[i]);
	year = BCDtoDEC(d[0]) + 2000;//��ݣ������ж��Ƿ�Ϊ����
	
	switch(i){//�����������ж�
		case 0: max = 0x99;min = 0x00;break;
		case 1:	max = 0x12;min = 0x01;break;
		case 2:{
			if(d[1] == 0x02){
				if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))//�����������ܱ�4�������ǲ��ܱ�100���� ���� �ܱ�400����
					max = 0x29;
				else	max = 0x28;
			}
			else if(d[1] == 4 || d[1] == 6 || d[1] == 9 || d[1] == 11)//С��
				max = 0x30;
			else
				max = 0x31;
			min = 0x01;
		}break;
	}
	
	
	if(~ope){//���ڼӼ�
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


void KBD_Process()//������̴�����
{
	static bit mflag;//ģʽѡ���־
	static u8 pcnt,ecnt;//������ʱ,���ó�����ʱ
	
	if(cnt >= 10){//��������������������д�ڴ�������
		KBD_Read();
		cnt = 0;
		
		if(trg == 0x18 && edit_index == 3)//S16̧�ֳ����ж�
			mflag = 1;
		if(mflag && cont == 0x00){//S16��ģʽѡ��̧���л�
			mflag = 0;
			pcnt = 0;
			mode++;
			if(mode >= 4)	mode = 1;
			if(mode == 3)	al_num = 0;
		}
		if(mflag && cont == 0x18){//S16����ʹ�ܣ������л�
			pcnt++;
			if(pcnt >= 100){//����1s����
				mflag = 0;
				pcnt = 0;
				al_flag = ~al_flag;
			}
		}
		
		if(trg == 0x14 && mode == 3){//S17���ӱ���л�
			al_num++;
			al_num %= 3;
		}
		
		if(cont == 0x24 && mode == 3 && edit_index == 3){//S13�����������
			pcnt++;
			if(pcnt >= 100){
				pcnt = 0;
				mAlarm_Clear();
			}
		}
		
		if(trg == 0x28){//S12�����޸Ĳ˵�
			u8 i;
			bit tempflag;
			if(edit_index >= 2){
				if(mode <= 3){
					mode += 3;//��ģʽ�л����Ӧ����ģʽ
					tempflag = al_flag;//��ʱ��������ʹ��ģʽ
					al_flag = 0;//��ֹ������������
					if(mode == 4){//ʱ������
						for(i = 0; i<3; i++)
							temp[i] = time[i];
					}
					else if(mode == 5){//��������
						for(i = 0; i<3; i++)
							temp[i] = date[i];
					}
					else{
						if(mAlarm_Exist()){
							for(i = 0; i<3; i++)
								temp[i] = alarm[al_num][i];//��������
						}
						else{
							for(i = 0; i<3; i++)
								temp[i] = 0x00;//�����ӣ���0�޸�
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
					al_flag = tempflag;//����ʹ��ģʽ�ָ�Ϊ����ǰ��
					edit_index = 3;
				}
			}
			else
				edit_index++;
		}

		if((cont == 0x81 || cont == 0x88) && edit_index != 3){//�������ټӼ���������ĺ���ÿ0.1s����һ�μӼ�
			pcnt++;
			ecnt++;
			if(pcnt >= 150){//1.5s����
				eflag = 1;
				pcnt = 0;
			}
		}
		if(((trg == 0x81 || trg == 0x88) && edit_index != 3) || (eflag && ecnt >= 10)){//S6���ü�,S5���ü�
			bit flag;//�����ж�
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
		
		if(ringing == 1 && cont != 0x00){//������ر�����
			ringing = 0;
			al_count = 0;
			al_ring = 3;
			Device_Ctrl(0xa0,0x00);
		}
		
		if(cont == 0x00){//���ֳ�����������
			pcnt = 0;
			ecnt = 0;
			eflag = 0;
		}
		
	}
}

void SEG_Process()//����ܴ�����
{
	seg_buf[2] = 0xbf;//"-"
	seg_buf[5] = 0xbf;
	
	switch(mode){
		case 1:{//��ʾ��ǰʱ��
		seg_buf[0] = tab[time[0] / 16];
		seg_buf[1] = tab[time[0] % 16];
		seg_buf[3] = tab[time[1] / 16];
		seg_buf[4] = tab[time[1] % 16];
		seg_buf[6] = tab[time[2] / 16];
		seg_buf[7] = tab[time[2] % 16];
		}break;
		
		case 2:{//��ʾ��ǰ����
		seg_buf[0] = tab[date[0] / 16];
		seg_buf[1] = tab[date[0] % 16];
		seg_buf[3] = tab[date[1] / 16];
		seg_buf[4] = tab[date[1] % 16];
		seg_buf[6] = tab[date[2] / 16];
		seg_buf[7] = tab[date[2] % 16];		
		}break;
		
		case 3:{//��ʾ����
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
		
		default:{//�༭ģʽ
			u8 lbit = edit_index * 3;
			u8 lbit1 = (lbit+3)%9;
			u8 lbit2 = (lbit+6)%9;
			u8 edit_index1 = (edit_index+1)%3;
			u8 edit_index2 = (edit_index+2)%3;
			if(edit_flash || eflag){//���༭λ��˸(Ϊ����״̬�²���˸)
				seg_buf[lbit] = tab[temp[edit_index] / 16];
				seg_buf[lbit + 1] = tab[temp[edit_index] % 16];
			}
			else{//Ϩ��
				seg_buf[lbit] = 0xff;
				seg_buf[lbit + 1] = 0xff;
			}
			seg_buf[lbit1] = tab[temp[edit_index1] / 16];//����λ������ʾ
			seg_buf[lbit1+1] = tab[temp[edit_index1] % 16];
			seg_buf[lbit2] = tab[temp[edit_index2] / 16];
			seg_buf[lbit2+1] = tab[temp[edit_index2] % 16];
		}break;
	}
}

void LED_Process()//LED������
{
	u8 led;
	switch(mode){//ģʽ�л�L1~L3
		case 1:led = 0xfe;break;
		case 2:led = 0xfd;break;
		case 3:led = 0xfb;break;
		case 4:led = 0xfe;break;
		case 5:led = 0xfd;break;
		case 6:led = 0xfb;break;
	}
	if(mode == 3 || mode == 6){
		switch(al_num){//����ѡ���л�L4~L6
			case 0:led &= 0xf7;break;
			case 1:led &= 0xef;break;
			case 2:led &= 0xdf;break;
		}
	}
	if(al_flag)	led &= 0xbf;//����ʹ�ܱ�־L7
	if(edit_index != 3 && edit_flash)	led &= 0x7f;//����ģʽL8
	
	if(ringing){
		if(mode == 3){//��������ģʽʱ
			switch(al_ring){//��Ϩ���Ӧ����
				case 0:led |= 0x08;break;
				case 1:led |= 0x10;break;
				case 2:led |= 0x20;break;
			}	
		}	
	}
	if(al_flash){
		switch(al_ring){//�����Ӧ������˸
			case 0:led &= 0xf7;break;
			case 1:led &= 0xef;break;
			case 2:led &= 0xdf;break;
		}
	}
	
	Device_Ctrl(0x80,led);
}

void BEEP_Process()//���ӽ��̺���
{
	u8 i;
	static bit flag;//��־λ
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
	if(al_count > 20){//����ʮ����Զ�����
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
	Uart_Send("�����ʼ���ɹ���");
	
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
	static u8 rcnt,acnt;//��ȡʱ�������ÿ0.1sһ��;����Ƶ�����
	SEG_Display();
	cnt++;
	if(cnt>=10){
		rcnt++;
		if(rcnt >= 10){
			DS1302_Read(time,date);//��ȡʵʱʱ��
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
//	if(RI == 1){//���ڽ�������
//		recv.dat[recv.i++] = SBUF;
//		RI = 0;
//	}
}

