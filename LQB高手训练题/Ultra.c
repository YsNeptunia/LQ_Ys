#include "Ultra.h"
//TxΪ1ʱ���ͳɹ���RxΪ0ʱ���ճɹ�
//��ס����ֵ�˳���0.017

void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}

void Ultra_Init()//��������ʼ������������8��40MHz�ķ���
{
	u8 i;
	for(i = 0;i < 8;i++){
		Tx = 1;
		Delay12us();
		Tx = 0;
		Delay12us();
	}
}

u8 Ultra_Read()//��ȡ����
{
	u16 t;//����ʱ��
	
	TMOD &= 0x0f;//��ʱ��1���ü�ʱģʽ
	TH1 = TL1 = 0;//��λ����ֵ���ȴ��������źŷ��� 
	Ultra_Init();//��ʼ��
	TR1 = 1;//��ʼ��ʱ
	while((Rx == 1) && (TF1 == 0));//�ȴ����յ��źŻ�ʱ�����
	TR1 = 0;//ֹͣ��ʱ
	if(TF1 == 0){//������������յ��ź�
		t = TH1 << 8 | TL1;//ת����ʱֵ
		return (t * 0.017);//���ؾ���ֵ
	}
	else{
		TF1 = 0;//��������־λ
		return 0;
	}
}
