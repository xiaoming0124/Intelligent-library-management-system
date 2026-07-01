#include "DHT11.h"

void DHT11_OUT_Init(void)
{
	RCC_APB2PeriphClockCmd(DHT11_RCC,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct; 
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin=DHT11_GPIO_Pin;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(DHT11_GPIOx,&GPIO_InitStruct);
}

void DHT11_IN_Init(void)
{
	RCC_APB2PeriphClockCmd(DHT11_RCC,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct; 
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin=DHT11_GPIO_Pin;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(DHT11_GPIOx,&GPIO_InitStruct);
}

void DHT11_Start(void)
{
	DHT11_OUT_Init();
	GPIO_ResetBits(DHT11_GPIOx,DHT11_GPIO_Pin);
	delay_ms(20);
	GPIO_SetBits(DHT11_GPIOx,DHT11_GPIO_Pin);
	delay_us(20);
	DHT11_IN_Init();
}

uint8_t DHT11_ReadData(void)
{
		while(GPIO_ReadInputDataBit(DHT11_GPIOx,DHT11_GPIO_Pin)==0);
		delay_us(40);
		if(GPIO_ReadInputDataBit(DHT11_GPIOx,DHT11_GPIO_Pin)==1)
		{
			while((GPIO_ReadInputDataBit(DHT11_GPIOx,DHT11_GPIO_Pin)==1));
			return 1;
		}
		else
			return 0;


}

uint8_t DHT11_Rec_Data(void)
{
	uint8_t i,data;
	for(i=0;i<8;i++)
	{
		data <<=1;
		data |= DHT11_ReadData();
	}
	return data;
}

uint8_t DHT11_Data(uint8_t *temp,uint8_t *tem,uint8_t *humi)
{
	uint8_t data[5],i;
	DHT11_Start();
	if(GPIO_ReadInputDataBit(DHT11_GPIOx,DHT11_GPIO_Pin)==0)
	{
	while(GPIO_ReadInputDataBit(DHT11_GPIOx,DHT11_GPIO_Pin)==0);
	while(GPIO_ReadInputDataBit(DHT11_GPIOx,DHT11_GPIO_Pin)==1);
	for(i=0;i<5;i++)
	{
		data[i]=DHT11_Rec_Data();
	}
	while(GPIO_ReadInputDataBit(DHT11_GPIOx,DHT11_GPIO_Pin)==0);
	DHT11_OUT_Init();
	GPIO_SetBits(DHT11_GPIOx,DHT11_GPIO_Pin);
	
	*humi=data[0];
	*temp=data[2];
	*tem=data[3];
	if(data[4]==data[0]+data[1]+data[2]+data[3])
		return 1;
	else 
		return 0;
	}
	return 0;
}



