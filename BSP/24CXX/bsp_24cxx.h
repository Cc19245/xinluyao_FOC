/*
 * bsp_24cxx.h
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 * 技术讨论QQ群: 680440867 
 */


#ifndef __BSP_24CXX_H
#define __BSP_24CXX_H

#include "bsp_myiic.h"   					  

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  

//Mini STM32开发板使用的是24c02，所以定义EE_TYPE为AT24C02
#define EE_TYPE AT24C02

//初始化IIC接口
void bsp_24cxx_init(void);
uint8_t bsp_24cxx_read_one_byte(uint16_t read_addr);
void bsp_24cxx_write_one_byte(uint16_t write_addr,uint8_t data_to_write);
void bsp_24cxx_write_len_byte(uint16_t write_addr,uint32_t data_to_write,uint8_t len);
uint32_t bsp_24cxx_read_len_byte(uint16_t read_addr,uint8_t len);
uint8_t bsp_24cxx_check(void);
void bsp_24cxx_read(uint16_t read_addr,uint8_t *p_buff,uint16_t num_to_read);
void bsp_24cxx_write(uint16_t write_addr,uint8_t *p_buff,uint16_t num_to_write);
					  
#endif
















