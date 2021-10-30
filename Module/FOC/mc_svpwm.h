/*
 * mc_svpwm.h
 *
 * Copyright (C) 2018
 * Author:
 *
 * This file is Confidentiality.
 */

#ifndef __MC_SVPWM_H
#define __MC_SVPWM_H

#include "sys.h"
#include "bsp_pwm.h"
#include "circle_limitation.h"

void svpwm_calc(alphabeta_t v_alpha_beta);
void sin_cos_table(int16_t angle, int16_t *sin, int16_t *cos);

#endif

