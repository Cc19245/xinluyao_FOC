/*
 * led driver.
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 * 技术讨论QQ群: 680440867
 */
 
#include "bsp_key.h"
#include "delay.h"

void bsp_key_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); //开启按键端口PA的时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //端口配置为上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//初始化端口
}


uint8_t bsp_key_value()
{
	uint8_t key_val = KEY_RELEASE;
	if (GPIO_ReadInputDataBit(GPIOA , GPIO_Pin_0) == KEY_PRESS) {
		delay_ms(10);
		if (GPIO_ReadInputDataBit(GPIOA , GPIO_Pin_0) == KEY_PRESS) {
			key_val = KEY_PRESS;
		}
	}
	
	return key_val;	
}

