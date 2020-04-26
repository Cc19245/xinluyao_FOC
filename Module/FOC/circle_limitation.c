/*
 * circle_limitation.c
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "circle_limitation.h"

/* MMI Table Motor 1 MAX_MODULATION_92_PER_CENT */
#define START_INDEX     54
#define MAX_MODULE      30145   // root(Vd^2+Vq^2) <= MAX_MODULE = 32767*92%
#define MMITABLE {\
32424,32091,31929,31611,31302,31002,30855,30568,30289,30017,\
29884,29622,29368,29243,28998,28759,28526,28412,28187,27968,\
27753,27648,27441,27238,27040,26942,26750,26563,26470,26288,\
26110,25935,25849,25679,25513,25350,25269,25111,24955,24803,\
24727,24579,24433,24361,24219,24079,23942,23874,23740,23609,\
23479,23415,23289,23165,23042,22982,22863,22745,22629,22572,\
22459,22347,22292,22183,22075,21970,21917,21813,21711,21610,\
21561,21462,21365,21268\
}

uint16_t circle_limit_table[74] = MMITABLE;

/*
	Check whether Vqd.q^2 + Vqd.d^2 <= 32767^2
 and if not it applies a limitation keeping constant ratio
 Vqd.q / Vqd.d.
*/
qd_t circle_limitation(qd_t v_qd)
{
	uint16_t table_element;
	uint32_t uw_temp;
	int32_t sw_temp;

	qd_t local_vqd = v_qd;
	sw_temp = (int32_t)(v_qd.q) * (v_qd.q) +
			  (int32_t)(v_qd.d) * (v_qd.d);	
	
	uw_temp = (uint32_t)sw_temp;
	
	/* uw_temp 最小值 0, 最大值 2*32767*32767 */
	if (uw_temp > (uint32_t)(MAX_MODULE) * MAX_MODULE) {
		uw_temp /= (uint32_t)(16777216); // 16777216 = 2 * 32768 * 256

		// 计算前，uw_temp 最小值 p_handle->start_index, 最大值 127 
		// 计算后，uw_temp 最小值0， 最大值127 - p_handle->start_index
		uw_temp -= START_INDEX;

		table_element = circle_limit_table[(uint8_t)uw_temp];

		sw_temp = v_qd.q * (int32_t)table_element;
		local_vqd.q = (int16_t)(sw_temp / 32768);

		sw_temp = v_qd.d * (int32_t)(table_element);
		local_vqd.d = (int16_t)(sw_temp / 32768);
	}

	return (local_vqd);
}


