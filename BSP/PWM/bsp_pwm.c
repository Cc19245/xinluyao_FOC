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

#include "bsp_led.h"
void bsp_pwm_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM1_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   // 是因为有外设复用吗？
	
	/* 配置定时器1使用的引脚 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   // 复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Lock all the pin config */   // 锁定IO引脚，防止功能被更改
	GPIO_PinLockConfig(GPIOA, GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10);
	
	TIM_DeInit(TIM1);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;  // 计数器始终分频，这里为0则不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;  // 计数模式为中心对齐模式1,计数先增后减，只在递减计数溢出时产生中断？
	TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD_CYCLES / 2;  // 重装载寄存器的值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  // 只在输入捕获的时候生效
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 1;  // 重计数值
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	
	/* OC1 OC2 OC3 config*/
	TIM_OCStructInit(&TIM1_OCInitStructure);
	TIM1_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;   // 模式1，即cnt<ccr时，输出高电平
	TIM1_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  // 开启主输出
    TIM1_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;  // 关闭互补输出
	TIM1_OCInitStructure.TIM_Pulse = 0;  // 占空比为0
	TIM1_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  // 输出通道电平极性配置为高电平
	TIM1_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;  // 输出通道空闲电平，也就是没有PWM输出的时候默认的电平

	TIM_OC1Init(TIM1, &TIM1_OCInitStructure);
	TIM_OC2Init(TIM1, &TIM1_OCInitStructure);
	TIM_OC3Init(TIM1, &TIM1_OCInitStructure);
	
	TIM1->CCR1 = 0;
    TIM1->CCR2 = 0;
    TIM1->CCR3 = 0;

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  // 预转载使能，下一个周期才更改CCR和ARR的值，也就是实现同步更改这两个值，防止出错
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
	NVIC_InitStruct.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct );

	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);    // 使能定时器中断
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE); // 主输出使能，一般定时器用不到，高级定时器才用到
	
	TIM_Cmd(TIM1, ENABLE);               
	TIM_GenerateEvent(TIM1, TIM_EventSource_Update);  // 在这里，人为的产生了一个Update事件，也就是产生了一个触发输出事件
}

void TIM1_UP_IRQHandler(void)
{
	if ((TIM1->SR & TIM_IT_Update) != RESET) {
        TIM1->SR = ~TIM_IT_Update;
		
        // 每次定时器中断执行一次FOC算法
        foc_update();
    }
}


