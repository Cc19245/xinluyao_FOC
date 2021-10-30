/*
 * lpf.c
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "lpf.h"

static void lpf_1rd_reset(lpf_1rd_t *lpf1rd)
{
	lpf1rd->z1 = 0;
	lpf1rd->tc = 0;
	lpf1rd->in = 0;
	lpf1rd->out = 0;
}

void lpf_1rd_init(lpf_1rd_t *lpf1rd, float tc, float z)
{
    lpf_1rd_reset(lpf1rd);
    lpf1rd->tc = tc;
}

float lpf_1rd_calc(lpf_1rd_t *lpf1rd, float new_sample)
{
    lpf1rd->in =  new_sample;
    lpf1rd->out = lpf1rd->z1 + lpf1rd->tc * (lpf1rd->in - lpf1rd->z1);
    lpf1rd->z1 =  lpf1rd->out;

    return lpf1rd->out;
}


static void lpf_3rd_reset(lpf_3rd_t *lpf3rd)
{
    lpf3rd->z1 = 0;
    lpf3rd->z2 = 0;
    lpf3rd->z3 = 0;
    lpf3rd->t1 = 0;
    lpf3rd->t2 = 0;
    lpf3rd->in = 0;
    lpf3rd->out = 0;
}

void lpf_3rd_init(lpf_3rd_t *lpf3rd, float tc, float z)
{
    lpf_3rd_reset(lpf3rd);
    lpf3rd->tc = tc;
    lpf3rd->z1 = z;
    lpf3rd->z2 = z;
    lpf3rd->z3 = z;

}

float lpf_3rd_calc(lpf_3rd_t *lpf3rd, float new_sample)
{
    lpf3rd->in = new_sample;
    lpf3rd->t1 = lpf3rd->z1 + lpf3rd->tc * (lpf3rd->in - lpf3rd->z1);
    lpf3rd->z1 = lpf3rd->t1;
    lpf3rd->t2 = lpf3rd->z2 + lpf3rd->tc * (lpf3rd->t1 - lpf3rd->z2);
    lpf3rd->z2 = lpf3rd->t2;
    lpf3rd->out = lpf3rd->z3 + lpf3rd->tc * (lpf3rd->t2 - lpf3rd->z3);
    lpf3rd->z3 = lpf3rd->out;

    return lpf3rd->out;
}


