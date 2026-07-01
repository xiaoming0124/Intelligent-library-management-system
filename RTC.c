#include "RTC.h"
#include "Systick.h"
#include "USART2.h"
#include <string.h>
#include <stdio.h>

// RTC时间全局变量
RTC_DateTypeDef rtc_time = {0};

// 月份天数表（非闰年）
static const uint8_t month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 判断是否为闰年
static bool IsLeapYear(uint16_t year) {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

// 获取月份天数
static uint8_t GetMonthDays(uint16_t year, uint8_t month) {
    if (month == 2 && IsLeapYear(year)) {
        return 29;
    }
    if (month >= 1 && month <= 12) {
        return month_days[month - 1];
    }
    return 0;
}

// RTC初始化
void RTC_Init(void) {
    // 检查RTC是否已经初始化
    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5) {
        // RTC未初始化，设置为默认时间
        USART2_Print("RTC Initializing with default time...\r\n");
        
        RTC_SetTime(2025, 10, 1, 0, 0, 0);
        
        // 标记RTC已初始化
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    } else {
        USART2_Print("RTC Already Initialized\r\n");
    }
}

// 设置RTC时间
bool RTC_SetTime(uint16_t year, uint8_t month, uint8_t day, 
                uint8_t hour, uint8_t minute, uint8_t second) {
    // 参数检查
    if (year < 1970 || year > 2099 || month < 1 || month > 12 || 
        day < 1 || day > GetMonthDays(year, month) || 
        hour > 23 || minute > 59 || second > 59) {
        USART2_Print("RTC SetTime: Invalid parameters\r\n");
        return false;
    }
    
    // 计算从1970-01-01 00:00:00到目标时间的总秒数
    uint32_t total_seconds = 0;
    
    // 计算年份的秒数
    for (uint16_t y = 1970; y < year; y++) {
        total_seconds += IsLeapYear(y) ? 31622400 : 31536000; // 366天或365天的秒数
    }
    
    // 计算月份的秒数
    for (uint8_t m = 1; m < month; m++) {
        total_seconds += GetMonthDays(year, m) * 86400;
    }
    
    // 计算天数、小时、分钟、秒的秒数
    total_seconds += (day - 1) * 86400;
    total_seconds += hour * 3600;
    total_seconds += minute * 60;
    total_seconds += second;
    
    // 更新RTC时间结构
    rtc_time.total_seconds = total_seconds;
    rtc_time.datetime.year = year;
    rtc_time.datetime.month = month;
    rtc_time.datetime.day = day;
    rtc_time.datetime.hour = hour;
    rtc_time.datetime.minute = minute;
    rtc_time.datetime.second = second;
    rtc_time.datetime.week_day = RTC_CalculateWeekDay(year, month, day);
    
    return true;
}

// 从字符串设置RTC时间（格式：YYYY-MM-DD HH:MM:SS）
bool RTC_SetTimeFromString(const char* time_str) {
    uint16_t year;
    uint8_t month, day, hour, minute, second;
    
    if (sscanf(time_str, "%hu-%hhu-%hhu %hhu:%hhu:%hhu", 
               &year, &month, &day, &hour, &minute, &second) == 6) {
        return RTC_SetTime(year, month, day, hour, minute, second);
    }
    
    USART2_Print("RTC SetTimeFromString: Format error\r\n");
    return false;
}

// 获取RTC时间
void RTC_GetTime(RTC_TimeTypeDef* time_struct) {
    if (time_struct != NULL) {
        *time_struct = rtc_time.datetime;
    }
}

// 获取完整的RTC时间信息
void RTC_GetFullTime(RTC_DateTypeDef* date_struct) {
    if (date_struct != NULL) {
        *date_struct = rtc_time;
    }
}

// 获取时间字符串（HH:MM:SS格式）
void RTC_GetTimeString(char* buffer, uint8_t buffer_size) {
    if (buffer != NULL && buffer_size >= 9) {
        snprintf(buffer, buffer_size, "%02d:%02d:%02d", 
                 rtc_time.datetime.hour, rtc_time.datetime.minute, rtc_time.datetime.second);
    }
}

// 获取日期字符串（HH:MM:SS格式）
void RTC_GetDayString(char* buffer, uint8_t buffer_size) {
    if (buffer != NULL && buffer_size >= 11) {
        snprintf(buffer, buffer_size, "%04d-%02d-%02d", 
                 rtc_time.datetime.year, rtc_time.datetime.month, rtc_time.datetime.day);
    }
}

