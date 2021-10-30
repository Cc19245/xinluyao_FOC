/*
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "bsp_adc3.h"
#include "delay.h"

/**
  * @brief  速度和位置控制旋钮ADC初始化
  */
void bsp_adc3_init(void)
{
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF |RCC_APB2Periph_ADC3, ENABLE );	  //使能ADC3通道时钟
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	//PA1 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		
	GPIO_Init(GPIOF, &GPIO_InitStructure);	

	ADC_DeInit(ADC3);  //复位ADC3,将外设 ADC3 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC3和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC3, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

  
	ADC_Cmd(ADC3, ENABLE);	//使能指定的ADC3
	
	ADC_ResetCalibration(ADC3);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC3));	//等待复位校准结束
	
	ADC_StartCalibration(ADC3);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC3));	 //等待校准结束
 
//	ADC_SoftwareStartConvCmd(ADC3, ENABLE);		//使能指定的ADC3的软件转换启动功能
}

/**
  * @brief  速度和位置控制旋钮ADC初始化
  * @retval ADC采样得到的值
  */
uint16_t bsp_get_adc(void)
{
	  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC3, ADC_Channel_4, 1, ADC_SampleTime_239Cycles5 );	//ADC3,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC3, ENABLE);		//使能指定的ADC3的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC));//等待转换结束

	return ADC_GetConversionValue(ADC3);	//返回最近一次ADC3规则组的转换结果
}

/**
  * @brief  速度和位置控制旋钮ADC初始化
  * @param times: 采样的时间
  * @retval 滤波之后的ADC值
  */
uint16_t bsp_get_adc_filter(uint8_t times)
{
	uint32_t temp_val = 0;
	uint8_t t;
	for (t = 0; t < times; t++) 
	{
		temp_val += bsp_get_adc();
		delay_ms(5);
	}
	return (temp_val/times);
}

