/*
 * mc_foc.h
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#ifndef __MC_FOC_H
#define __MC_FOC_H

#include "sys.h"
#include "circle_limitation.h"

void foc_v_qd_set(qd_t set);
void foc_update(void);

#endif

