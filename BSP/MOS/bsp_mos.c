/*
 * led driver.
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 * 技术讨论QQ群: 680440867 
 */
 
#include "bsp_mos.h"

 
/**
  * @brief  MP6536使能引脚和错误信息引脚初始化
  */
void bsp_mos_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	//MP6536使能引脚，当输出高时，MP6536芯片使能，输出低时，MP6536芯片关闭。
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	//MP6536使能引脚输出低电平，芯片为关闭状态。
	GPIO_ResetBits(GPIOG, GPIO_Pin_9);

	/* 
		MP6536错误信息引脚, 默认为高。芯片出错变为低，并自动关闭芯片输出
		当用户控制这个引脚输出为低时，芯片U、V、W输出引脚为高阻态。
	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	PWR_BackupAccessCmd( ENABLE );/* 允许修改RTC和后备寄存器*/
	BKP_TamperPinCmd(DISABLE);  /* 关闭入侵检测功能,PC13可以用作普通IO*/
	PWR_BackupAccessCmd(DISABLE);/* 禁止修改RTC和后备寄存器*/

	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
  * @brief  MP6536芯片使能
  */
void bsp_mos_on(void)
{
	GPIO_SetBits(GPIOG, GPIO_Pin_9);
}

/**
  * @brief  MP6536芯片关闭
  */
void bsp_mos_off(void)
{
	GPIO_ResetBits(GPIOG, GPIO_Pin_9);
}
