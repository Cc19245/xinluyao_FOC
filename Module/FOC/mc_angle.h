/*
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 * 技术讨论QQ群: 680440867
 */

#ifndef __MC_ANGLE_H
#define __MC_ANGLE_H	
 
#include "sys.h"

typedef struct {
    int16_t value;
	
    uint16_t mech;
    uint16_t last_mech;
	
    int16_t elec;
    int16_t elec_offset;
} mc_angle_t;

extern mc_angle_t mc_angle;

	
void mc_angle_init(void);
void mc_angle_process(int16_t val);

void mc_set_elec_offset(void);
void mc_read_elec_offset(void);
void mc_save_elec_offset(void);

#endif
