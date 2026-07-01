#include "stm32f10x.h"                  // Device header
#include "USART2.h"
#include <stdio.h>
uint8_t usart2_data=0,usart3_data=0;
uint8_t flag=0,usart3_flag=0;

void USART2_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 | RCC_APB1Periph_USART3,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitSturct;
	GPIO_InitSturct.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitSturct.GPIO_Pin=USART2_RX;
	GPIO_InitSturct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(USART2_GPIO,&GPIO_InitSturct);
	
	GPIO_InitSturct.GPIO_Pin=USART3_RX;
	GPIO_Init(USART3_GPIO,&GPIO_InitSturct);
	
	GPIO_InitSturct.GPIO_Mode=GPIO_Mode_AF_PP ;
	GPIO_InitSturct.GPIO_Pin=USART2_TX;
	GPIO_InitSturct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(USART2_GPIO,&GPIO_InitSturct);
	
	GPIO_InitSturct.GPIO_Pin=USART3_TX;
	GPIO_Init(USART3_GPIO,&GPIO_InitSturct);
	
	USART_InitTypeDef USART_InitSturct;
	USART_InitSturct.USART_BaudRate=115200;
	USART_InitSturct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitSturct.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;
	USART_InitSturct.USART_Parity=USART_Parity_No;
	USART_InitSturct.USART_StopBits=USART_StopBits_1;
	USART_InitSturct.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART2,&USART_InitSturct);
	USART_Init(USART3,&USART_InitSturct);
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitSturct;
	NVIC_InitSturct.NVIC_IRQChannel=USART2_IRQn;
	NVIC_InitSturct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitSturct.NVIC_IRQChannelPreemptionPriority=6;
	NVIC_InitSturct.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitSturct);
	
	NVIC_InitSturct.NVIC_IRQChannel=USART3_IRQn;
	NVIC_InitSturct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitSturct.NVIC_IRQChannelPreemptionPriority=7;
	NVIC_InitSturct.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitSturct);
	
	USART_Cmd(USART2,ENABLE);
	USART_Cmd(USART3,ENABLE);
}

void USART2_Send(uint16_t data)
{
	USART_SendData(USART2,data);
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);
}

void USART3_Send(uint16_t data)
{
	USART_SendData(USART3,data);
	while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
}

void USART2_SendString(char* data)
{
	uint8_t i;
	for(i=0;data[i]!='\0';i++)
	{
		USART_SendData(USART2,data[i]);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==0);
	}
}

void USART3_SendString(char* data)
{
	uint8_t i;
	for(i=0;data[i]!='\0';i++)
	{
		USART_SendData(USART3,data[i]);
		while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==0);
	}
}

uint8_t USART2_GetFlag()
{
	if(flag == 1)
	{	
		flag=0;
		return 1;
	}
	else
		return 0;
}

bool USART3_GetFlag(void)
{
	if(usart3_flag == 1)
	{	
		if(usart3_data==0x88)
		{
			usart3_flag=0;
			usart3_data=0;
			return 1;
		}
		else
		{
			usart3_flag=0;
			usart3_data=0;
			return 0;
		}
	}
	
}

int fputc(int ch, FILE *f)
{
	USART2_Send(ch);			//将printf的底层重定向到自己的发送字节函数
	return ch;
}

uint8_t USART_GetData()
{
	return usart2_data;
}

void USART2_IRQHandler()
{
	if(USART_GetITStatus(USART2,USART_IT_RXNE)==SET)
	{
		usart2_data = USART_ReceiveData(USART2);
		flag=1;
		USART2_Send(usart2_data);	
	}
	USART_ClearITPendingBit(USART2,USART_IT_RXNE);
}

void USART3_IRQHandler()
{
	if(USART_GetITStatus(USART3,USART_IT_RXNE)==SET)
	{
		usart3_data = USART_ReceiveData(USART3);
		USART3_Send(usart3_data);	
		usart3_flag=1;
	}
	USART_ClearITPendingBit(USART3,USART_IT_RXNE);
}
