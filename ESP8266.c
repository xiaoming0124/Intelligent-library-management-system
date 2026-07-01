#include "stm32f10x.h"
#include "SysTick.h"
#include "ESP8266.h"
#include <string.h>
#include <stdbool.h>

uint8_t flag2;
uint16_t usart1_data;
struct STRUCT_USARTx_Fram strESP8266_Fram={0};
int Light_Num=0;

// 原有的NTP相关函数保持不变
// ?謹?屢?瞳.c匡?櫓，??路??屢
typedef struct {
    uint32_t seconds;
    uint32_t fraction;
} ntp_timestamp_t;

typedef struct {
    uint8_t li_vn_mode;
    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t reference_id;
    ntp_timestamp_t ref_ts;
    ntp_timestamp_t orig_ts;
    ntp_timestamp_t recv_ts;
    ntp_timestamp_t trans_ts;
} ntp_packet_t;

uint32_t ntohl(uint32_t x) {
    return ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) | 
           ((x & 0xFF0000) >> 8) | ((x & 0xFF000000) >> 24);
}

// 创建NTP数据包
void create_ntp_packet(ntp_packet_t *packet) {
    memset(packet, 0, sizeof(ntp_packet_t));
    packet->li_vn_mode = (0x03 << 3) | 0x03; // 版本3，客户端模式
}

// 将NTP时间戳转换为UNIX时间戳
uint32_t ntp_to_unix_time(ntp_timestamp_t ntp_ts) {
    // NTP时间戳从1900年1月1日开始，UNIX从1970年1月1日开始
    // 需要减去2208988800秒
    const uint32_t ntp_to_unix_seconds = 2208988800UL;
    return ntohl(ntp_ts.seconds) - ntp_to_unix_seconds;
}

// UNIX时间戳转换为日期时间
void unix_time_to_datetime(uint32_t unix_time, uint16_t* year, uint8_t* month, uint8_t* day, 
                          uint8_t* hour, uint8_t* minute, uint8_t* second) {
    // 简化的UNIX时间戳转换（不考虑闰秒等复杂情况）
    uint32_t days = unix_time / 86400;
    uint32_t seconds_in_day = unix_time % 86400;
    
    *hour = seconds_in_day / 3600;
    *minute = (seconds_in_day % 3600) / 60;
    *second = seconds_in_day % 60;
    
    // 简化计算：从1970年开始
    *year = 1970;
    while (days >= 365) {
        if ((*year % 4 == 0 && *year % 100 != 0) || (*year % 400 == 0)) {
            if (days >= 366) {
                days -= 366;
                (*year)++;
            } else {
                break;
            }
        } else {
            days -= 365;
            (*year)++;
        }
    }
    
    uint8_t month_days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if ((*year % 4 == 0 && *year % 100 != 0) || (*year % 400 == 0)) {
        month_days[1] = 29;
    }
    
    *month = 1;
    for (int i = 0; i < 12; i++) {
        if (days < month_days[i]) {
            *day = days + 1;
            break;
        }
        days -= month_days[i];
        (*month)++;
    }
}

