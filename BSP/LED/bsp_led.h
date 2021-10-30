/*
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 * 技术讨论QQ群: 680440867
 */

#ifndef __BSP_LED_H
#define __BSP_LED_H	
 
#include "sys.h"


#define LED1 PEout(4)

void bsp_led_init(void);//初始化
void bsp_led_on(void);
void bsp_led_off(void);
		 				    
#endif
