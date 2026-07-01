#include "Beep.h"

void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 启用GPIO时钟
    RCC_APB2PeriphClockCmd(BUZZER_RCC, ENABLE);
    
    // 配置蜂鸣器引脚为推挽输出
    GPIO_InitStructure.GPIO_Pin = BUZZER_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50MHz速度
    GPIO_Init(BUZZER_GPIO_PORT, &GPIO_InitStructure);
    
    // 初始状态关闭蜂鸣器
    BUZZER_OFF();
}
