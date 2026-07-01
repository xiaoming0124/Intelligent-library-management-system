#ifndef __PWM_H
#define __PWM_H
#include "stm32f10x.h"                  // Device header

#define	LED_GPIO	GPIO_Pin_8
#define	Motor_GPIO	GPIO_Pin_1
#define	Steer_GPIO	GPIO_Pin_7
#define	Steer2_GPIO	GPIO_Pin_6

void PWM_Init(void);
void LED_PWM(uint16_t num);
void Motor_PWM(int8_t num);
void Steer_PWM(uint16_t num);
void Steer_Angle(float Angle);
void Steer2_PWM(uint16_t num);
void Steer2_Angle(float Angle);
void Auto_Motor(int temp,int target_temperature);

#endif
