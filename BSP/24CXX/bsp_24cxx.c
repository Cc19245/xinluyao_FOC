/*
 * bsp_24cxx.c
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 * 技术讨论QQ群: 680440867 
 */


#include "bsp_24cxx.h" 
#include "delay.h"

//初始化IIC接口
void bsp_24cxx_init(void)
{
	bsp_iic_init();
}

//在AT24CXX指定地址读出一个数据
//read_addr:开始读数的地址  
//返回值  :读到的数据
uint8_t bsp_24cxx_read_one_byte(uint16_t read_addr)
{				  
	uint8_t temp=0;		  	    																 
    bsp_iic_start();  
	if(EE_TYPE>AT24C16)
	{
		bsp_iic_send_byte(0XA0);	   //发送写命令
		bsp_iic_wait_ack();
		bsp_iic_send_byte(read_addr>>8);//发送高地址
		bsp_iic_wait_ack();		 
	}else bsp_iic_send_byte(0XA0+((read_addr/256)<<1));   //发送器件地址0XA0,写数据 	 

	bsp_iic_wait_ack(); 
    bsp_iic_send_byte(read_addr%256);   //发送低地址
	bsp_iic_wait_ack();	    
	bsp_iic_start();  	 	   
	bsp_iic_send_byte(0XA1);           //进入接收模式			   
	bsp_iic_wait_ack();	 
    temp=bsp_iic_read_byte(0);		   
    bsp_iic_stop();//产生一个停止条件	    
	return temp;
}
//在AT24CXX指定地址写入一个数据
//write_addr  :写入数据的目的地址    
//data_to_write:要写入的数据
void bsp_24cxx_write_one_byte(uint16_t write_addr,uint8_t data_to_write)
{				   	  	    																 
    bsp_iic_start();  
	if(EE_TYPE>AT24C16)
	{
		bsp_iic_send_byte(0XA0);	    //发送写命令
		bsp_iic_wait_ack();
		bsp_iic_send_byte(write_addr>>8);//发送高地址
 	}else
	{
		bsp_iic_send_byte(0XA0+((write_addr/256)<<1));   //发送器件地址0XA0,写数据 
	}	 
	bsp_iic_wait_ack();	   
    bsp_iic_send_byte(write_addr%256);   //发送低地址
	bsp_iic_wait_ack(); 	 										  		   
	bsp_iic_send_byte(data_to_write);     //发送字节							   
	bsp_iic_wait_ack();  		    	   
    bsp_iic_stop();//产生一个停止条件 
	delay_ms(10);	 
}
//在AT24CXX里面的指定地址开始写入长度为len的数据
//该函数用于写入16bit或者32bit的数据.
//write_addr  :开始写入的地址  
//data_to_write:数据数组首地址
//len        :要写入数据的长度2,4
void bsp_24cxx_write_len_byte(uint16_t write_addr,uint32_t data_to_write,uint8_t len)
{  	
	uint8_t t;
	for(t=0;t<len;t++)
	{
		bsp_24cxx_write_one_byte(write_addr+t,(data_to_write>>(8*t))&0xff);
	}												    
}

//在AT24CXX里面的指定地址开始读出长度为len的数据
//该函数用于读出16bit或者32bit的数据.
//read_addr   :开始读出的地址 
//返回值     :数据
//len        :要读出数据的长度2,4
uint32_t bsp_24cxx_read_len_byte(uint16_t read_addr,uint8_t len)
{  	
	uint8_t t;
	uint32_t temp=0;
	for(t=0;t<len;t++)
	{
		temp<<=8;
		temp+=bsp_24cxx_read_one_byte(read_addr+len-t-1); 	 				   
	}
	return temp;												    
}
//检查AT24CXX是否正常
//这里用了24XX的最后一个地址(255)来存储标志字.
//如果用其他24C系列,这个地址要修改
//返回1:检测失败
//返回0:检测成功
uint8_t bsp_24cxx_check(void)
{
	uint8_t temp;
	temp=bsp_24cxx_read_one_byte(255);//避免每次开机都写AT24CXX			   
	if(temp==0X55)return 0;		   
	else//排除第一次初始化的情况
	{
		bsp_24cxx_write_one_byte(255,0X55);
	    temp=bsp_24cxx_read_one_byte(255);	  
		if(temp==0X55)return 0;
	}
	return 1;											  
}

//在AT24CXX里面的指定地址开始读出指定个数的数据
//read_addr :开始读出的地址 对24c02为0~255
//p_buff  :数据数组首地址
//num_to_read:要读出数据的个数
void bsp_24cxx_read(uint16_t read_addr,uint8_t *p_buff,uint16_t num_to_read)
{
	while(num_to_read)
	{
		*p_buff++=bsp_24cxx_read_one_byte(read_addr++);	
		num_to_read--;
	}
}  
//在AT24CXX里面的指定地址开始写入指定个数的数据
//write_addr :开始写入的地址 对24c02为0~255
//p_buff   :数据数组首地址
//num_to_write:要写入数据的个数
void bsp_24cxx_write(uint16_t write_addr,uint8_t *p_buff,uint16_t num_to_write)
{
	while(num_to_write--)
	{
		bsp_24cxx_write_one_byte(write_addr,*p_buff);
		write_addr++;
		p_buff++;
	}
}
 











