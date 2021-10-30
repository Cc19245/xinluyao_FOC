/*
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "bsp_adc_current.h"
#include "delay.h"

#include "mc_type.h"
#include "mc_foc.h"

#include "bsp_led.h"
#include "mc_const.h"
/**
  * @brief  速度和位置控制旋钮ADC初始化  ？？ 这里应该是配置电流采样的ADC
  * M_CURRENT_A: PA4, M_CURRENT_B: PA1, M_CURRENT_C: PB1
  */
void bsp_adc_current_init(void)
{
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF |RCC_APB2Periph_ADC3, ENABLE );	  //使能ADC3通道时钟
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	/*
	 * IA: PA4(ADC12_IN4)
	 * IB: PA1(ADC12_IN1)
	 * IC: PB1(ADC12_IN9)
	 */
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	ADC_Cmd(ADC1, DISABLE);
	ADC_Cmd(ADC2, DISABLE);

	ADC_DeInit(ADC1);
	ADC_DeInit(ADC2);
	
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Mode = ADC_Mode_InjecSimult;  // ADC工作模式，这里不是独立模式，具体还要看数据手册
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;  // 扫描模式，每次转换所有通道都进行转换
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  // 不停转换的功能，关闭
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  // 不用外部触发，软件开启转换
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;   // ADC1的转换通道个数为1
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Mode = ADC_Mode_InjecSimult;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC2, &ADC_InitStructure);
	
	ADC_Cmd(ADC1, ENABLE);
	ADC_Cmd(ADC2, ENABLE);
	
	ADC_ResetCalibration(ADC1); 				// 初始化 ADC 校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1)); // 等待校准寄存器初始化完成
	ADC_StartCalibration(ADC1); 				// ADC 开始校准
	while(ADC_GetCalibrationStatus(ADC1));		// 等待校准完成
	
	ADC_ResetCalibration(ADC2);
	while(ADC_GetResetCalibrationStatus(ADC2));
	ADC_StartCalibration(ADC2);
	while(ADC_GetCalibrationStatus(ADC2));
	
	delay_ms(100);
	
	bsp_injected_adc_offset_config();  // 没有启动ADC的时候进行几个数值的采样，消除硬件偏差
	
	bsp_injected_adc_conv_config();
	
	NVIC_InitStruct.NVIC_IRQChannel = ADC1_2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct );
	
	
	ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);
}

/**
  * @brief 不启动电机的时候测量ADC采样值，这个值作为0点时候值，消除硬件带来的偏差  
  */
void bsp_injected_adc_offset_config(void)
{
	uint32_t i;
	uint32_t offset[3];
	
	offset[0] = 0;
	offset[1] = 0;
	offset[2] = 0;
	
	ADC_ITConfig(ADC1, ADC_IT_JEOC, DISABLE);
	ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_None);
	
	ADC_InjectedSequencerLengthConfig(ADC1,3);
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_13Cycles5);
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_13Cycles5);
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_9, 3, ADC_SampleTime_13Cycles5);
	
	ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);
	
	for(i=0; i<64; i++)
	{
		ADC_SoftwareStartInjectedConvCmd(ADC1,ENABLE);
		while(ADC_GetFlagStatus(ADC1,ADC_FLAG_JEOC) == RESET);
		ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);
		offset[0] += ADC1->JDR1;
		offset[1] += ADC1->JDR2;
		offset[2] += ADC1->JDR3;
		delay_ms(1);
	}
	
	offset[0] /= 64;
	offset[1] /= 64;
	offset[2] /= 64;
	
	ADC1->JOFR1 = offset[0];
	ADC2->JOFR1 = offset[1];
}

/**
* @brief ADC配置为inject模式
  */
void bsp_injected_adc_conv_config(void)
{
	// Pharse A
	ADC_InjectedSequencerLengthConfig(ADC1, 1);
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_7Cycles5);
	ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T1_TRGO);
	ADC_ExternalTrigInjectedConvCmd(ADC1,ENABLE);

	// Pharse B
	ADC_InjectedSequencerLengthConfig(ADC2, 1);
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
	ADC_ExternalTrigInjectedConvCmd(ADC2,ENABLE);
}

extern ab_t i_ab_get;
void ADC1_2_IRQHandler(void)
{
	ab_t iab;
	
	if(ADC1->SR & (ADC_IT_JEOC >> 8)) {
		
		ADC1->SR = ~(ADC_IT_JEOC >> 8);
		
		iab.a = -ADC1->JDR1;
		iab.b = -ADC2->JDR1;
		
		i_ab_get = iab;
		
		foc_update();
    }
}