// 获取NTP时间的函数 - 只使用阿里云NTP服务器
bool ESP8266_GetNTPTime(char* ntp_server, char* time_buffer) {
    char cmd[128];
    ntp_packet_t ntp_packet;
    
    // 清除接收缓冲区
    strESP8266_Fram.InfBit.FramLenth = 0;
    memset(strESP8266_Fram.DATA_RX_BUF, 0, RX_MAX_LEN);
    
    // 建立UDP连接
    sprintf(cmd, "AT+CIPSTART=\"UDP\",\"%s\",123\r\n", ntp_server);
    if(!ESP8266_Cmd(cmd, "OK", "CONNECT", 5000)) {
        USART2_Print("UDP connect failed to %s\r\n", ntp_server);
        return false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 创建NTP数据包
    create_ntp_packet(&ntp_packet);
    
    // 发送NTP数据包
    sprintf(cmd, "AT+CIPSEND=%d\r\n", sizeof(ntp_packet));
    if(!ESP8266_Cmd(cmd, ">", NULL, 2000)) {
        USART2_Print("Send prompt failed\r\n");
        return false;
    }
    
    // 发送NTP数据包
    for(uint16_t i = 0; i < sizeof(ntp_packet); i++) {
        USART_SendData(ESP8266_USART, ((uint8_t*)&ntp_packet)[i]);
        while(USART_GetFlagStatus(ESP8266_USART, USART_FLAG_TXE) == RESET);
    }
    
    // 发送结束标志
    ESP8266_SendString("\r\n");
    
    // 等待响应
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 检查是否有响应数据
    if (strESP8266_Fram.InfBit.FramLenth == 0) {
        USART2_Print("No response from NTP server\r\n");
        return false;
    }
    
    // 查找IPD数据
    char *ipd_start = strstr(strESP8266_Fram.DATA_RX_BUF, "+IPD");
    if(ipd_start != NULL) {
        // 查找数据起始位置
        char *data_start = strchr(ipd_start, ':');
        if(data_start != NULL) {
            data_start++; // 跳过冒号
            
            // 确保有足够的数据
            if(strESP8266_Fram.InfBit.FramLenth - (data_start - strESP8266_Fram.DATA_RX_BUF) >= sizeof(ntp_packet_t)) {
                ntp_packet_t response;
                memcpy(&response, data_start, sizeof(ntp_packet_t));
                
                // 转换时间
                uint32_t unix_time = ntp_to_unix_time(response.trans_ts);
                
                // 转换为日期时间
                uint16_t year;
                uint8_t month, day, hour, minute, second;
                unix_time_to_datetime(unix_time, &year, &month, &day, &hour, &minute, &second);
                
                sprintf(time_buffer, "%04d-%02d-%02d %02d:%02d:%02d",
                       year, month, day, hour+8, minute, second);
                
                USART2_Print("NTP time from %s: %s\r\n", ntp_server, time_buffer);
                return true;
            }
        }
    }
    
    USART2_Print("Failed to parse NTP response\r\n");
    return false;
}

bool ESP8266_GetSimpleNTPTime(char* time_buffer) {

    const char* aliyun_ntp_servers[] = {
        "ntp1.aliyun.com",    
        "ntp2.aliyun.com",    
        "ntp3.aliyun.com",    
        "ntp4.aliyun.com",    
        "ntp5.aliyun.com",    
        "ntp6.aliyun.com",    
        "ntp7.aliyun.com" 
    };
    
    USART2_Print("Trying to get time from Aliyun NTP servers...\r\n");
    
    for(int i = 0; i < 7; i++) {
        USART2_Print("Trying server: %s\r\n", aliyun_ntp_servers[i]);
        if(ESP8266_GetNTPTime((char*)aliyun_ntp_servers[i], time_buffer)) {
            USART2_Print("Successfully got time from Aliyun NTP server: %s\r\n", aliyun_ntp_servers[i]);
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // 所有阿里云服务器失败，返回默认时间
    USART2_Print("All Aliyun NTP servers failed, using default time\r\n");
    sprintf(time_buffer, "2025-10-01 00:00:00");
    return false;
}


void ESP8266_USART_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitSturct;
	GPIO_InitSturct.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitSturct.GPIO_Pin=ESP8266_RX;
	GPIO_InitSturct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(ESP8266_GPIO,&GPIO_InitSturct);
	
	GPIO_InitSturct.GPIO_Mode=GPIO_Mode_AF_PP ;
	GPIO_InitSturct.GPIO_Pin=ESP8266_TX;
	GPIO_InitSturct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(ESP8266_GPIO,&GPIO_InitSturct);
	
	USART_InitTypeDef USART_InitSturct;
	USART_InitSturct.USART_BaudRate=115200;
	USART_InitSturct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitSturct.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;
	USART_InitSturct.USART_Parity=USART_Parity_No;
	USART_InitSturct.USART_StopBits=USART_StopBits_1;
	USART_InitSturct.USART_WordLength=USART_WordLength_8b;
	USART_Init(ESP8266_USART,&USART_InitSturct);
	
	USART_ITConfig(ESP8266_USART,USART_IT_RXNE,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitSturct;
	NVIC_InitSturct.NVIC_IRQChannel=USART1_IRQn;
	NVIC_InitSturct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitSturct.NVIC_IRQChannelPreemptionPriority=6;
	NVIC_InitSturct.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitSturct);
	
	USART_Cmd(ESP8266_USART,ENABLE);
}

void ESP8266_Send(uint16_t data)
{
	USART_SendData(ESP8266_USART,data);
	while(USART_GetFlagStatus(ESP8266_USART,USART_FLAG_TXE)==RESET);
}

void ESP8266_SendString(char* data)
{
	uint8_t u;
	for(u=0;data[u] != '\0';u++)
	{
		USART_SendData(ESP8266_USART,data[u]);
		while(USART_GetFlagStatus(ESP8266_USART,USART_FLAG_TXE)==RESET);
	}
}

bool ESP8266_Cmd(char* cmd,char* delay1,char* delay2,uint16_t cont)
{
	strESP8266_Fram.InfBit.FramLenth=0;
	ESP8266_Print("%s\r\n",cmd);
	if((delay1==0)&&(delay2==0))
		return true;
	vTaskDelay(pdMS_TO_TICKS(cont));
	strESP8266_Fram.DATA_RX_BUF[strESP8266_Fram.InfBit.FramLenth]='\0';
	USART2_Print("%s",strESP8266_Fram.DATA_RX_BUF);
	if((delay1 != 0) && (delay2 != 0))
		return ((bool)strstr(strESP8266_Fram.DATA_RX_BUF,delay1)||(bool)strstr(strESP8266_Fram.DATA_RX_BUF,delay2));
	else if(delay1 != 0)
		return ((bool)strstr(strESP8266_Fram.DATA_RX_BUF,delay1));
	else if(delay2 != 0)
		return ((bool)strstr(strESP8266_Fram.DATA_RX_BUF,delay2));
}



bool ESP8266_AT(void)
{	
	return ESP8266_Cmd("AT\r\n","OK",NULL,2500);	
}

bool ESP8266_MODE(uint8_t num)
{	
	char Lenth[120];
	sprintf(Lenth,"AT+CWMODE=%d\r\n",num);
	return ESP8266_Cmd("AT","OK",NULL,2500);	
}

bool ESP8266_CWJAP(char* name,char* password)
{	
	char Lenth[120];
	sprintf(Lenth,"AT+CWJAP=\"%s\",\"%s\"\r\n",name,password);
	return ESP8266_Cmd(Lenth,"OK",NULL,2500);	
}

bool ESP8266_MQTTUSERCFG(char* username,char* password)
{	
	char Lenth[120];
	sprintf(Lenth,"AT+MQTTUSERCFG=0,1,\"NULL\",\"%s\",\"%s\",0,0,\"\"\r\n",username,password);
	return ESP8266_Cmd(Lenth,"OK",NULL,2500);	
}

bool ESP8266_MQTTCLIENTID(char* Clientid)
{	
	char Lenth[120];
	sprintf(Lenth,"AT+MQTTCLIENTID=0,\"%s\"\r\n",Clientid);
	return ESP8266_Cmd(Lenth,"OK",NULL,2500);	
}

bool ESP8266_MQTTCONN(char* URL)
{	
	char Lenth[120];
	sprintf(Lenth,"AT+MQTTCONN=0,\"%s\",1883,1\r\n",URL);
	return ESP8266_Cmd(Lenth,"OK",NULL,2500);	
}

bool ESP8266_MQTTSUB(char* name)
{	
	char Lenth[120];
	sprintf(Lenth,"AT+MQTTSUB=0,\"/k29urbBNUR5/%s/user/get\",1\r\n",name);
	return ESP8266_Cmd(Lenth,"OK",NULL,2500);	
}

uint8_t ESP8266_GetFlag(void)
{
	if(ESP8266_Cmd("AT","OK",NULL,2500))
		return 1;
	else
		return 0;
}

void ESP8266_Init(void)
{
	if(ESP8266_AT())
	{
//		ESP8266_MODE(1);
//		ESP8266_CWJAP(WiFiname,Wpassword);
		while(ESP8266_MQTTUSERCFG(Username,Userpassward)==0)
			vTaskDelay(pdMS_TO_TICKS(1000));
		while(ESP8266_MQTTCLIENTID(ID)==0)
			vTaskDelay(pdMS_TO_TICKS(1000));
		while(ESP8266_MQTTCONN(Url)==0)
			vTaskDelay(pdMS_TO_TICKS(1000));
		while(ESP8266_MQTTSUB(Device_name)==0)
			vTaskDelay(pdMS_TO_TICKS(1000));;
	}
}

void ESP8266_Send_MQTTPUB(char* name,uint8_t temp,uint8_t tem,uint8_t humi,int Flame_Flage,int Smoke_Num,int TargetTemperature,int open)
{	
	char Lenth[256];
	sprintf(Lenth,"AT+MQTTPUB=0,\"/sys/k29urbBNUR5/%s/thing/event/property/post\",\"{\\\"params\\\":{\\\"temperature\\\":%d.%d\\,\\\"Flame_State\\\":%d\\,\\\"SmokeSensorState\\\":%d\\,\\\"Humidity\\\":%d\\,\\\"TargetTemperature\\\":%d\\,\\\"OPEN\\\":%d}}\",0,0",name,temp,tem,Flame_Flage,Smoke_Num,humi,TargetTemperature,open);
	ESP8266_Print("%s\r\n",Lenth);
}

void ESP8266_Send_FlameMQTTPUB(char* name,int Flame_Flage)
{	
	char Lenth[256];
	sprintf(Lenth,"AT+MQTTPUB=0,\"/sys/k29urbBNUR5/%s/thing/event/property/post\",\"{\\\"params\\\":{\\\"Flame_State\\\":%d}}\",0,0",name,Flame_Flage);
	ESP8266_Print("%s\r\n",Lenth);
}

void ESP8266_Send_OPENMQTTPUB(char* name,bool open)
{	
	char Lenth[256];
	sprintf(Lenth,"AT+MQTTPUB=0,\"/sys/k29urbBNUR5/%s/thing/event/property/post\",\"{\\\"params\\\":{\\\"OPEN\\\":%d}}\",0,0",name,open);
	ESP8266_Print("%s\r\n",Lenth);
}

void USART1_IRQHandler()
{
	if(USART_GetITStatus(ESP8266_USART,USART_IT_RXNE)==SET)
	{
		usart1_data = USART_ReceiveData(ESP8266_USART);
		if(strESP8266_Fram.InfBit.FramLenth < RX_MAX_LEN - 1)
		{
			strESP8266_Fram.DATA_RX_BUF[strESP8266_Fram.InfBit.FramLenth++]=usart1_data;
		}
		USART_ClearITPendingBit(ESP8266_USART,USART_IT_RXNE);
	}
	if(USART_GetITStatus(ESP8266_USART,USART_IT_IDLE)==SET)
	{
		strESP8266_Fram.InfBit.Framflag=1;
		usart1_data = USART_ReceiveData(ESP8266_USART); // 读取DR以清除IDLE标志
		// 确保字符串结束
		if(strESP8266_Fram.InfBit.FramLenth < RX_MAX_LEN)
		{
			strESP8266_Fram.DATA_RX_BUF[strESP8266_Fram.InfBit.FramLenth]='\0';
			
		}
		else
		{
			strESP8266_Fram.DATA_RX_BUF[RX_MAX_LEN-1]='\0';
			strESP8266_Fram.InfBit.FramLenth = RX_MAX_LEN - 1;
		}
		USART_ClearITPendingBit(ESP8266_USART,USART_IT_IDLE);
	}
}

void ESP8266_Getdata(int* temp,bool* open,bool* open_flag)
{
    strESP8266_Fram.DATA_RX_BUF[strESP8266_Fram.InfBit.FramLenth]='\0';
    
    // 查找TargetTemperature的value值
    char* target_temp_start = strstr(strESP8266_Fram.DATA_RX_BUF, "TargetTemperature");
    // 查找OPEN的value值
    char* target_open_start = strstr(strESP8266_Fram.DATA_RX_BUF, "OPEN");
    
    // 处理TargetTemperature数据
    if(target_temp_start != NULL)
    {
        // 在TargetTemperature对象中查找value字段
        char* value_start = strstr(target_temp_start, "\"value\":");
        if(value_start != NULL)
        {
            value_start += 8; // 跳过 "\"value\":"
            
            // 跳过可能的空格
            while(*value_start == ' ' || *value_start == '\t' || *value_start == '\n' || *value_start == '\r')
            {
                value_start++;
            }
            
            char value_str[16] = {0};
            int i = 0;
            
            // 提取数字（支持负数和整数）
            if(*value_start == '-') // 处理负数
            {
                if(i < 15)
                {
                    value_str[i++] = '-';
                    value_start++;
                }
            }
            
            // 提取数字部分
            while(*value_start >= '0' && *value_start <= '9')
            {
                if(i < 15)
                {
                    value_str[i++] = *value_start;
                    value_start++;
                }
                else
                {
                    break;
                }
            }
            value_str[i] = '\0';
            
            if(strlen(value_str) > 0 && !(strlen(value_str) == 1 && value_str[0] == '-'))
            {
                // 转换为int类型
                int target_temperature = atoi(value_str);
                *temp = target_temperature;
                USART2_Print("Parsed TargetTemperature: %d\r\n", target_temperature);
            }
            else
            {
                USART2_Print("Failed to extract valid temperature value\r\n");
            }
        }
        else
        {
            USART2_Print("Value field not found in TargetTemperature\r\n");
        }
    }
    
    // 处理OPEN数据
    if(target_open_start != NULL)
    {
        // 在OPEN对象中查找value字段
        char* value_start = strstr(target_open_start, "\"value\":");
        if(value_start != NULL)
        {
            value_start += 8; // 跳过 "\"value\":"
            
            // 跳过可能的空格
            while(*value_start == ' ' || *value_start == '\t' || *value_start == '\n' || *value_start == '\r')
            {
                value_start++;
            }
            
            char value_str[16] = {0};
            int i = 0;
            
            // 提取数字（0或1）
            while(*value_start >= '0' && *value_start <= '9')
            {
                if(i < 15)
                {
                    value_str[i++] = *value_start;
                    value_start++;
                }
                else
                {
                    break;
                }
            }
            value_str[i] = '\0';
            
            if(strlen(value_str) > 0)
            {
                // 转换为bool类型（0或1）
                int open_value = atoi(value_str);
                *open = open_value;
                *open_flag = 1;
                USART2_Print("Parsed OPEN: %d\r\n", open_value);
            }
            else
            {
                USART2_Print("Failed to extract valid OPEN value\r\n");
            }
        }
        else
        {
            USART2_Print("Value field not found in OPEN\r\n");
        }
    }
    
    strESP8266_Fram.InfBit.FramLenth=0;
}
