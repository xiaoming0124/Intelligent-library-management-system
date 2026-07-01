#include "Key.h"

int Key_Model = 0x02;

void System_Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_InitStruct.GPIO_Pin = System_Key | Motor_Key | Steer_Key | Light_Key | Infrared;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(Key_GPIOx, &GPIO_InitStruct);    
}

void Infrared_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin =Infrared;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(Key_GPIOx, &GPIO_InitStruct); 
}

uint8_t Infrared_Get(void)
{
	if(GPIO_ReadInputDataBit(GPIOB,Infrared))
	{
		vTaskDelay(pdMS_TO_TICKS(100));
		if(GPIO_ReadInputDataBit(GPIOB,Infrared)==1)
			return 1;
		else 
			return 0;
	}
}

void Get_KeyNum(void)
{
	uint16_t time=0;
	if(GPIO_ReadInputDataBit(GPIOB,System_Key))
	{
		vTaskDelay(pdMS_TO_TICKS(500));	
		if(GPIO_ReadInputDataBit(GPIOB,System_Key))
		{	
			while(GPIO_ReadInputDataBit(GPIOB,System_Key))
			{
				time++;
				for(uint8_t i=0;i<100;i++);
				if(time>10000)
				{
					Key_Model|=0x80;
					Key_Model&=0xBF;
					break;
				}
			}
			while(GPIO_ReadInputDataBit(GPIOB,System_Key))
				vTaskDelay(pdMS_TO_TICKS(100));
			if(time<10000)
			{
				Key_Model|=0x40;
				Key_Model&=0x7F;
			}
		}
		else 
		{	
			Key_Model|=0x40;
			Key_Model&=0x7F;
		}
	}	
	else if(GPIO_ReadInputDataBit(GPIOB,Motor_Key))
	{
		vTaskDelay(pdMS_TO_TICKS(500));	
		if(GPIO_ReadInputDataBit(GPIOB,Motor_Key))
		{	
			while(GPIO_ReadInputDataBit(GPIOB,Motor_Key))
			{
				time++;
				for(uint8_t i=0;i<100;i++);		
				if(time>=10000)
				{
					Key_Model|=0x20;
					Key_Model&=0xEF;
					break;
				}
			}
			while(GPIO_ReadInputDataBit(GPIOB,Motor_Key))
				vTaskDelay(pdMS_TO_TICKS(100));
			if(time<10000) 
				{
					Key_Model&=0xDF;
					Key_Model|=0x10;		
				}
		}
		else 
		{
			Key_Model&=0xDF;
			Key_Model|=0x10;		
		}
	}		
	else if(GPIO_ReadInputDataBit(GPIOB,Steer_Key))
	{
		vTaskDelay(pdMS_TO_TICKS(500));	
		if(GPIO_ReadInputDataBit(GPIOB,Steer_Key))
		{	
			while(GPIO_ReadInputDataBit(GPIOB,Steer_Key))
			{
				time++;
				for(uint8_t i=0;i<100;i++);
				if(time>=10000)
				{
					Key_Model|=0x08;
					break;
				}			
			}
			while(GPIO_ReadInputDataBit(GPIOB,Steer_Key))
				vTaskDelay(pdMS_TO_TICKS(100));
			if(time<10000)
				Key_Model|=0x04;
		}
		else 
			Key_Model|=0x04;
	}	
	else if(GPIO_ReadInputDataBit(GPIOB,Light_Key))
	{
		vTaskDelay(pdMS_TO_TICKS(500));	
		if(GPIO_ReadInputDataBit(GPIOB,Light_Key))
		{			
			while(GPIO_ReadInputDataBit(GPIOB,Light_Key))
			{
				time++;
				for(uint8_t i=0;i<100;i++);
				if(time>=10000)
				{
					Key_Model|=0x02;
					Key_Model&=0xFE;
					break;
				}
			}
			while(GPIO_ReadInputDataBit(GPIOB,Light_Key))
			vTaskDelay(pdMS_TO_TICKS(100));
			if(time<10000)
			{
				Key_Model&=0xFD;
				Key_Model|=0x01;
			}		
		}
		else 
		{
			Key_Model&=0xFD;
			Key_Model|=0x01;
		}	
	}
		time=0;
}
