#ifndef __FLAME_H
#define __FLAME_H

#include "stm32f10x.h"

extern volatile uint8_t Flame_flag;

// 初始化PA5为外部中断模式
void FlameSensor_Init(void);
void RainSensor_Init(void);          // 雨量传感器初始化
uint8_t Read_RainSensor(void);       // 读取雨量传感器状态

#endif
