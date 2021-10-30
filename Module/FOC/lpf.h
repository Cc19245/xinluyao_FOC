/*
 * mc_ctrl.c
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#ifndef __LPF_H
#define __LPF_H

#include "sys.h"

typedef struct {
    float z1;
    float tc;
    float in;
    float out;
} lpf_1rd_t;

typedef struct {
    float z1;
    float z2;
    float z3;
    float t1;
    float t2;
    float tc;
    float in;
    float out;
} lpf_3rd_t;

void lpf_1rd_init(lpf_1rd_t *lpf1rd, float tc, float z);
float lpf_1rd_calc(lpf_1rd_t *lpf1rd, float new_sample);

void lpf_3rd_init(lpf_3rd_t *lpf3rd, float tc, float z);
float lpf_3rd_calc(lpf_3rd_t *lpf3rd, float new_sample);

#endif


