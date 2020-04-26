/*
 * delay.
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */
 
#include "delay.h"
static const uint8_t SYS_CLK = 72;

static uint8_t  fac_us=0;	//us延时倍乘数			   
	
/**
 * 初始化延时函数.
 * SYSTICK的时钟固定为HCLK时钟的1/8
 */
void delay_init()
{
	uint32_t reload;
	
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);	//选择外部时钟
	fac_us = SYS_CLK;
	
	reload = SYS_CLK * 1000; //1000 =  1000000 / 1000
	
	SysTick->LOAD = reload;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}								    

/**
 * 延时n us.
 *
* @param nus 需要延时的微秒数.
 */    								   
void delay_us(uint32_t nus)
{
    volatile uint32_t ticks;
    volatile uint32_t told, tnow, tcnt = 0;
    
	uint32_t reload = SysTick->LOAD;
    ticks = nus * fac_us;
    told = SysTick->VAL;

    while (1) {
        tnow = SysTick->VAL;

        if (tnow != told) {
            if (tnow < told)
				tcnt += told - tnow;
            else 
				tcnt += reload - tnow + told;

            told = tnow;

            if (tcnt >= ticks)
				break;
        }
    }
}

/**
 * 延时n ms.
 */  
void delay_ms(uint16_t nms)
{
    uint32_t i;

    for (i = 0; i < nms; i++) 
		delay_us(1000);
}




































