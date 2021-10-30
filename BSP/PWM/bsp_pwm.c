/*
 * led driver.
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */
 
#include "bsp_pwm.h"
#include "mc_const.h"
#include "mc_foc.h"

void bsp_pwm_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM1_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	/* 配置定时器1使用的引脚 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Lock all the pin config */
	GPIO_PinLockConfig(GPIOA, GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10);
	
	TIM_DeInit(TIM1);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;
	TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD_CYCLES / 2;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//输入捕获用到，此时这个参数无影响
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 1;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	
	TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_OC4Ref);
	
	/* OC1 OC2 OC3 config*/
	TIM_OCStructInit(&TIM1_OCInitStructure);
	TIM1_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM1_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM1_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM1_OCInitStructure.TIM_Pulse = 0;
	TIM1_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM1_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;

	TIM_OC1Init(TIM1, &TIM1_OCInitStructure);
	TIM_OC2Init(TIM1, &TIM1_OCInitStructure);
	TIM_OC3Init(TIM1, &TIM1_OCInitStructure);
	
	/* 第4通道，作为采样的触发通道 */
	TIM_OCStructInit(&TIM1_OCInitStructure);
	TIM1_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM1_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM1_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM1_OCInitStructure.TIM_Pulse = 0;
	TIM1_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM1_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM1_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM1_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;	

	TIM_OC4Init(TIM1, &TIM1_OCInitStructure);

	TIM_Cmd(TIM1, ENABLE);               
	TIM_GenerateEvent(TIM1, TIM_EventSource_Update);

	TIM1->CCR1 = 0;
    TIM1->CCR2 = 0;
    TIM1->CCR3 = 0;
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}
