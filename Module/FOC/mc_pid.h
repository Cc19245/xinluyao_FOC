#ifndef _MC_PID_H
#define _MC_PID_H

// C standard library
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

typedef struct {
	int16_t h_def_kp;
	int16_t h_def_ki;
	int16_t h_kp;
	int16_t h_ki;
	
	int32_t w_integral_term;
	
	// limit
	int32_t w_upper_integral_limit;
	int32_t w_lower_integral_limit;
	int16_t h_upper_output_limit;
	int16_t h_lower_output_limit;
	
	uint16_t h_kp_divisor_pow2;
	uint16_t h_ki_divisor_pow2;
	
} mc_pid_t;

extern void mc_pid_clear(mc_pid_t *pid);
int16_t mc_pi_controller(mc_pid_t *pid, int32_t w_process_var_error);

#endif

