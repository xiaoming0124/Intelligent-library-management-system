#ifndef __RTC_H
#define __RTC_H

#include "stm32f10x.h"
#include <stdbool.h>
#include <time.h>
#include "ESP8266.h"

// RTC时间结构体
typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t week_day; // 0=周日, 1=周一, ..., 6=周六
} RTC_TimeTypeDef;

// RTC日期结构体
typedef struct {
    uint32_t total_seconds;  // 从1970-01-01 00:00:00开始的秒数
    RTC_TimeTypeDef datetime;
} RTC_DateTypeDef;

// 函数声明
void RTC_Init(void);
bool RTC_SetTime(uint16_t year, uint8_t month, uint8_t day, 
                uint8_t hour, uint8_t minute, uint8_t second);
bool RTC_SetTimeFromString(const char* time_str);
void RTC_GetTime(RTC_TimeTypeDef* time_struct);
void RTC_GetFullTime(RTC_DateTypeDef* date_struct);
void RTC_GetTimeString(char* buffer, uint8_t buffer_size);
void RTC_GetDayString(char* buffer, uint8_t buffer_size);
void RTC_GetDateTimeString(char* buffer, uint8_t buffer_size);
void RTC_IncrementSecond(void);
bool RTC_IsTimeValid(void);
uint32_t RTC_GetUnixTime(void);
void RTC_SetUnixTime(uint32_t unix_time);
uint8_t RTC_CalculateWeekDay(uint16_t year, uint8_t month, uint8_t day);

// 外部变量声明
extern RTC_DateTypeDef rtc_time;

#endif
