/*
 * mc_pid.c
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */
#include "mc_pid.h"

#include "sys.h"

int16_t mc_pi_controller(mc_pid_t *pid, int32_t w_process_var_error)
{
	int32_t w_proportional_term, w_integral_term, w_output_32, w_integral_sum_tmp;
	int32_t w_discharge = 0;
	int16_t h_upper_output_limit = pid->h_upper_output_limit;
	int16_t h_lower_output_limit = pid->h_lower_output_limit;
	
	/* 1 - 比例项计算 */
	w_proportional_term = pid->h_kp * w_process_var_error;
	
	/* 2 - 积分项计算 */
	if (pid->h_ki == 0) {
		pid->w_integral_term = 0;
	} else {
		w_integral_term = pid->h_ki * w_process_var_error;
		w_integral_sum_tmp = pid->w_integral_term + w_integral_term;
		
		// 处理w_integral_sum_tmp溢出的情况
		if (w_integral_sum_tmp < 0) {
			if (pid->w_integral_term > 0) {
				if (w_integral_term > 0) {
					/* 正常情况下 pid->w_integral_term > 0 , w_integral_term > 0，其求和结果w_integral_sum_tmp必 > 0；
					w_integral_sum_tmp < 0，只可能是求和的值超过了INT32范围而溢出，所以这里将其赋值为INT32_MAX来纠正这个错误 */
					w_integral_term = INT32_MAX;	
				}
			}
		} else {
			if (pid->w_integral_term < 0) {
				if (w_integral_term < 0) {
					/* 正常情况下 pid->w_integral_term < 0 , w_integral_term < 0，其求和结果w_integral_sum_tmp必 < 0；
					w_integral_sum_tmp >= 0，只可能是求和的值超过了INT32范围而溢出，所以这里将其赋值为-INT32_MAX来纠正这个错误 */
					w_integral_term = -INT32_MAX;	
				}
			}
		}
		
		// 限制积分项范围
		if (w_integral_sum_tmp > pid->w_upper_integral_limit) {
			pid->w_integral_term = pid->w_upper_integral_limit;
		} else if (w_integral_sum_tmp < pid->w_lower_integral_limit) {
			pid->w_integral_term = pid->w_lower_integral_limit;
		} else {
			
			pid->w_integral_term = w_integral_sum_tmp;
		}
	}
	
	/* 3 - 计算pid运算的输出 */
	/* 注意：下面这行代码不满足 MISRA规范；用户需要验证编译器是使用Cortex-M3 ASR(算术右移指令)而不是
	LSR(逻辑右移指令)来处理右移操作，否则结果会得到错误的结果。 
	笔者验证过，绝大多数ARM系列的编译器(gcc ARMCC ARMClang)，对于这行代码都会使用ASR，如不需要进行MISRA认证，这里可以放心使用。 */
	w_output_32 = (w_proportional_term >> pid->h_kp_divisor_pow2) + (pid->w_integral_term >> pid->h_ki_divisor_pow2);
	
	
	// 限制pid输出的范围
	if (w_output_32 > h_upper_output_limit) {
		w_discharge = h_upper_output_limit - w_output_32;
		w_output_32 = h_upper_output_limit;
	} else if (w_output_32 < h_lower_output_limit) {
		w_discharge = h_lower_output_limit - w_output_32;
		w_output_32 = h_lower_output_limit;
	} else {
		/* Nothing to do here*/
	}
	
	pid->w_integral_term += w_discharge;
	
	return ((int16_t) (w_output_32));
	
}


/*
LSL 逻辑左移：
ASL 算术左移：
	逻辑左移与算术左移的操作是一样的，都是将操作数向左移位，低位补零，移除的高位进行丢弃。

LSR 逻辑右移：
	逻辑右移与逻辑左移是相对的，逻辑右移其实就是往右移位，左边补零。

ASR 算术右移：
	将各位依次右移指定位数，然后在左侧用原符号位补齐。
	以用算术右移来进行有符号数据的除法。把一个数右移n位，相当于该数除以2的n次方。
*/

