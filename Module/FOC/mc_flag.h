/*
 * mc_flag.h
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#ifndef __MC_FLAG_H
#define __MC_FLAG_H

#include "sys.h"
#include "lpf.h"

typedef enum {
    MC_NO_CTRL_MODE        = 0x00,
	MC_OPEN_MODE		   = 0x01,
    MC_ELEC_CALIB_MODE     = 0x02,
    MC_TEST_MODE           = 0x03,
	MC_SPD_CLOSE_LOOP_MODE = 0x04,
    MC_POS_CLOSE_LOOP_MODE = 0x05,
} mc_ctrl_loop_mode_e;

typedef struct {
    mc_ctrl_loop_mode_e ctrl_loop_mode;
} mc_flag_t;

extern mc_flag_t mc_flag;

extern lpf_1rd_t lpf_speed_fb;
extern lpf_3rd_t lpf_adc;

extern int32_t mc_speed;		// 电机速度
extern int32_t mc_pos;			// 电机位置
extern int32_t mc_pos_last;	// 上次计算电机速度时的位置，两次位置差来计算速度

extern int32_t speed_ctrl;	// 速度环的目标值
extern int32_t pos_ctrl;	// 位置环的目标值


#endif

