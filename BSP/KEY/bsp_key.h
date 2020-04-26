/*
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 * 技术讨论QQ群: 680440867
 */

#ifndef __BSP_KEY_H
#define __BSP_KEY_H	 

#include "sys.h"


#define KEY_PRESS	0	//按键按下
#define KEY_RELEASE	1

void bsp_key_init(void);
uint8_t bsp_key_value(void);

		 				    
#endif
