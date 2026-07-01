#include "PWM.h"

void PWM_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1 |RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4,ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin=LED_GPIO;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=Motor_GPIO;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=Steer_GPIO;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=Steer2_GPIO;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	TIM_InternalClockConfig(TIM1);
	TIM_InternalClockConfig(TIM3);
	TIM_InternalClockConfig(TIM4);
	
	//LED驱动
	TIM_TimeBaseInitTypeDef TIM_InitStruct;
	TIM_InitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_InitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Period=4095-1;
	TIM_InitStruct.TIM_Prescaler=34-1;
	TIM_InitStruct.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM1,&TIM_InitStruct);
	
	//直流电机驱动
	TIM_InitStruct.TIM_Period=100-1;
	TIM_InitStruct.TIM_Prescaler=36-1;
	TIM_InitStruct.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM3,&TIM_InitStruct);
	
	//舵机驱动
	TIM_InitStruct.TIM_Period=20000-1;
	TIM_InitStruct.TIM_Prescaler=72-1;
	TIM_InitStruct.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM4,&TIM_InitStruct);	
	//LED灯
	TIM_OCInitTypeDef	TIM_OCInitStruct;
	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse=0;
	TIM_OC1Init(TIM1,&TIM_OCInitStruct);
	//直流电机
	TIM_OCInitStruct.TIM_Pulse=0;
	TIM_OC4Init(TIM3,&TIM_OCInitStruct);
	//舵机
	TIM_OCInitStruct.TIM_Pulse=0;
	TIM_OC2Init(TIM4,&TIM_OCInitStruct);
	
	TIM_OCInitStruct.TIM_Pulse=0;
	TIM_OC1Init(TIM4,&TIM_OCInitStruct);
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	TIM_Cmd(TIM4,ENABLE);
	TIM_Cmd(TIM3,ENABLE);
	TIM_Cmd(TIM1,ENABLE);
}

void LED_PWM(uint16_t num)
{
	TIM_SetCompare1(TIM1,num);
}

void Motor_PWM(int8_t num)
{
	TIM_SetCompare4(TIM3,num);
}

void Steer_PWM(uint16_t num)
{
	TIM_SetCompare2(TIM4,num);
}

void Steer_Angle(float Angle)
{
	Steer_PWM(Angle/180*2000+500);
}

void Steer2_PWM(uint16_t num)
{
	TIM_SetCompare1(TIM4,num);
}

void Steer2_Angle(float Angle)
{
	Steer2_PWM(Angle/180*2000+500);
}

void Auto_Motor(int temp,int target_temperature)
{
	int speed;
		if(temp>=target_temperature+3)
		{
		speed=100;
		}
		else if(temp>=target_temperature+2)
		{
			speed=75;
		}
		else if(temp>=target_temperature+1)
		{
			speed=50;
		}
		else if(temp>=target_temperature)
		{
			speed=25;
		}
		else if(temp<target_temperature)
		{
			speed=0;
		}
		Motor_PWM(speed);
}
