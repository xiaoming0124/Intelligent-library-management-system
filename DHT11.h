#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f10x.h"                  // Device header
#include "SysTick.h"
#include "FreeRTOS.h"
#include "task.h"

#define DHT11_GPIOx 		GPIOA
#define DHT11_GPIO_Pin	GPIO_Pin_4
#define DHT11_RCC				RCC_APB2Periph_GPIOA

void DHT11_OUT_Init(void);
void DHT11_IN_Init(void);
void DHT11_Start(void);
uint8_t DHT11_ReadData(void);
uint8_t DHT11_Rec_Data(void);
uint8_t DHT11_Data(uint8_t *temp,uint8_t *tem,uint8_t *humi);

#endif
