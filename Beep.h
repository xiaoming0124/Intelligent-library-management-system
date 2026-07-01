#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f10x.h"

// 蜂鸣器引脚定义
#define BUZZER_GPIO_PORT    GPIOB
#define BUZZER_GPIO_PIN     GPIO_Pin_5
#define BUZZER_RCC          RCC_APB2Periph_GPIOB

// 蜂鸣器状态控制
#define BUZZER_ON()         GPIO_SetBits(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN)
#define BUZZER_OFF()        GPIO_ResetBits(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN)

// 初始化函数
void Buzzer_Init(void);

#endif /* __BUZZER_H */
