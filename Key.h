#ifndef __KEY_H
#define __KEY_H
#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "FreeRTOS.h"
#include "task.h"

extern int Key_Model; 
extern volatile uint8_t LowPowerMode;  // 添加低功耗模式标志

#define Key_GPIOx		GPIOB
#define	System_Key	GPIO_Pin_12
#define	Motor_Key	GPIO_Pin_13
#define	Steer_Key	GPIO_Pin_14
#define	Light_Key	GPIO_Pin_15
#define Infrared  GPIO_Pin_5

void Infrared_Init(void);
void System_Key_Init(void);
void Get_KeyNum(void);
uint8_t Infrared_Get(void);
	
#endif
