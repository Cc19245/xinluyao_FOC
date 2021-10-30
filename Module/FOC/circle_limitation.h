/*
 * circle_limitation.h
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#ifndef __CIRCLE_LIMITATION_H
#define __CIRCLE_LIMITATION_H

#include "sys.h"
#include "mc_type.h"

typedef struct {
	uint16_t max_module;	/* circle limitation maximum allowed module */

	uint16_t circle_limit_table[87];	/* circle limitation table */
	uint8_t start_index;
} circle_limitation_handle_t;

qd_t circle_limitation(qd_t v_qd);

#endif
