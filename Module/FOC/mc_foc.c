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


extern mc_angle_t mc_angle;

qd_t v_qd_set;

void foc_v_qd_set(qd_t set)
{
    v_qd_set.q = set.q;
    v_qd_set.d = set.d;
}

int16_t encoder_elec_test = 0;

/**
  * @brief 此函数每次定时器1中断执行一次，主要根据v_qd_set计算v_alpha_beta
  */
void foc_update(void)
{

    qd_t v_qd;
	alphabeta_t v_alpha_beta;
	
	bsp_enc_read_angle();

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
        // 当电机处于速度闭环模式，位置闭环模式，或者测试模式进入此分支

        if (mc_flag.ctrl_loop_mode == MC_TEST_MODE) { 
            /*
            当电机处于测试模式，给一个固定的力矩，设置v_qd_set，驱动电机快速转动
            */            

            qd_t volt;
            volt.q = 8000;
            volt.d = 0;
            foc_v_qd_set(volt);
        }
        // 当电机处于速度闭环模式或位置闭环模式，v_qd_set由pid算法设置(位于mc_ctrl.c文件中)。            

        // 对设置的v_qd进行限幅，得到可正常输出的v_qd    
        v_qd = circle_limitation(v_qd_set);

        // 反park变换，得到v_alpha_beta
        v_alpha_beta = mc_rev_park(v_qd, mc_angle.elec);
    }

    // 根据v_alpha_beta输出svpwm波，驱动电机运动
    svpwm_calc(v_alpha_beta);

}
