#include "stm32f10x.h"                  // Device header
#include "FreeRTOS.h"
#include "task.h"
#include "Systick.h"
#include "system.h"
#include "semphr.h"
#include "queue.h"
#include <stdio.h>
#include "OLED.h"
#include "USART2.h"
#include "DHT11.h"
#include "ESP8266.h"
#include "Light.h"
#include "PWM.h"
#include "Key.h"
#include "Flame.h"
#include "Beep.h"
#include "RTC.h"
#include "event_groups.h"

extern void xPortSysTickHandler(void);

TaskHandle_t security_SYSTEM;
TaskHandle_t WiFiTask_Handler;
TaskHandle_t DHT11Task_Handler;
TaskHandle_t OLEDTask_Handler;
TaskHandle_t AUTOTask_Handler;
TaskHandle_t KeyTask_Handler;
TaskHandle_t DoorTask_Handler;
SemaphoreHandle_t USARTMutex = NULL;
SemaphoreHandle_t WiFiMutex = NULL;

void security_task(void *pvParameters);
void auto_task(void *pvParameters);
void WiFi_task(void *pvParameters);
void dht11_task(void *pvParameters);
void oled_task(void *pvParameters);
void key_task(void *pvParameters);
void door_task(void *pvParameters);
void vMonitorTask(void *pvParameters);

EventGroupHandle_t TimeInitEvent_Handle =NULL;
#define Time_EVENT (0x01 << 0)

uint8_t temp=0,tem=0,humi=0;
float Lux=0,Steer=0,PPM_Num=0;
int Speed=0,i=0,Security_Model=0,Door=0,TargetTemperature=25;
char Time[32];
char DataTime[9],GetDay[11];
char DataDay[20];
bool open=1,open_flag=1,open_F=0;

RTC_TimeTypeDef* DetaTime=NULL;

int main()
{
	SysTick_Init(72);
	USART2_Init();
	RTC_Init();
	USARTMutex = xSemaphoreCreateMutex();
	WiFiMutex = xSemaphoreCreateMutex();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	TimeInitEvent_Handle = xEventGroupCreate();
	xTaskCreate(vMonitorTask, "Monitor", 128, NULL, 1, NULL);
	xTaskCreate(security_task,"security_task",128,NULL,4,&security_SYSTEM);
	xTaskCreate(oled_task,"oled_task",256,NULL,3,&OLEDTask_Handler);	
	xTaskCreate(auto_task,"auto_task",128,NULL,3,&AUTOTask_Handler);
	xTaskCreate(key_task,"key_task",64,NULL,4,&KeyTask_Handler);	
	vTaskStartScheduler();   
}

