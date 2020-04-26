/*
 * bsp_timer2.c.
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */
 
#include "bsp_timer2.h"
#include "mc_ctrl.h"

void bsp_timer2_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 720;   

	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	TIM_PrescalerConfig(TIM2, 99, TIM_PSCReloadMode_Immediate); //设置预分频 100分频
	
	/* 允许TIM2更新事件中断 */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	/* TIM2 enable counter */
    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	
    do {		
		mc_ctrl();
    } while (0);
    
}

