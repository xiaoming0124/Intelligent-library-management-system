#ifndef __ESP8266_H
#define __ESP8266_H
#include "USART2.h"
#include "Convert.h"
#include <stdio.h>
#include <stdbool.h>
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include <time.h>
#include <stdlib.h>

/// MQTT配置参数
#define WiFiname "ming"
#define Wpassword "123454321"
#define Username "test&k29urbBNUR5"
#define Userpassward "d2168b0a2f03b8abcafca9879c206810d0d81f841190de2486afb683d6a06c6a"
#define ID "k29urbBNUR5.test|securemode=2\\,signmethod=hmacsha256\\,timestamp=1745583921291|"
#define Url "iot-06z00d49iqnt922.mqtt.iothub.aliyuncs.com"
#define Device_name "test"

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

#define RX_MAX_LEN 1024

extern struct STRUCT_USARTx_Fram
{
    char DATA_RX_BUF [RX_MAX_LEN];
    union {
    volatile uint16_t InfAll;
        struct{
            volatile uint16_t FramLenth :15;
            volatile uint16_t Framflag :1;			
        } InfBit;
    };
} strESP8288_Fram;

#define ESP8266_GPIO GPIOA
#define ESP8266_TX GPIO_Pin_9
#define ESP8266_RX GPIO_Pin_10
#define ESP8266_USART USART1

#define ESP8266_Print( fmt, ... ) USART_printf ( USART1, fmt, ##__VA_ARGS__ )
#define PC_Print( fmt, ... ) printf (fmt, ##__VA_ARGS__ )
#define USART2_Print( fmt, ... ) USART_printf ( USART2, fmt, ##__VA_ARGS__ )

// 函数声明
void ESP8266_USART_Init(void);
void ESP8266_Send(uint16_t data);
void ESP8266_SendString(char* data);
bool ESP8266_Cmd(char* cmd,char* delay1,char* delay2,uint16_t cont);
bool ESP8266_AT(void);
bool ESP8266_MODE(uint8_t num);
bool ESP8266_CWJAP(char* name,char* password);
bool ESP8266_MQTTUSERCFG(char* username,char* password);
bool ESP8266_MQTTCLIENTID(char* Clientid);
bool ESP8266_MQTTCONN(char* URL);
bool ESP8266_MQTTSUB(char* name);
void ESP8266_Init(void);
void ESP8266_Send_MQTTPUB(char* name,uint8_t temp,uint8_t tem,uint8_t humi,int Flame_Flage,int Smoke_Num,int TargetTemperature,int open);
bool ESP8266_GetNTPTime(char* ntp_server, char* time_buffer);
bool ESP8266_GetSimpleNTPTime(char* time_buffer);
uint8_t ESP8266_GetFlag(void);
void ESP8266_Getdata(int* temp,bool* open,bool* open_flag);
void ESP8266_Send_FlameMQTTPUB(char* name,int Flame_Flage);
void ESP8266_Send_OPENMQTTPUB(char* name,bool open);
#endif
