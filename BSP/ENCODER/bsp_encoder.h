/*
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H	 

#include "sys.h"

#define ENC_CS_H			GPIOB->BSRR = GPIO_Pin_5
#define ENC_CS_L			GPIOB->BRR = GPIO_Pin_5

#define CMD_READ_STATUS                 0x8001
#define CMD_READ_ANGLE                  0x8021

#define CMD_WRITE_MOD1                  0x5061
#define CMD_WRITE_MOD2                  0x5081
#define CMD_WRITE_IFAB                  0x5061
#define CMD_WRITE_MOD4                  0x5061

void bsp_enc_init(void);
void bsp_enc_read_angle(void);
void bsp_enc_write_data(uint16_t cmd, uint16_t dat);

#endif
