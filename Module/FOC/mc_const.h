/*
 * mc_const.h
 *
 * Copyright (C) 2018
 * Author:
 *
 * This file is Confidentiality.
 */

#ifndef __MC_CONST_H
#define __MC_CONST_H

#define MC_POLE_PAIRS_NUM 7

#define UINT15_MAX 32768

#define ANGLE_0             ((int16_t)(0))
#define ANGLE_90            ((int16_t)(65536 * 90 / 360))
#define ANGLE_180           ((int16_t)(65536 * 180 / 360))


#define PWM_FREQ	(20000) 	// 20 k

#define PWM_PERIOD_CYCLES (uint16_t)(72*\
                                      (unsigned long long)1000000u/((uint16_t)(PWM_FREQ)))
									  
#define SQRT3FACTOR (uint16_t) 0xDDB4 /* = (16384 * 1.732051 * 2)*/

#define hT_Sqrt3 ((3600*SQRT3FACTOR)/16384u)


#endif


