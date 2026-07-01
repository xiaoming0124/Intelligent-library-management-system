#ifndef __USART2_H
#define __USART2_H

#include <stdbool.h>

#define USART2_GPIO GPIOA
#define USART2_TX		GPIO_Pin_2
#define USART2_RX		GPIO_Pin_3

#define USART3_GPIO GPIOB
#define USART3_TX		GPIO_Pin_10
#define USART3_RX		GPIO_Pin_11

void USART2_Init(void);
void USART2_Send(uint16_t data);
void USART2_SendString(char* data);
void USART3_Send(uint16_t data);
void USART3_SendString(char* data);
uint8_t USART2_GetFlag(void);
bool USART3_GetFlag(void);
uint8_t USART_GetData(void);

#endif
