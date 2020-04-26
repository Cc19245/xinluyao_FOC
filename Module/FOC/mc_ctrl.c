/*
 * mc_ctrl.c
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "mc_ctrl.h"

#include "sys.h"

#include "bsp_encoder.h"
#include "mc_angle.h"

#include "mc_foc.h"

#include "mc_const.h"
#include "mc_svpwm.h"
#include "mc_angle.h"

#include "mc_flag.h"
#include "mc_pid.h"
#include "circle_limitation.h"

#include "lpf.h"

static void mc_1ms_loop(void);
static void mc_10ms_loop(void);

mc_pid_t speed_pid = {
	.h_kp = 40,
	.h_ki = 2,
	
	.w_integral_term = 0,
	
	.h_kp_divisor_pow2 = 0,
	.h_ki_divisor_pow2 = 0, 
	
	.w_upper_integral_limit = 32767,
	.w_lower_integral_limit = -32768,
	
	.h_upper_output_limit = 32767,
	.h_lower_output_limit = -32768,
};

mc_pid_t pos_pid = {
	.h_kp = 5,
	.h_ki = 0,
	
	.w_integral_term = 0,
	
	.h_kp_divisor_pow2 = 8,
	.h_ki_divisor_pow2 = 0, 
	
	.w_upper_integral_limit = 1500,
	.w_lower_integral_limit = -1500,
	
	.h_upper_output_limit = 1500,
	.h_lower_output_limit = -1500,
};


void mc_ctrl(void)
{
	static int32_t i = 0;
	
	i++;
	
	// 1ms
	mc_1ms_loop();
	
	// 10ms
	if (i % 10 == 0) {
		mc_10ms_loop();
	}
}

/**
  * @brief 此函数每1ms执行一次
  * @note  函数里主要进行电机速度测量以及速度环pid运算
  		   当电机在速度闭环模式运行，程序仅运行速度环。根据speed_ctrl的值控制电机转速。
  		   当电机在位置闭环模式运行，程序运行速度环和位置环，速度环为内环，位置环为外环。
  */
float mc_speed_tmp = 0;
static void mc_1ms_loop(void)
{
	// 将速度单位转化为rpm
	int32_t pos_dec = mc_pos - mc_pos_last;
	if (pos_dec < 3000)
		mc_speed_tmp = (mc_pos - mc_pos_last) * 60000 / UINT15_MAX; 
	else 
		mc_speed_tmp = 0;
	
	mc_speed = lpf_1rd_calc(&lpf_speed_fb, mc_speed_tmp);
	mc_pos_last = mc_pos;
	
	if (mc_flag.ctrl_loop_mode == MC_SPD_CLOSE_LOOP_MODE || mc_flag.ctrl_loop_mode == MC_POS_CLOSE_LOOP_MODE) {
		volatile int32_t w_err; 
		w_err = speed_ctrl - mc_speed;
		
		qd_t v_qd;
		v_qd.q = mc_pi_controller(&speed_pid, w_err);
		v_qd.d = 0; 
		
		foc_v_qd_set(v_qd);
	}
}


/**
  * @brief 此函数每10ms执行一次
  * @note  当电机处于电角度校准模式，进行电角度校准。
  		   当电机处于位置闭环模式，运行位置环pid。
  */
static void mc_10ms_loop(void)
{
	static int32_t cnt = 0;
	static int16_t vq = 0;
	qd_t volt_set;
	if (mc_flag.ctrl_loop_mode == MC_ELEC_CALIB_MODE) {

		volt_set.q = vq;
		volt_set.d = 0;
		foc_v_qd_set(volt_set);

		cnt += 1;
		if (cnt < 180)
			vq += 100;

		if (cnt > 300) {
			vq = 0;
			cnt = 0;
			
			mc_set_elec_offset();
			
			volt_set.q = vq;
			volt_set.d = 0;
			foc_v_qd_set(volt_set);
			
			mc_save_elec_offset();
			
			mc_flag.ctrl_loop_mode = MC_POS_CLOSE_LOOP_MODE;
		}
	} else if (mc_flag.ctrl_loop_mode == MC_POS_CLOSE_LOOP_MODE) { // pos loop
		 int32_t w_err = pos_ctrl - mc_pos;
		 
		speed_ctrl = mc_pi_controller(&pos_pid, w_err);
	}
}
