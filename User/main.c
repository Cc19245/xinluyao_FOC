/*
 * mian.
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "sys.h"
#include "delay.h"

#include "bsp_led.h"
#include "bsp_key.h"
#include "bsp_24cxx.h"
#include "bsp_encoder.h"
#include "bsp_mos.h"
#include "bsp_pwm.h"
#include "bsp_timer2.h"
#include "bsp_adc3.h"
#include "bsp_adc_current.h"

#include "mc_flag.h"
#include "mc_angle.h"
#include "lpf.h"

/**
  * @brief  main函数，C环境准备好后，最先执行的代码
  */
int main(void)
{	
	lpf_1rd_init(&lpf_speed_fb, 0.2, 0);
	lpf_3rd_init(&lpf_adc, 0.6, 0);
	
	delay_init();	//定时器初始化，初始化后可以使用delay_ms和delay_us进行延时
	bsp_led_init();	//led灯初始化
	bsp_key_init();	//led灯初始化
	bsp_mos_init(); //MP6536使能脚和刹车引脚初始化
	bsp_24cxx_init();
	
	mc_read_elec_offset();
	bsp_enc_init();
	mc_angle_init();	
	bsp_pwm_init();
	bsp_adc_current_init();
	bsp_timer2_init();
	
	bsp_adc3_init();
	
	// 开启驱动芯片输出，若无这行代码，这个芯片一直无输出，电机不会工作
	bsp_mos_on();
	
	mc_flag.ctrl_loop_mode = MC_OPEN_MODE;
	//mc_flag.ctrl_loop_mode = MC_SPD_CLOSE_LOOP_MODE;
	//mc_flag.ctrl_loop_mode = MC_POS_CLOSE_LOOP_MODE;
	//mc_flag.ctrl_loop_mode = MC_TEST_MODE;
	
	TIM1_OC4_TRIG_ADC_ENABLE();
	
	while(1)
	{
		// ADC采样进行速度控制
		uint16_t adcx_tmp = bsp_get_adc_filter(2);	// 10ms
		uint16_t adcx = lpf_3rd_calc(&lpf_adc, adcx_tmp);
		
		if (mc_flag.ctrl_loop_mode == MC_SPD_CLOSE_LOOP_MODE)
		{
			int16_t spd = adcx - 2048;
			int16_t speed = spd * 2200 / 2048;
			if (speed < -1500) {
				speed_ctrl = -1500;
			} else if (speed > 1500) {
				speed_ctrl = 1500;
			} else {
				speed_ctrl = speed;
			}
		} 
		else if (mc_flag.ctrl_loop_mode == MC_POS_CLOSE_LOOP_MODE)
		{
			int32_t pos = adcx * 32768 / 4095;
			if (pos < 0) {
				pos_ctrl = 0;
			} else if (pos > 32768) {
				pos_ctrl = 32768;
			} else {
				pos_ctrl = pos;
			}
		}
		
		// LED闪烁指示程序正在运行
		static uint32_t t = 0;
		t++;
		if (t > 50) {
			t = 0;
			LED1 = !LED1;
		} else {
			t++;
		}

	}
}


