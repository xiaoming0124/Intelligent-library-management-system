#ifndef __Light_H
#define __Light_H
#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "FreeRTOS.h"
#include "task.h"

#define	ADCx				ADC1
#define ADC_GPIO		GPIOA
#define Light_GPIO	GPIO_Pin_0
#define Smoke_GPIO	GPIO_Pin_1

extern uint16_t data[2];
extern int Smoke_Flag;

void Light_Init(void);
void Auto_Light(void);

float ConvertToLux(void);
float ConvertToPPM(void);

#endif
