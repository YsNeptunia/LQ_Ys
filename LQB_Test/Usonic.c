#include "Usonic.h"

void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 38;	//33
	while (--i);
}

void Ultra_Init()
{
	u8 i;
	
	for(i = 0;i < 8;i++){
		Tx = 1;
		Delay12us();
		Tx = 0;
		Delay12us();
	}
}

u8 Ultra_Read()
{
	u16 t;
	
	CH = CL = 0;
	CCON = 0;
	
	EA = 0;
	Ultra_Init();
	EA = 1;
	
	CR = 1;
	while(Rx && ~CF);
	CR = 0;
	if(CF == 0){
		t = CH << 8 | CL;
		return (t * 0.017);
	}
	else{
		CF = 0;
		return 0;
	}
}
