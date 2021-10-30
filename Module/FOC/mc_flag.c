/*
 * mc_flag.h
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "mc_flag.h"

#include "sys.h"

mc_flag_t mc_flag = {
.ctrl_loop_mode = MC_OPEN_MODE,
};

// 速度滤波
lpf_1rd_t lpf_speed_fb;
lpf_3rd_t lpf_adc;

int32_t mc_speed;		// 电机速度
int32_t mc_pos;			// 电机位置
int32_t mc_pos_last;	// 上次计算电机速度时的位置，两次位置差来计算速度

int32_t speed_ctrl;	// 速度环的目标值
int32_t pos_ctrl;	// 位置环的目标值



