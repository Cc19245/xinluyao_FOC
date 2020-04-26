/*
 * mc_svpwm.c
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "sys.h"

#include "mc_const.h"
#include "mc_svpwm.h"

#define SECTOR_1  0u
#define SECTOR_2  1u
#define SECTOR_3  2u
#define SECTOR_4  3u
#define SECTOR_5  4u
#define SECTOR_6  5u

/**
  * @brief 根据v_alpha_beta计算svpwm
  */
void svpwm_calc(alphabeta_t v_alpha_beta)
{
	int32_t w_x, w_y, w_z, w_u_alpha, w_u_beta, w_time_ph_a, w_time_ph_b, w_time_ph_c;

    uint8_t sector;

	w_u_alpha = v_alpha_beta.alpha * (int32_t)hT_Sqrt3;
	w_u_beta = -(v_alpha_beta.beta * ( int32_t )(PWM_PERIOD_CYCLES) ) * 2;
	
	w_x = w_u_beta;
	w_y = ( w_u_beta + w_u_alpha ) / 2;
	w_z = ( w_u_beta - w_u_alpha ) / 2;
	
	/* Sector calculation from w_x, w_y, w_z */
	if (w_y < 0) {
		if (w_z < 0) {
			sector = SECTOR_5;
			w_time_ph_a =  ( int32_t )( PWM_PERIOD_CYCLES ) / 4 + ( ( w_y - w_z ) / ( int32_t )262144 );
			w_time_ph_b = w_time_ph_a + w_z / 131072;
			w_time_ph_c = w_time_ph_a - w_y / 131072;
			//pSetADCSamplingPoint = pHandle->pFctSetADCSampPointSect5;
		} else if (w_x <= 0) { /* w_z >= 0 */
			sector = SECTOR_4;
			w_time_ph_a = ( int32_t )( PWM_PERIOD_CYCLES ) / 4 + ( ( w_x - w_z ) / ( int32_t )262144 );
			w_time_ph_b = w_time_ph_a + w_z / 131072;
			w_time_ph_c = w_time_ph_b - w_x / 131072;
			//pSetADCSamplingPoint = pHandle->pFctSetADCSampPointSect4;
		} else { // w_x > 0
			sector = SECTOR_3;
			w_time_ph_a = ( int32_t )( PWM_PERIOD_CYCLES ) / 4 + ( ( w_y - w_x ) / ( int32_t )262144 );
			w_time_ph_c = w_time_ph_a - w_y / 131072;
			w_time_ph_b = w_time_ph_c + w_x / 131072;
			//pSetADCSamplingPoint = pHandle->pFctSetADCSampPointSect3;
		}
	} else { // w_y > 0
		if ( w_z >= 0 )
    {
      sector = SECTOR_2;
      w_time_ph_a = ( int32_t )( PWM_PERIOD_CYCLES ) / 4 + ( ( w_y - w_z ) / ( int32_t )262144 );
      w_time_ph_b = w_time_ph_a + w_z / 131072;
      w_time_ph_c = w_time_ph_a - w_y / 131072;
      //pSetADCSamplingPoint = pHandle->pFctSetADCSampPointSect2;
    }
    else /* w_z < 0 */
      if ( w_x <= 0 )
      {
        sector = SECTOR_6;
        w_time_ph_a = ( int32_t )( PWM_PERIOD_CYCLES ) / 4 + ( ( w_y - w_x ) / ( int32_t )262144 );
        w_time_ph_c = w_time_ph_a - w_y / 131072;
        w_time_ph_b = w_time_ph_c + w_x / 131072;
        //pSetADCSamplingPoint = pHandle->pFctSetADCSampPointSect6;
      }
      else /* w_x > 0 */
      {
        sector = SECTOR_1;
        w_time_ph_a = ( int32_t )( PWM_PERIOD_CYCLES ) / 4 + ( ( w_x - w_z ) / ( int32_t )262144 );
        w_time_ph_b = w_time_ph_a + w_z / 131072;
        w_time_ph_c = w_time_ph_b - w_x / 131072;
        //pSetADCSamplingPoint = pHandle->pFctSetADCSampPointSect1;
      }
	}
	
	TIM1->CCR1 = ( uint16_t )w_time_ph_a;
    TIM1->CCR2 = ( uint16_t )w_time_ph_b;
    TIM1->CCR3 = ( uint16_t )w_time_ph_c;

}