// 获取日期时间字符串（YYYY-MM-DD HH:MM:SS格式）
void RTC_GetDateTimeString(char* buffer, uint8_t buffer_size) {
    if (buffer != NULL && buffer_size >= 20) {
        snprintf(buffer, buffer_size, "%04d-%02d-%02d %02d:%02d:%02d", 
                 rtc_time.datetime.year, rtc_time.datetime.month, rtc_time.datetime.day,
                 rtc_time.datetime.hour, rtc_time.datetime.minute, rtc_time.datetime.second);
    }
}

// 递增秒数（用于系统时钟滴答）
void RTC_IncrementSecond(void) {
    rtc_time.total_seconds++;
    rtc_time.datetime.second++;
    
    if (rtc_time.datetime.second >= 60) {
        rtc_time.datetime.second = 0;
        rtc_time.datetime.minute++;
        
        if (rtc_time.datetime.minute >= 60) {
            rtc_time.datetime.minute = 0;
            rtc_time.datetime.hour++;
            
            if (rtc_time.datetime.hour >= 24) {
                rtc_time.datetime.hour = 0;
                rtc_time.datetime.day++;
                rtc_time.datetime.week_day = (rtc_time.datetime.week_day + 1) % 7;
                
                if (rtc_time.datetime.day > GetMonthDays(rtc_time.datetime.year, rtc_time.datetime.month)) {
                    rtc_time.datetime.day = 1;
                    rtc_time.datetime.month++;
                    
                    if (rtc_time.datetime.month > 12) {
                        rtc_time.datetime.month = 1;
                        rtc_time.datetime.year++;
                    }
                }
            }
        }
    }
}

// 检查时间是否有效
bool RTC_IsTimeValid(void) {
    return (rtc_time.datetime.year >= 1970 && rtc_time.datetime.year <= 2099 &&
            rtc_time.datetime.month >= 1 && rtc_time.datetime.month <= 12 &&
            rtc_time.datetime.day >= 1 && rtc_time.datetime.day <= GetMonthDays(rtc_time.datetime.year, rtc_time.datetime.month) &&
            rtc_time.datetime.hour <= 23 && rtc_time.datetime.minute <= 59 && rtc_time.datetime.second <= 59);
}

// 获取Unix时间戳
uint32_t RTC_GetUnixTime(void) {
    return rtc_time.total_seconds;
}

// 设置Unix时间戳
void RTC_SetUnixTime(uint32_t unix_time) {
    rtc_time.total_seconds = unix_time;
    
    // 将Unix时间戳转换为日期时间
    uint32_t seconds_remaining = unix_time;
    
    // 计算年份
    rtc_time.datetime.year = 1970;
    while (seconds_remaining >= (IsLeapYear(rtc_time.datetime.year) ? 31622400 : 31536000)) {
        seconds_remaining -= IsLeapYear(rtc_time.datetime.year) ? 31622400 : 31536000;
        rtc_time.datetime.year++;
    }
    
    // 计算月份
    rtc_time.datetime.month = 1;
    while (seconds_remaining >= (GetMonthDays(rtc_time.datetime.year, rtc_time.datetime.month) * 86400)) {
        seconds_remaining -= GetMonthDays(rtc_time.datetime.year, rtc_time.datetime.month) * 86400;
        rtc_time.datetime.month++;
    }
    
    // 计算天数
    rtc_time.datetime.day = seconds_remaining / 86400 + 1;
    seconds_remaining %= 86400;
    
    // 计算小时
    rtc_time.datetime.hour = seconds_remaining / 3600;
    seconds_remaining %= 3600;
    
    // 计算分钟和秒
    rtc_time.datetime.minute = seconds_remaining / 60;
    rtc_time.datetime.second = seconds_remaining % 60;
    
    // 计算星期
    rtc_time.datetime.week_day = RTC_CalculateWeekDay(rtc_time.datetime.year, rtc_time.datetime.month, rtc_time.datetime.day);
}

// 计算星期几（Zeller公式）
uint8_t RTC_CalculateWeekDay(uint16_t year, uint8_t month, uint8_t day) {
    if (month < 3) {
        month += 12;
        year--;
    }
    
    uint16_t century = year / 100;
    uint16_t year_of_century = year % 100;
    
    // Zeller's Congruence公式
    uint16_t h = (day + (13 * (month + 1)) / 5 + year_of_century + 
                 year_of_century / 4 + century / 4 + 5 * century) % 7;
    
    // 调整结果：0=周六, 1=周日, 2=周一, ..., 6=周五
    // 转换为：0=周日, 1=周一, ..., 6=周六
    return (h + 5) % 7 + 1;
}