void door_task(void *pvParameters)
{
	Infrared_Init();
	while(1)
	{
		Door=Infrared_Get();
		if(Door==0||Smoke_Flag==1||Flame_flag==1)
		{
			Steer2_Angle(90);
			while(Infrared_Get()==0||Smoke_Flag==1||Flame_flag==1)
				vTaskDelay(pdMS_TO_TICKS(100));
				vTaskDelay(pdMS_TO_TICKS(1000));
		if(Infrared_Get()==1||Smoke_Flag==0||Flame_flag==0)
			Steer2_Angle(0);
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
void security_task(void *pvParameters)
{
    FlameSensor_Init();
    while(1)
    {
			if (Flame_flag == 1 && Smoke_Flag == 1)
			{
				USART3_Send(0x11);
				Speed=100;
				Key_Model|=0x50;
				Key_Model&=0xDF;
				vTaskDelay(pdMS_TO_TICKS(5000));
			}
			else
			{
					if (Smoke_Flag == 1)
					{
							Speed=100;
							USART3_Send(0x12);
							Key_Model|=0x50;
							Key_Model&=0xDF;							
							vTaskDelay(pdMS_TO_TICKS(5000));
					}
					if (Flame_flag == 1)
					{
						if(xSemaphoreTake(WiFiMutex,portMAX_DELAY) == pdTRUE)
						{						
							ESP8266_Send_FlameMQTTPUB(Device_name,Flame_flag);
							xSemaphoreGive(WiFiMutex);
						}	
							Speed=100;
							USART3_Send(0x13);
							Key_Model|=0x50;
							Key_Model&=0xDF;
							vTaskDelay(pdMS_TO_TICKS(5000));
					}
			}
			vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void key_task(void *pvParameters)
{
	System_Key_Init();
	xTaskCreate(door_task,"door_task",64,NULL,4,&DoorTask_Handler);
	while(1)
	{
		Get_KeyNum();
		DHT11_Data(&temp,&tem,&humi);		
		vTaskDelay(pdMS_TO_TICKS(100));	
	}
}

void auto_task(void *pvParameters)
{	
	Light_Init();
	PWM_Init();
	RainSensor_Init();
	int time=0,flag=0,a=0,of=0;
	while(1)
	{
		PPM_Num=ConvertToPPM();
		if(PPM_Num<=100)
			Smoke_Flag=0;
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)==SET)
			Flame_flag=0;
		if((open!=open_flag) && open_F==1)
		{
			open_F=0;
			if(open_flag==0)
			{
					Key_Model|=0x80;
					Key_Model&=0xBF;
					of=1;				
			}
			else
			{
				Key_Model|=0x40;
				Key_Model&=0x7F;
			}
		}
		if(0x40==(Key_Model&0x40))
		{
			USART3_Send(0x66);
			flag=time=0;
			vTaskResume(DoorTask_Handler);
			vTaskResume(OLEDTask_Handler);
			open=1;
			Key_Model&=0x3F;
		}
		else if(0x80==(Key_Model&0x80))
		{
			time++;			
			if(time==1)
				USART3_Send(0x77);
			if(USART3_GetFlag()==1||of==1)
			{
				flag=1;
				of=0;
			}
			if(time>100)
			{
				Key_Model&=0x7F;
				time=0;
				USART3_Send(0x80);
			}
		}
		if(time<=100&&flag==1)
		{
			Key_Model&=0x7F;
			vTaskSuspend(DoorTask_Handler);
			vTaskSuspend(OLEDTask_Handler);
			OLED_Clear();
			OLED_Update();
			Motor_PWM(0);
			LED_PWM(0);
			Steer_Angle(0);
			open=0;
			vTaskDelay(pdMS_TO_TICKS(100));	
			continue;
		}
		//直流电机	
		if(0x20==(Key_Model&0x20))
		{
			Auto_Motor(temp,TargetTemperature);
		}
		else if(0x10==(Key_Model&0x10))
		{
			Key_Model&=0xCF;
			Speed+=25;				
			if(Speed==125||Smoke_Flag==1||Flame_flag==1)
				Speed=0;	
			Motor_PWM(Speed);			
		}
		//舵机
		if(0x08==(Key_Model&0x08))
		{					
			Key_Model&=0xF3;
			Steer=0;			
		}
		else if(0x04==(Key_Model&0x04))
		{
			Key_Model&=0xFB;
			Steer+=30;															
		}
		//LED
		if(0x02==(Key_Model&0x02))
		{
			Key_Model&=0xFE;
			Auto_Light();			
		}
		else if(0x01==(Key_Model&0x01))
		{
			if(i==0){
			Key_Model&=0xFE;
			LED_PWM(4095);					
			i=1;
			}
			else if(i==1)
			{
			Key_Model&=0xFE;
			LED_PWM(0);					
			i=0;
			}
		}
		
		if(Steer==180||Read_RainSensor()==0)
			Steer=0;
		Steer_Angle(Steer);
		vTaskDelay(pdMS_TO_TICKS(100));	
	}
}

void WiFi_task(void *pvParameters)
{
    int num = 0;
    ESP8266_USART_Init();    
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP8266_SendString("AT+RST\r\n");
    vTaskDelay(pdMS_TO_TICKS(5000));   
    ESP8266_MODE(1);
		ESP8266_CWJAP(WiFiname,Wpassword);
    // 连接成功后立即同步时间
    if(xSemaphoreTake(USARTMutex, portMAX_DELAY) == pdTRUE)
    {
        char time_buf[32];
        
        if(ESP8266_GetSimpleNTPTime(time_buf)) {
            strcpy(Time, time_buf);
						RTC_SetTimeFromString(time_buf);
            USART2_Print("Time sync success: %s\r\n", Time);
        } else {
            strcpy(Time, "2025-10-01 00:00:00");
            USART2_Print("Time sync failed, using default time\r\n");
        }        
        xSemaphoreGive(USARTMutex);
    }
		xEventGroupSetBits(TimeInitEvent_Handle,Time_EVENT);
    ESP8266_Init();
    while(1)
    {    
				if(num == 0)
					if(xSemaphoreTake(WiFiMutex, portMAX_DELAY) == pdTRUE)
					{						
						ESP8266_Send_MQTTPUB(Device_name, temp, tem, humi, Flame_flag, (int)PPM_Num,TargetTemperature,open); 
						xSemaphoreGive(WiFiMutex);
					}						
					if(xSemaphoreTake(USARTMutex, portMAX_DELAY) == pdTRUE)
						{						
							ESP8266_Getdata(&TargetTemperature,&open_flag,&open_F); 
							xSemaphoreGive(USARTMutex);
						} 			    							
        num++;
        num %= 100;
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void oled_task(void *pvParameters)
{
	EventBits_t Wait_event=Time_EVENT;
	OLED_Init();
	xTaskCreate(WiFi_task,"WiFi_task",256,NULL,2,&WiFiTask_Handler);
	int num=0,pug=0;
	OLED_ShowImage(0,0,128,64,Bmp_Logo);
	OLED_Update();	
	while(Wait_event!=xEventGroupWaitBits(TimeInitEvent_Handle,Time_EVENT,pdTRUE,pdFALSE,portMAX_DELAY))
		vTaskDelay(pdMS_TO_TICKS(1000));
	for(int a=0;a<=64;a+=2)
	{
		OLED_ShowImage(0,a,128,64,Bmp_Logo);
		OLED_Printf(1,(-31+a),OLED_8X16,"温度:%2d.%d°",temp,tem);
		OLED_ShowChar(80,(-31+a),'C',OLED_8X16);
		OLED_Printf(1,(-47+a),OLED_8X16,"湿度:%d%%",humi);
		OLED_Printf(num,(-15+a),OLED_8X16,"请不要在图书馆内吸烟!请不要在图书馆内吸烟!");
		OLED_Update();
		vTaskDelay(pdMS_TO_TICKS(50));
		OLED_Clear();
	}
	
	while(1)
	{			
		
		if(Smoke_Flag==1&&Flame_flag==1)
		{
			OLED_Clear();
			OLED_Printf(num,1,OLED_8X16,"发生火灾请立即有序撤离!发生火灾请立即有序撤离!");	
			OLED_Update();			
		}
		else if(Smoke_Flag==1)
		{
			OLED_Clear();
			OLED_Printf(1,17,OLED_8X16,"检测到烟雾!");
			OLED_Update();
		}
		else if(Flame_flag==1)
		{
			OLED_Clear();
			OLED_Printf(1,33,OLED_8X16,"检测到明火!");
			OLED_Update();
		}		
		else
		{
			if(pug>=125)
			{
				OLED_Clear();
				RTC_GetDayString(GetDay,11);
				OLED_Printf(24,1,OLED_8X16,"%s",GetDay);
			}else
			{
				OLED_Clear();
				RTC_GetTimeString(DataTime,9);			
				OLED_Printf(32,1,OLED_8X16,"%s",DataTime);
			}
			
			OLED_Printf(1,33,OLED_8X16,"温度:%2d.%d°",temp,tem);
			OLED_ShowChar(80,33,'C',OLED_8X16);
			OLED_Printf(1,17,OLED_8X16,"湿度:%d%%",humi);
			OLED_Printf(num,49,OLED_8X16,"请不要在图书馆内吸烟!请不要在图书馆内吸烟!");
			OLED_Update();			
		}
		num-=4;
		pug++;
		num%=160;
		pug%=250;
		vTaskDelay(pdMS_TO_TICKS(100)); 
	}
}

void vMonitorTask(void *pvParameters) 
{
	char *pcWriteBuffer = pvPortMalloc(1024); // 分配足够大的缓冲区
	while(1) {
		vTaskList(pcWriteBuffer); // 获取任务列表信息
		if(xSemaphoreTake(USARTMutex, portMAX_DELAY)==pdTRUE)
		{
			USART2_SendString(pcWriteBuffer);
			USART2_SendString("\r\n"); 
			xSemaphoreGive(USARTMutex);
		}
			vTaskDelay(pdMS_TO_TICKS(5000)); // 每5秒打印一次
	}
}

void SysTick_Handler(void)
{
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
				extern void xPortSysTickHandler(void);
        xPortSysTickHandler();
    }
    
    // 维护RTC时间
    static uint32_t tick_count = 0;
    tick_count++;
    if(tick_count >= 1000) { // 每秒更新一次
        tick_count = 0;
        RTC_IncrementSecond();
    }
}
