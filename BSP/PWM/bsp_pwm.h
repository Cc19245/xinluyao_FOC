/*
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#ifndef __BSP_PWM_H
#define __BSP_PWM_H	 

#include "sys.h"
#include "mc_const.h"

#define TIM1_OC4_TRIG_ADC_ENABLE()	TIM1->CCR4 = SAMPLE_POINT
#define TIM1_OC4_TRIG_ADC_DISABLE()	TIM1->CCR4 = 0

void bsp_pwm_init(void);
		 				    
#endif
