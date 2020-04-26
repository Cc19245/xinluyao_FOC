/*
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "bsp_encoder.h"

#include "delay.h"
#include "mc_angle.h"

#define ENCODER_SAFE_WORD (0x7E00)
static uint8_t crc8_tab[256];

static void crc8_tab_create(void)
	
{
    uint32_t i, j;
    uint8_t crc;

    for (i = 0; i < 256; i ++) {
        crc = i;

        for (j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc <<= 1;
                crc ^= 0x1D;
            } else {
                crc <<= 1;
            }
        }

        crc8_tab[i] = crc;
    }
}

static void bsp_spi1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	ENC_CS_H;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	SPI_Cmd(SPI1, DISABLE);
	SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);
}

static uint16_t bsp_enc_spi_rw(uint16_t dat)
{
    uint16_t res;

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    SPI_I2S_SendData(SPI1, dat);

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

    res = SPI_I2S_ReceiveData(SPI1);

    return res;
}

void bsp_enc_init(void)
{
	crc8_tab_create();
	
	bsp_spi1_init();
	
	ENC_CS_H;
}
static uint8_t ang_rx_crc8_tab_calc(int16_t encoder_origin)
{
    uint8_t msg[4];
    uint8_t crc = 0xFF;

    msg[0] = 0x80;
    msg[1] = 0x21;
    msg[2] = BYTE1(encoder_origin);
    msg[3] = BYTE0(encoder_origin);

    crc = crc8_tab[crc ^ msg[0]];
    crc = crc8_tab[crc ^ msg[1]];
    crc = crc8_tab[crc ^ msg[2]];
    crc = crc8_tab[crc ^ msg[3]];

    return (~crc);
}

void bsp_enc_read_angle(void)
{
	int16_t val_origin, val_tmp;
	uint16_t code;
	
	ENC_CS_L;
	
	bsp_enc_spi_rw(CMD_READ_ANGLE);
	val_origin = bsp_enc_spi_rw(0xFFFF);
    val_tmp = val_origin << 1;
	code = bsp_enc_spi_rw(0xFFFF);
	
	if (ang_rx_crc8_tab_calc(val_origin) == BYTE0(code) && (code & 0x1000)) {
		mc_angle.value = val_tmp;
		mc_angle_process(mc_angle.value);
	}
	
	ENC_CS_H;

}
