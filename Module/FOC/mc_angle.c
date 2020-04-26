/*
 * mc_angle.c
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 * 技术讨论QQ群: 680440867
 */

#include "mc_angle.h"

#include "delay.h"
#include "bsp_encoder.h"
#include "bsp_24cxx.h"

#include "mc_const.h"
#include "lpf.h"

#include "mc_flag.h"

mc_angle_t mc_angle = {
	.value = 0,
	
	.mech = 0,
	.last_mech = 0,
	
	.elec = 0,
	.elec_offset = 0,
};
 
void mc_angle_init()
{
	for (uint8_t i = 0; i < 10; i++) {
		bsp_enc_read_angle();
		delay_us(50);
	}	
}

/**
  * @brief  根据编码器数据计算电角度和电机位置
  * @param val:从编码器读取到的数值
  */
void mc_angle_process(int16_t val)
{
	static int32_t circle = 0;
	
	int16_t value_lpf = val;

	// 计算电角度，MC_POLE_PIARS_NUM为电机极对数
    mc_angle.elec = value_lpf * MC_POLE_PAIRS_NUM - mc_angle.elec_offset;
	
	uint16_t mech_tmp = (uint16_t) value_lpf;	// 将编码器值转换为无符号类型，方便进行右移运算
    mc_angle.mech = mech_tmp >> 1;	// TLE5012B有效数据为15bit，将数据右移一位，去掉无效的数据位


    /*
		测量电机当前位置。circle记录电机转过的圈数，电机mech数据范围为0~32768。根据电机正常转速，diff绝对值不会太大。
	如果diff <= -20000，说明电机mech值从最大变为最小(电机正方向转过一圈)；如diff >= 20000,说明电机mech值从最小跳变到最大(电机正方向转过一圈)
	20000为经验值，在20000附近很大范围内的值都可以正常工作。根据电机转速不同，可以调整这个值。
    */
    int32_t diff;
    diff = mc_angle.mech - mc_angle.last_mech;

    if (diff <= -20000) {
        circle += 1;
    } else if (diff >= 20000) {
        circle -= 1;
    }

    mc_angle.last_mech = mc_angle.mech;

	// 计算电机位置
    mc_pos = circle * UINT15_MAX + mc_angle.mech;
}

/**
  * @brief  设置电角度校准值
  */
void mc_set_elec_offset(void)
{
	int16_t value_lpf = mc_angle.value;
	mc_angle.elec_offset = value_lpf * MC_POLE_PAIRS_NUM;
}

/**
  * @brief  从EEPROM读取电角度校准值
  */
void mc_read_elec_offset()
{
	bsp_24cxx_read(0, (uint8_t *)&mc_angle.elec_offset, sizeof(mc_angle.elec_offset));
}

/**
  * @brief  保存电角度校准值到EEPROM
  */
void mc_save_elec_offset()
{
	bsp_24cxx_write(0, (uint8_t *)&mc_angle.elec_offset, sizeof(mc_angle.elec_offset));
}
