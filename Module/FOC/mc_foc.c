/*
 * mc_foc
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "mc_foc.h"

#include "sys.h"

#include "bsp_encoder.h"

#include "mc_type.h"
#include "mc_const.h"
#include "mc_angle.h"
#include "mc_svpwm.h"
#include "mc_angle.h"
#include "mc_math.h"

#include "mc_flag.h"
#include "circle_limitation.h"
#include "mc_pid.h"

extern mc_angle_t mc_angle;
extern mc_pid_t iq_pid;
extern mc_pid_t id_pid;

ab_t i_ab_get;

qd_t i_qd_set;
qd_t v_qd_set;


void foc_v_qd_set(qd_t set)
{
    v_qd_set.q = set.q;
    v_qd_set.d = set.d;
}

void foc_i_qd_set(qd_t set)
{
    i_qd_set.q = set.q;
    i_qd_set.d = set.d;
}


// for debug
//qd_t v_qd;
//alphabeta_t i_alpha_beta;
//alphabeta_t v_alpha_beta;
//qd_t i_qd;

/**
  * @brief 此函数每次定时器1中断执行一次，主要根据v_qd_set计算v_alpha_beta
  */
void foc_update(void)
{
	qd_t v_qd;  // int16_t
	alphabeta_t i_alpha_beta;  // int16_t
	alphabeta_t v_alpha_beta;
	qd_t i_qd;
	
	static int16_t encoder_elec_test = 0;
	
	bsp_enc_read_angle();	// 先读取电角度
	


	if (mc_flag.ctrl_loop_mode == MC_OPEN_MODE) {
        /*
        当电机处于开环模式，直接设置vq=3000，vd=0。让电机缓慢转动，
        此模式可用于测量电机本体或者驱动芯片是否正常
        */

		encoder_elec_test += 10;
        qd_t volt;
        volt.q = 3000;
        volt.d = 0;

        v_qd = circle_limitation(volt);
        v_alpha_beta = mc_rev_park(v_qd, encoder_elec_test);

	} else if (mc_flag.ctrl_loop_mode == MC_ELEC_CALIB_MODE) { // 电角度校准
		/*
        当电机处于电角度校准模式，将电角度固定为-90度。电角度校准详情请参考文档
        */

        v_qd = circle_limitation(v_qd_set);
        v_alpha_beta = mc_rev_park(v_qd, -ANGLE_90);
		
    } else {
        // 当电机处于力矩模式，速度闭环模式，位置闭环模式，或者测试模式进入此分支
		
		if (mc_flag.ctrl_loop_mode == MC_TEST_MODE) { 
            /*
            当电机处于测试模式，给一个固定的力矩，设置v_qd_set，驱动电机快速转动
            */            			
			v_qd.q = 1000;
			v_qd.d = 0;
			
        } else {
			
			volatile int32_t w_err; 
			
			i_alpha_beta = mc_clark(i_ab_get);
			i_qd = mc_park(i_alpha_beta, mc_angle.elec);
			
			// 当电机处于速度闭环模式或位置闭环模式，i_qd_set由pid算法设置(位于mc_ctrl.c文件中)。            
			w_err = i_qd_set.q - i_qd.q;
			v_qd.q = mc_pi_controller(&iq_pid, w_err);
			
			w_err = i_qd_set.d - i_qd.d;
			v_qd.d = mc_pi_controller(&id_pid, w_err);
		}
       
        // 对设置的v_qd进行限幅，得到可正常输出的v_qd    
        v_qd = circle_limitation(v_qd);

        // 反park变换，得到v_alpha_beta
        v_alpha_beta = mc_rev_park(v_qd, mc_angle.elec);
    }

    // 根据v_alpha_beta输出svpwm波，驱动电机运动
    svpwm_calc(v_alpha_beta);
}
