#include "Light.h"

uint16_t data[2];
int Smoke_Flag=0;

void ADC_Dog(uint16_t Dog_data)
{
	ADC_AnalogWatchdogSingleChannelConfig(ADCx,ADC_Channel_1);
	ADC_AnalogWatchdogThresholdsConfig(ADCx,Dog_data,0x0000);
	
	ADC_AnalogWatchdogCmd(ADCx,ADC_AnalogWatchdog_SingleRegEnable);
	ADC_ITConfig(ADCx,ADC_IT_AWD,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=ADC1_2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=6;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
	NVIC_Init(&NVIC_InitStruct);
	
}	


void Light_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin=Light_GPIO | Smoke_GPIO;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(ADC_GPIO,&GPIO_InitStruct);

	ADC_RegularChannelConfig(ADCx,ADC_Channel_0,1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADCx,ADC_Channel_1,2, ADC_SampleTime_239Cycles5);
	
	ADC_InitTypeDef ADC_InitStruct;
	ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;
	ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_ContinuousConvMode=ENABLE;
	ADC_InitStruct.ADC_ScanConvMode=ENABLE;
	ADC_InitStruct.ADC_NbrOfChannel=2;
	ADC_Init(ADCx,&ADC_InitStruct);
	
	ADC_Cmd(ADCx,ENABLE);
	
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_BufferSize=2;
	DMA_InitStruct.DMA_DIR=DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_M2M=DMA_M2M_Disable;
	DMA_InitStruct.DMA_MemoryBaseAddr=(uint32_t)data;
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_Mode=DMA_Mode_Circular;
	DMA_InitStruct.DMA_PeripheralBaseAddr=(uint32_t)&ADCx->DR;
	DMA_InitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_Priority=DMA_Priority_Medium;
	DMA_Init(DMA1_Channel1,&DMA_InitStruct);
	ADC_DMACmd(ADCx,ENABLE);
	DMA_Cmd(DMA1_Channel1,ENABLE);
	
	ADC_ResetCalibration(ADCx);
	while(ADC_GetResetCalibrationStatus(ADCx)==SET);
	ADC_StartCalibration(ADCx);
	while(ADC_GetCalibrationStatus(ADCx)==SET);
	ADC_SoftwareStartConvCmd(ADCx,ENABLE);
	
	ADC_Dog(1000);
}

float ConvertToLux(void)
{
    const float Vref = 3.3f;
    float voltage = (data[0] * Vref) / 4095.0f;
    float lux = (3.3f - voltage) * 2000.0f / 3.3f;
    return lux > 0 ? lux : 0;
}

float ConvertToPPM(void)
{
    // 假设使用MQ-2烟雾传感器，典型转换公式：
    const float Vref = 3.3f;
    float voltage = (data[1] * Vref) / 4095.0f;
    
    // 电压转PPM公式（需根据校准曲线调整）
    // MQ-2近似公式：ppm = 1000 * (voltage / 3.3)^3
    float ratio = voltage / 3.3f;
    return 1000.0f * ratio * ratio * ratio;
}


void Auto_Light(void)
{
	LED_PWM(data[0]>=300?(data[0]/100)*100:0);
}

void ADC1_2_IRQHandler(void)
{
	if(ADC_GetITStatus(ADCx,ADC_IT_AWD)==1)
	{
		Smoke_Flag=1;
	}
	ADC_ClearITPendingBit(ADCx,ADC_IT_AWD);
}	
