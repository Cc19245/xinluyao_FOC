/*
 * led driver.
 *
 * Copyright (C) 2019 XinLuYao, Inc. All Rights Reserved.
 * Written by quantum (quantum@whxinluyao.com)
 * Website: www.whxinluyao.com
 */

#include "bsp_lcd.h"
#include "delay.h"

lcd_dev_t lcddev;

//LCD的画笔颜色和背景色	   
uint32_t POINT_COLOR = 0x00000000;
uint32_t BACK_COLOR = 0xFFFFFFFF; 

static uint16_t bgr2rgb(uint16_t bgr)
{
    uint16_t r, g, b, rgb;
    b = (bgr >> 0) & 0x1f;
    g = (bgr >> 5) & 0x3f;
    r = (bgr >> 11) & 0x1f;
    rgb = (b << 11) + (g << 5) + (r << 0);
    return (rgb);
}

//当mdk -O1时间优化时需要设置
//延时i
static void opt_delay(uint8_t i)
{
	while(i--);
}

static void lcd_wr_reg(volatile uint16_t val)
{
	lcd->reg = val;
}

static void lcd_wr_data(volatile uint16_t data)
{
	lcd->ram = data;
}

static uint16_t lcd_rd_data(void)
{
	volatile uint16_t ram;
	ram = lcd->ram;
	return ram;
}

static void lcd_write_reg(volatile uint16_t reg, volatile uint16_t val)
{
	lcd->reg = reg;
	lcd->ram = val;
}

static uint16_t lcd_read_reg(uint16_t reg)
{
	lcd_wr_reg(reg);
	delay_us(5);
	return lcd_rd_data();
}

void bsp_lcd_write_ram_prepare(void)
{
	lcd->reg = lcddev.wramcmd;
}

void bsp_lcd_write_ram(uint16_t rgb)
{
	lcd->ram = rgb;
}

/* @brief 读取LCD上某个点的颜色
 * @param  x lcd横坐标(屏幕左上角为起点)
 * @param  y lcd纵坐标(屏幕左上角为起点)
 * @retval ret rgb565格式的颜色数据
 */
uint16_t bsp_lcd_read_point(uint16_t x, uint16_t y)
{
    uint16_t r = 0, g = 0, b = 0;

    if (x >= lcddev.width || y >= lcddev.height)
        return 0;   //超过了范围,直接返回

    bsp_lcd_set_cursor(x, y);

    if (lcddev.id == 0x9341)
        lcd_wr_reg(0x2E);
    else if (lcddev.id == 0x8009 || lcddev.id == 0x5510)
        lcd_wr_reg(0x2E00);

    r = lcd_rd_data();                              //dummy Read
    opt_delay(2);
    r = lcd_rd_data();                               //实际坐标颜色
    //9341/OTM8009要分2次读出
    opt_delay(2);
    b = lcd_rd_data();
    g = r & 0XFF;   //对于9341/8009,第一次读取的是RG的值,R在前,G在后,各占8位
    g <<= 8;

    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11)); //ILI9341/NT35310/NT35510需要公式转换一下
}

/* @brief 打开LCD显示
 * @retval none
 */
void bsp_lcd_disp_on(void)
{
    if (lcddev.id == 0X9341)
        lcd_wr_reg(0X29);
    else if (lcddev.id == 0x8009 || lcddev.id == 5510)
        lcd_wr_reg(0X2900);

}

/* @brief 关闭LCD显示
 * @retval none
 */
void bsp_lcd_disp_off(void)
{
    if (lcddev.id == 0X9341)
        lcd_wr_reg(0X28);
    else if (lcddev.id == 0x8009 || lcddev.id == 0x5510)
        lcd_wr_reg(0X2800);

}


/* @brief 设置光标位置
 * @param  Xpos lcd横坐标(屏幕左上角为起点)
 * @param  Ypos lcd纵坐标(屏幕左上角为起点)
 * @retval none
 */
void bsp_lcd_set_cursor(uint16_t Xpos, uint16_t Ypos)
{
    if (lcddev.id == 0X9341)
    {
        lcd_wr_reg(lcddev.setxcmd);
        lcd_wr_data(Xpos >> 8);
        lcd_wr_data(Xpos & 0XFF);
        lcd_wr_reg(lcddev.setycmd);
        lcd_wr_data(Ypos >> 8);
        lcd_wr_data(Ypos & 0XFF);
    }
    else if (lcddev.id == 0x8009)
    {
        bsp_lcd_set_window(Xpos, Ypos, lcddev.width, lcddev.height);
    }
    else if (lcddev.id == 0x5510) 
    {
        lcd_wr_reg(lcddev.setxcmd);
        lcd_wr_data(Xpos >> 8);
        lcd_wr_reg(lcddev.setxcmd + 1);
        lcd_wr_data(Xpos & 0XFF);
        lcd_wr_reg(lcddev.setycmd);
        lcd_wr_data(Ypos >> 8);
        lcd_wr_reg(lcddev.setycmd + 1);
        lcd_wr_data(Ypos & 0XFF);
    }
}

/* @brief 设置LCD扫描方向
 * @param  dir lcd扫描方向
 * @retval none
 */
void bsp_lcd_scan_dir(uint8_t dir)
{
    uint16_t regval = 0;
    uint16_t dirreg = 0;
    uint16_t temp;

    if (lcddev.id == 0x9341 || lcddev.id == 0x8009 || lcddev.id == 0x5510)   //9341/8009,特殊处理
    {
        switch (dir)
        {
        case L2R_U2D://从左到右,从上到下
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;

        case L2R_D2U://从左到右,从下到上
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;

        case R2L_U2D://从右到左,从上到下
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;

        case R2L_D2U://从右到左,从下到上
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;

        case U2D_L2R://从上到下,从左到右
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;

        case U2D_R2L://从上到下,从右到左
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;

        case D2U_L2R://从下到上,从左到右
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;

        case D2U_R2L://从下到上,从右到左
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
        }

        if (lcddev.id == 0X8009 || lcddev.id == 0x5510)
            dirreg = 0X3600;
        else
            dirreg = 0X36;

        if ((lcddev.id != 0X8009) && (lcddev.id != 0x5510))
            regval |= 0X08; //8009不需要BGR

        lcd_write_reg(dirreg, regval);

        if (regval & 0X20)
        {
            if (lcddev.width < lcddev.height)   //交换X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height)   //交换X,Y
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }

        if (lcddev.id == 0x8009 || lcddev.id == 0x5510)
        {
            lcd_wr_reg(lcddev.setxcmd);
            lcd_wr_data(0);
            lcd_wr_reg(lcddev.setxcmd + 1);
            lcd_wr_data(0);
            lcd_wr_reg(lcddev.setxcmd + 2);
            lcd_wr_data((lcddev.width - 1) >> 8);
            lcd_wr_reg(lcddev.setxcmd + 3);
            lcd_wr_data((lcddev.width - 1) & 0XFF);
            lcd_wr_reg(lcddev.setycmd);
            lcd_wr_data(0);
            lcd_wr_reg(lcddev.setycmd + 1);
            lcd_wr_data(0);
            lcd_wr_reg(lcddev.setycmd + 2);
            lcd_wr_data((lcddev.height - 1) >> 8);
            lcd_wr_reg(lcddev.setycmd + 3);
            lcd_wr_data((lcddev.height - 1) & 0XFF);
        }
        else
        {
            lcd_wr_reg(lcddev.setxcmd);
            lcd_wr_data(0);
            lcd_wr_data(0);
            lcd_wr_data((lcddev.width - 1) >> 8);
            lcd_wr_data((lcddev.width - 1) & 0XFF);
            lcd_wr_reg(lcddev.setycmd);
            lcd_wr_data(0);
            lcd_wr_data(0);
            lcd_wr_data((lcddev.height - 1) >> 8);
            lcd_wr_data((lcddev.height - 1) & 0XFF);
        }
    }
}

/* @brief 在LCD显示屏上画点，颜色为POINT_COLOR
 * @param x lcd横坐标(屏幕左上角为起点)
 * @param y lcd纵坐标(屏幕左上角为起点)
 * @retval none
 */
void bsp_lcd_draw_point(uint16_t x, uint16_t y)
{
    bsp_lcd_set_cursor(x, y);       //设置光标位置
    bsp_lcd_write_ram_prepare();    //开始写入GRAM
    lcd->ram = POINT_COLOR;
}

/* @brief 在LCD显示屏上画点，颜色为color
 * @param x lcd横坐标(屏幕左上角为起点)
 * @param y lcd纵坐标(屏幕左上角为起点)
 * @param color rgb565格式颜色值
 * @retval none
 */
void bsp_lcd_draw_color_point(uint16_t x, uint16_t y, uint16_t color)
{
    if (lcddev.id == 0x9341)
    {
        lcd_wr_reg(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_data(x & 0XFF);
        lcd_wr_reg(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_data(y & 0XFF);
    }
    else if (lcddev.id == 0x8009)
    {
        lcd_wr_reg(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_reg(lcddev.setxcmd + 1);
        lcd_wr_data(x & 0XFF);
        lcd_wr_reg(lcddev.setxcmd + 2);
        lcd_wr_data(x >> 8);
        lcd_wr_reg(lcddev.setxcmd + 3);
        lcd_wr_data(x & 0XFF);

        lcd_wr_reg(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_reg(lcddev.setycmd + 1);
        lcd_wr_data(y & 0XFF);
        lcd_wr_reg(lcddev.setycmd + 2);
        lcd_wr_data(y >> 8);
        lcd_wr_reg(lcddev.setycmd + 3);
        lcd_wr_data(y & 0xFF);
    }
    else if (lcddev.id == 0x5510)
    {
        lcd_wr_reg(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_reg(lcddev.setxcmd + 1);
        lcd_wr_data(x & 0XFF);

        lcd_wr_reg(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_reg(lcddev.setycmd + 1);
        lcd_wr_data(y & 0XFF);
    }

    lcd->reg = lcddev.wramcmd;
    lcd->ram = color;
}

/* @brief 设置LCD为横屏还是竖屏方式显示
 * @param dir 显示方式 0：横屏，1：竖屏
 * @retval none
 */
void bsp_lcd_disp_dir(uint8_t dir)
{
    lcddev.dir = dir;   //竖屏/横屏

    if (dir == 0)       //竖屏
    {
        lcddev.width = 240;
        lcddev.height = 320;

        if (lcddev.id == 0X9341)
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }
        else if (lcddev.id == 0x8009 || lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 480;
            lcddev.height = 800;
        }
    }
    else                 //横屏
    {
        lcddev.width = 320;
        lcddev.height = 240;

        if (lcddev.id == 0X9341)
        {
            lcddev.wramcmd = 0X2C;
            lcddev.setxcmd = 0X2A;
            lcddev.setycmd = 0X2B;
        }
        else if (lcddev.id == 0x8009 || lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0X2C00;
            lcddev.setxcmd = 0X2A00;
            lcddev.setycmd = 0X2B00;
            lcddev.width = 800;
            lcddev.height = 480;
        }
    }

    bsp_lcd_scan_dir(DFT_SCAN_DIR);
}


/* @brief 设置LCD显示区域大小
 * @param sx 起始x坐标
 * @param sy 起始y坐标
 * @param ex 结束x坐标
 * @param ey 结束y坐标
 * @retval none
 */
void bsp_lcd_set_window(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    if (lcddev.id == 0X9341)
    {
        lcd_wr_reg(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_data(sx & 0XFF);
        lcd_wr_data(ex >> 8);
        lcd_wr_data(ex & 0XFF);
        lcd_wr_reg(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_data(sy & 0XFF);
        lcd_wr_data(ey >> 8);
        lcd_wr_data(ey & 0XFF);
    }
    else if (lcddev.id == 0x8009 || lcddev.id == 0x5510)
    {

        lcd_wr_reg(lcddev.setxcmd);
        lcd_wr_data(sx >> 8);
        lcd_wr_reg(lcddev.setxcmd + 1);
        lcd_wr_data(sx & 0XFF);
        lcd_wr_reg(lcddev.setxcmd + 2);
        lcd_wr_data(ex >> 8);
        lcd_wr_reg(lcddev.setxcmd + 3);
        lcd_wr_data(ex & 0XFF);


        lcd_wr_reg(lcddev.setycmd);
        lcd_wr_data(sy >> 8);
        lcd_wr_reg(lcddev.setycmd + 1);
        lcd_wr_data(sy & 0XFF);
        lcd_wr_reg(lcddev.setycmd + 2);
        lcd_wr_data(ey >> 8);
        lcd_wr_reg(lcddev.setycmd + 3);
        lcd_wr_data(ey & 0XFF);
    }
}
        
/* @brief 初始化LCD显示屏
 * @retval none
 */
void bsp_lcd_init(void)
{ 	  
	GPIO_InitTypeDef GPIO_InitStructure;
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
	FSMC_NORSRAMTimingInitTypeDef  writeTiming;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC,ENABLE);	//使能FSMC时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOG,ENABLE);//使能PORTB,D,E,G以及AFIO复用功能时钟

	//PE6 推挽输出 背光
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_14|GPIO_Pin_15;				 //	//PORTD复用推挽输出  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
	 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 //	//PORTD复用推挽输出  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);    	    	 											 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_12; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure); 

	readWriteTiming.FSMC_AddressSetupTime = 0x01;	 //地址建立时间（ADDSET）为2个HCLK 1/36M=27ns
	readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到	
	readWriteTiming.FSMC_DataSetupTime = 0x0f;		 // 数据保存时间为16个HCLK,因为液晶驱动IC的读数据的时候，速度不能太快，尤其对1289这个IC。
	readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
	readWriteTiming.FSMC_CLKDivision = 0x00;
	readWriteTiming.FSMC_DataLatency = 0x00;
	readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 


	writeTiming.FSMC_AddressSetupTime = 0x00;	 //地址建立时间（ADDSET）为1个HCLK  
	writeTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
	writeTiming.FSMC_DataSetupTime = 0x03;		 ////数据保存时间为4个HCLK	
	writeTiming.FSMC_BusTurnAroundDuration = 0x00;
	writeTiming.FSMC_CLKDivision = 0x00;
	writeTiming.FSMC_DataLatency = 0x00;
	writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 


	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;//  这里我们使用NE4 ，也就对应BTCR[6],[7]。
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // 不复用数据地址
	FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit   
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable; 
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//  存储器写使能
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // 读写使用不同的时序
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; 
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //读写时序
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //写时序

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置

	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);  // 使能BANK1 
	
	delay_ms(50); // delay 50 ms 
	
	//尝试9341 ID的读取
    lcd_wr_reg(0XD3);
    lcddev.id = lcd_rd_data();  //dummy read
    lcddev.id = lcd_rd_data();  //读到0X00
    lcddev.id = lcd_rd_data();  //读取93
    lcddev.id <<= 8;
    lcddev.id |= lcd_rd_data(); //读取41

    if (lcddev.id != 0X9341)    //非9341,尝试看看是不是OTM8009A
    {
        lcd_wr_reg(0XDA00);
        lcddev.id = lcd_rd_data(); //dummy read
        lcddev.id = lcd_rd_data();

        if (lcddev.id == 0x40) {
            lcddev.id = 0x8009; //OTM8009A
        } else if (lcddev.id == 0x55) {
            lcddev.id = 0x5510; //NT35510
        }
    }

    // printf(" LCD ID:%x\r\n",lcddev.id);
    if (lcddev.id == 0X9341)
    {
        lcd_wr_reg(0xCF);
        lcd_wr_data(0x00);
        lcd_wr_data(0xC1);
        lcd_wr_data(0X30);
        lcd_wr_reg(0xED);
        lcd_wr_data(0x64);
        lcd_wr_data(0x03);
        lcd_wr_data(0X12);
        lcd_wr_data(0X81);
        lcd_wr_reg(0xE8);
        lcd_wr_data(0x85);
        lcd_wr_data(0x10);
        lcd_wr_data(0x7A);
        lcd_wr_reg(0xCB);
        lcd_wr_data(0x39);
        lcd_wr_data(0x2C);
        lcd_wr_data(0x00);
        lcd_wr_data(0x34);
        lcd_wr_data(0x02);
        lcd_wr_reg(0xF7);
        lcd_wr_data(0x20);
        lcd_wr_reg(0xEA);
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xC0);    //Power control
        lcd_wr_data(0x1B);   //VRH[5:0]
        lcd_wr_reg(0xC1);    //Power control
        lcd_wr_data(0x01);   //SAP[2:0];BT[3:0]
        lcd_wr_reg(0xC5);    //VCM control
        lcd_wr_data(0x30);   //3F
        lcd_wr_data(0x30);   //3C
        lcd_wr_reg(0xC7);    //VCM control2
        lcd_wr_data(0XB7);
        lcd_wr_reg(0x36);    // Memory Access Control
        lcd_wr_data(0x48);
        lcd_wr_reg(0x3A);
        lcd_wr_data(0x55);
        lcd_wr_reg(0xB1);
        lcd_wr_data(0x00);
        lcd_wr_data(0x1A);
        lcd_wr_reg(0xB6);    // Display Function Control
        lcd_wr_data(0x0A);
        lcd_wr_data(0xA2);
        lcd_wr_reg(0xF2);    // 3Gamma Function Disable
        lcd_wr_data(0x00);
        lcd_wr_reg(0x26);    //Gamma curve selected
        lcd_wr_data(0x01);
        lcd_wr_reg(0xE0);    //Set Gamma
        lcd_wr_data(0x0F);
        lcd_wr_data(0x2A);
        lcd_wr_data(0x28);
        lcd_wr_data(0x08);
        lcd_wr_data(0x0E);
        lcd_wr_data(0x08);
        lcd_wr_data(0x54);
        lcd_wr_data(0XA9);
        lcd_wr_data(0x43);
        lcd_wr_data(0x0A);
        lcd_wr_data(0x0F);
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_reg(0XE1);    //Set Gamma
        lcd_wr_data(0x00);
        lcd_wr_data(0x15);
        lcd_wr_data(0x17);
        lcd_wr_data(0x07);
        lcd_wr_data(0x11);
        lcd_wr_data(0x06);
        lcd_wr_data(0x2B);
        lcd_wr_data(0x56);
        lcd_wr_data(0x3C);
        lcd_wr_data(0x05);
        lcd_wr_data(0x10);
        lcd_wr_data(0x0F);
        lcd_wr_data(0x3F);
        lcd_wr_data(0x3F);
        lcd_wr_data(0x0F);
        lcd_wr_reg(0x2B);
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_data(0x01);
        lcd_wr_data(0x3f);
        lcd_wr_reg(0x2A);
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_data(0x00);
        lcd_wr_data(0xef);
        lcd_wr_reg(0x11); //Exit Sleep
        delay_ms(120);
        lcd_wr_reg(0x29); //display on
    }
    else if (lcddev.id == 0x8009)
    {
        // Other IF Enable access command 2 registers
        //enable access command2
        lcd_wr_reg(0xFF00);
        lcd_wr_data(0x80);
        lcd_wr_reg(0xFF01);
        lcd_wr_data(0x09);
        lcd_wr_reg(0xFF02);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xFF80);
        lcd_wr_data(0x80);
        lcd_wr_reg(0xFF81);
        lcd_wr_data(0x09);

        lcd_wr_reg(0xFF03);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xC5B1);
        lcd_wr_data(0xA9);
        lcd_wr_reg(0xC591);
        lcd_wr_data(0x0F);

        // Panel Driving Mode:Column Inversion
        lcd_wr_reg(0xC0B4);
        lcd_wr_data(0x50);

        // Gamma Correction Characteristics Setting
        lcd_wr_reg(0xE100);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xE101);
        lcd_wr_data(0x09);
        lcd_wr_reg(0xE102);
        lcd_wr_data(0x0F);
        lcd_wr_reg(0xE103);
        lcd_wr_data(0x0E);
        lcd_wr_reg(0xE104);
        lcd_wr_data(0x07);
        lcd_wr_reg(0xE105);
        lcd_wr_data(0x10);
        lcd_wr_reg(0xE106);
        lcd_wr_data(0x0B);
        lcd_wr_reg(0xE107);
        lcd_wr_data(0x0A);
        lcd_wr_reg(0xE108);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xE109);
        lcd_wr_data(0x07);
        lcd_wr_reg(0xE10A);
        lcd_wr_data(0x0B);
        lcd_wr_reg(0xE10B);
        lcd_wr_data(0x08);
        lcd_wr_reg(0xE10C);
        lcd_wr_data(0x0F);
        lcd_wr_reg(0xE10D);
        lcd_wr_data(0x10);
        lcd_wr_reg(0xE10E);
        lcd_wr_data(0x0A);
        lcd_wr_reg(0xE10F);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xE200);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xE201);
        lcd_wr_data(0x09);
        lcd_wr_reg(0xE202);
        lcd_wr_data(0x0F);
        lcd_wr_reg(0xE203);
        lcd_wr_data(0x0E);
        lcd_wr_reg(0xE204);
        lcd_wr_data(0x07);
        lcd_wr_reg(0xE205);
        lcd_wr_data(0x10);
        lcd_wr_reg(0xE206);
        lcd_wr_data(0x0B);
        lcd_wr_reg(0xE207);
        lcd_wr_data(0x0A);
        lcd_wr_reg(0xE208);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xE209);
        lcd_wr_data(0x07);
        lcd_wr_reg(0xE20A);
        lcd_wr_data(0x0B);
        lcd_wr_reg(0xE20B);
        lcd_wr_data(0x08);
        lcd_wr_reg(0xE20C);
        lcd_wr_data(0x0F);
        lcd_wr_reg(0xE20D);
        lcd_wr_data(0x10);
        lcd_wr_reg(0xE20E);
        lcd_wr_data(0x0A);
        lcd_wr_reg(0xE20F);
        lcd_wr_data(0x01);

        // VCOM voltage setting
        lcd_wr_reg(0xD900);
        lcd_wr_data(0x4E);

        //osc=70HZ
        lcd_wr_reg(0xc181);
        lcd_wr_data(0x77);

        // Power Control Setting 2 for Normal Mode
        lcd_wr_reg(0xc592);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xc595);
        lcd_wr_data(0x34);

        //GVDD / NGVDD setting
        lcd_wr_reg(0xd800);
        lcd_wr_data(0x79);
        lcd_wr_reg(0xd801);
        lcd_wr_data(0x79);

        // Power Control Setting 2 for Normal Mode
        lcd_wr_reg(0xc594);
        lcd_wr_data(0x33);

        //panel timing setting
        lcd_wr_reg(0xc0a3);
        lcd_wr_data(0x1B);

        // Power control setting
        lcd_wr_reg(0xc582);
        lcd_wr_data(0x83);

        lcd_wr_reg(0xc481);
        lcd_wr_data(0x83);        //source driver setting

        //RGB Video Mode Setting
        lcd_wr_reg(0xc1a1);
        lcd_wr_data(0x0E);

        // Panel Type : Normal Panel
        lcd_wr_reg(0xb3a6);
        lcd_wr_data(0x20);
        lcd_wr_reg(0xb3a7);
        lcd_wr_data(0x01);

        // GOA VST
        lcd_wr_reg(0xce80);
        lcd_wr_data(0x85);
        lcd_wr_reg(0xce81);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xce82);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xce83);
        lcd_wr_data(0x84);
        lcd_wr_reg(0xce84);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xce85);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcea0);
        lcd_wr_data(0x18);
        lcd_wr_reg(0xcea1);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcea2);
        lcd_wr_data(0x03);
        lcd_wr_reg(0xcea3);
        lcd_wr_data(0x39);
        lcd_wr_reg(0xcea4);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcea5);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcea6);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcea7);
        lcd_wr_data(0x18);
        lcd_wr_reg(0xcea8);
        lcd_wr_data(0x03);
        lcd_wr_reg(0xcea9);
        lcd_wr_data(0x03);
        lcd_wr_reg(0xceaa);
        lcd_wr_data(0x3a);
        lcd_wr_reg(0xceab);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xceac);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcead);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xceb0);
        lcd_wr_data(0x18);
        lcd_wr_reg(0xceb1);
        lcd_wr_data(0x02);
        lcd_wr_reg(0xceb2);
        lcd_wr_data(0x03);
        lcd_wr_reg(0xceb3);
        lcd_wr_data(0x3b);
        lcd_wr_reg(0xceb4);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xceb5);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xceb6);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xceb7);
        lcd_wr_data(0x18);
        lcd_wr_reg(0xceb8);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xceb9);
        lcd_wr_data(0x03);
        lcd_wr_reg(0xceba);
        lcd_wr_data(0x3c);
        lcd_wr_reg(0xcebb);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcebc);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcebd);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcfc0);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xcfc1);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xcfc2);
        lcd_wr_data(0x20);
        lcd_wr_reg(0xcfc3);
        lcd_wr_data(0x20);
        lcd_wr_reg(0xcfc4);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcfc5);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcfc6);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xcfc7);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcfc8);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcfc9);
        lcd_wr_data(0x00);

        lcd_wr_reg(0xcfd0);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb80);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb81);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb82);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb83);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb84);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb85);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb86);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb87);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb88);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb89);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb90);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb91);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb92);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb93);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb94);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb95);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb96);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb97);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb98);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb99);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb9a);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb9b);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb9c);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb9d);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcb9e);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcba0);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcba1);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcba2);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcba3);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcba4);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcba5);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcba6);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcba7);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcba8);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcba9);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbaa);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbab);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbac);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbad);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbae);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbb0);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbb1);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbb2);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbb3);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbb4);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbb5);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbb6);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbb7);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbb8);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbb9);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbc0);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbc1);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcbc2);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcbc3);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcbc4);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcbc5);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcbc6);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbc7);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbc8);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbc9);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbca);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbcb);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbcc);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbcd);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbce);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbd0);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbd1);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbd2);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbd3);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbd4);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbd5);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbd6);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcbd7);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcbd8);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcbd9);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcbda);
        lcd_wr_data(0x04);
        lcd_wr_reg(0xcbdb);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbdc);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbdd);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbde);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbe0);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbe1);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbe2);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbe3);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbe4);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbe5);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbe6);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbe7);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbe8);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbe9);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcbf0);
        lcd_wr_data(0xFF);
        lcd_wr_reg(0xcbf1);
        lcd_wr_data(0xFF);
        lcd_wr_reg(0xcbf2);
        lcd_wr_data(0xFF);
        lcd_wr_reg(0xcbf3);
        lcd_wr_data(0xFF);
        lcd_wr_reg(0xcbf4);
        lcd_wr_data(0xFF);
        lcd_wr_reg(0xcbf5);
        lcd_wr_data(0xFF);
        lcd_wr_reg(0xcbf6);
        lcd_wr_data(0xFF);
        lcd_wr_reg(0xcbf7);
        lcd_wr_data(0xFF);
        lcd_wr_reg(0xcbf8);
        lcd_wr_data(0xFF);
        lcd_wr_reg(0xcbf9);
        lcd_wr_data(0xFF);
        lcd_wr_reg(0xcc80);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc81);
        lcd_wr_data(0x26);
        lcd_wr_reg(0xcc82);
        lcd_wr_data(0x09);
        lcd_wr_reg(0xcc83);
        lcd_wr_data(0x0B);
        lcd_wr_reg(0xcc84);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xcc85);
        lcd_wr_data(0x25);
        lcd_wr_reg(0xcc86);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc87);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc88);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc89);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc90);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc91);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc92);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc93);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc94);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc95);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc96);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc97);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc98);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc99);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc9a);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcc9b);
        lcd_wr_data(0x26);
        lcd_wr_reg(0xcc9c);
        lcd_wr_data(0x0A);
        lcd_wr_reg(0xcc9d);
        lcd_wr_data(0x0C);
        lcd_wr_reg(0xcc9e);
        lcd_wr_data(0x02);
        lcd_wr_reg(0xcca0);
        lcd_wr_data(0x25);
        lcd_wr_reg(0xcca1);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcca2);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcca3);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcca4);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcca5);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcca6);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcca7);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcca8);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcca9);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccaa);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccab);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccac);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccad);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccae);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccb0);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccb1);
        lcd_wr_data(0x25);
        lcd_wr_reg(0xccb2);
        lcd_wr_data(0x0C);
        lcd_wr_reg(0xccb3);
        lcd_wr_data(0x0A);
        lcd_wr_reg(0xccb4);
        lcd_wr_data(0x02);
        lcd_wr_reg(0xccb5);
        lcd_wr_data(0x26);
        lcd_wr_reg(0xccb6);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccb7);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccb8);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccb9);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccc0);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccc1);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccc2);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccc3);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccc4);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccc5);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccc6);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccc7);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccc8);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccc9);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccca);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xcccb);
        lcd_wr_data(0x25);
        lcd_wr_reg(0xcccc);
        lcd_wr_data(0x0B);
        lcd_wr_reg(0xcccd);
        lcd_wr_data(0x09);
        lcd_wr_reg(0xccce);
        lcd_wr_data(0x01);
        lcd_wr_reg(0xccd0);
        lcd_wr_data(0x26);
        lcd_wr_reg(0xccd1);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccd2);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccd3);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccd4);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccd5);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccd6);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccd7);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccd8);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccd9);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccda);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccdb);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccdc);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccdd);
        lcd_wr_data(0x00);
        lcd_wr_reg(0xccde);
        lcd_wr_data(0x00);

        // Interface Pixel Format
        lcd_wr_reg(0x3A00);
        lcd_wr_data(0x55);

        // Sleep Out
        lcd_wr_reg(0x1100);
        delay_ms(100);

        // Display On
        lcd_wr_reg(0x2900);
        delay_ms(50);

        // Memory Write - This command is used to transfer data from MCU to frame memory
        lcd_wr_reg(0x2C00);

        // Column Address Set
        lcd_wr_reg(0x2A00);
        lcd_wr_data(0x00);
        lcd_wr_reg(0x2A01);
        lcd_wr_data(0x00);
        lcd_wr_reg(0x2A02);
        lcd_wr_data(0x01);
        lcd_wr_reg(0x2A03);
        lcd_wr_data(0xe0);

        // Page Address Set
        lcd_wr_reg(0x2B00);
        lcd_wr_data(0x00);
        lcd_wr_reg(0x2B01);
        lcd_wr_data(0x00);
        lcd_wr_reg(0x2B02);
        lcd_wr_data(0x03);
        lcd_wr_reg(0x2B03);
        lcd_wr_data(0x20);

    } else if (lcddev.id == 0x5510) {
        lcd_write_reg(0xF000, 0x55);
		lcd_write_reg(0xF001, 0xAA);
		lcd_write_reg(0xF002, 0x52);
		lcd_write_reg(0xF003, 0x08);
		lcd_write_reg(0xF004, 0x01);
		//AVDD Set AVDD 5.2V
		lcd_write_reg(0xB000, 0x0D);
		lcd_write_reg(0xB001, 0x0D);
		lcd_write_reg(0xB002, 0x0D);
		//AVDD ratio
		lcd_write_reg(0xB600, 0x34);
		lcd_write_reg(0xB601, 0x34);
		lcd_write_reg(0xB602, 0x34);
		//AVEE -5.2V
		lcd_write_reg(0xB100, 0x0D);
		lcd_write_reg(0xB101, 0x0D);
		lcd_write_reg(0xB102, 0x0D);
		//AVEE ratio
		lcd_write_reg(0xB700, 0x34);
		lcd_write_reg(0xB701, 0x34);
		lcd_write_reg(0xB702, 0x34);
		//VCL -2.5V
		lcd_write_reg(0xB200, 0x00);
		lcd_write_reg(0xB201, 0x00);
		lcd_write_reg(0xB202, 0x00);
		//VCL ratio
		lcd_write_reg(0xB800, 0x24);
		lcd_write_reg(0xB801, 0x24);
		lcd_write_reg(0xB802, 0x24);
		//VGH 15V (Free pump)
		lcd_write_reg(0xBF00, 0x01);
		lcd_write_reg(0xB300, 0x0F);
		lcd_write_reg(0xB301, 0x0F);
		lcd_write_reg(0xB302, 0x0F);
		//VGH ratio
		lcd_write_reg(0xB900, 0x34);
		lcd_write_reg(0xB901, 0x34);
		lcd_write_reg(0xB902, 0x34);
		//VGL_REG -10V
		lcd_write_reg(0xB500, 0x08);
		lcd_write_reg(0xB501, 0x08);
		lcd_write_reg(0xB502, 0x08);
		lcd_write_reg(0xC200, 0x03);
		//VGLX ratio
		lcd_write_reg(0xBA00, 0x24);
		lcd_write_reg(0xBA01, 0x24);
		lcd_write_reg(0xBA02, 0x24);
		//VGMP/VGSP 4.5V/0V
		lcd_write_reg(0xBC00, 0x00);
		lcd_write_reg(0xBC01, 0x78);
		lcd_write_reg(0xBC02, 0x00);
		//VGMN/VGSN -4.5V/0V
		lcd_write_reg(0xBD00, 0x00);
		lcd_write_reg(0xBD01, 0x78);
		lcd_write_reg(0xBD02, 0x00);
		//VCOM
		lcd_write_reg(0xBE00, 0x00);
		lcd_write_reg(0xBE01, 0x64);
		//Gamma Setting
		lcd_write_reg(0xD100, 0x00);
		lcd_write_reg(0xD101, 0x33);
		lcd_write_reg(0xD102, 0x00);
		lcd_write_reg(0xD103, 0x34);
		lcd_write_reg(0xD104, 0x00);
		lcd_write_reg(0xD105, 0x3A);
		lcd_write_reg(0xD106, 0x00);
		lcd_write_reg(0xD107, 0x4A);
		lcd_write_reg(0xD108, 0x00);
		lcd_write_reg(0xD109, 0x5C);
		lcd_write_reg(0xD10A, 0x00);
		lcd_write_reg(0xD10B, 0x81);
		lcd_write_reg(0xD10C, 0x00);
		lcd_write_reg(0xD10D, 0xA6);
		lcd_write_reg(0xD10E, 0x00);
		lcd_write_reg(0xD10F, 0xE5);
		lcd_write_reg(0xD110, 0x01);
		lcd_write_reg(0xD111, 0x13);
		lcd_write_reg(0xD112, 0x01);
		lcd_write_reg(0xD113, 0x54);
		lcd_write_reg(0xD114, 0x01);
		lcd_write_reg(0xD115, 0x82);
		lcd_write_reg(0xD116, 0x01);
		lcd_write_reg(0xD117, 0xCA);
		lcd_write_reg(0xD118, 0x02);
		lcd_write_reg(0xD119, 0x00);
		lcd_write_reg(0xD11A, 0x02);
		lcd_write_reg(0xD11B, 0x01);
		lcd_write_reg(0xD11C, 0x02);
		lcd_write_reg(0xD11D, 0x34);
		lcd_write_reg(0xD11E, 0x02);
		lcd_write_reg(0xD11F, 0x67);
		lcd_write_reg(0xD120, 0x02);
		lcd_write_reg(0xD121, 0x84);
		lcd_write_reg(0xD122, 0x02);
		lcd_write_reg(0xD123, 0xA4);
		lcd_write_reg(0xD124, 0x02);
		lcd_write_reg(0xD125, 0xB7);
		lcd_write_reg(0xD126, 0x02);
		lcd_write_reg(0xD127, 0xCF);
		lcd_write_reg(0xD128, 0x02);
		lcd_write_reg(0xD129, 0xDE);
		lcd_write_reg(0xD12A, 0x02);
		lcd_write_reg(0xD12B, 0xF2);
		lcd_write_reg(0xD12C, 0x02);
		lcd_write_reg(0xD12D, 0xFE);
		lcd_write_reg(0xD12E, 0x03);
		lcd_write_reg(0xD12F, 0x10);
		lcd_write_reg(0xD130, 0x03);
		lcd_write_reg(0xD131, 0x33);
		lcd_write_reg(0xD132, 0x03);
		lcd_write_reg(0xD133, 0x6D);
		lcd_write_reg(0xD200, 0x00);
		lcd_write_reg(0xD201, 0x33);
		lcd_write_reg(0xD202, 0x00);
		lcd_write_reg(0xD203, 0x34);
		lcd_write_reg(0xD204, 0x00);
		lcd_write_reg(0xD205, 0x3A);
		lcd_write_reg(0xD206, 0x00);
		lcd_write_reg(0xD207, 0x4A);
		lcd_write_reg(0xD208, 0x00);
		lcd_write_reg(0xD209, 0x5C);
		lcd_write_reg(0xD20A, 0x00);

		lcd_write_reg(0xD20B, 0x81);
		lcd_write_reg(0xD20C, 0x00);
		lcd_write_reg(0xD20D, 0xA6);
		lcd_write_reg(0xD20E, 0x00);
		lcd_write_reg(0xD20F, 0xE5);
		lcd_write_reg(0xD210, 0x01);
		lcd_write_reg(0xD211, 0x13);
		lcd_write_reg(0xD212, 0x01);
		lcd_write_reg(0xD213, 0x54);
		lcd_write_reg(0xD214, 0x01);
		lcd_write_reg(0xD215, 0x82);
		lcd_write_reg(0xD216, 0x01);
		lcd_write_reg(0xD217, 0xCA);
		lcd_write_reg(0xD218, 0x02);
		lcd_write_reg(0xD219, 0x00);
		lcd_write_reg(0xD21A, 0x02);
		lcd_write_reg(0xD21B, 0x01);
		lcd_write_reg(0xD21C, 0x02);
		lcd_write_reg(0xD21D, 0x34);
		lcd_write_reg(0xD21E, 0x02);
		lcd_write_reg(0xD21F, 0x67);
		lcd_write_reg(0xD220, 0x02);
		lcd_write_reg(0xD221, 0x84);
		lcd_write_reg(0xD222, 0x02);
		lcd_write_reg(0xD223, 0xA4);
		lcd_write_reg(0xD224, 0x02);
		lcd_write_reg(0xD225, 0xB7);
		lcd_write_reg(0xD226, 0x02);
		lcd_write_reg(0xD227, 0xCF);
		lcd_write_reg(0xD228, 0x02);
		lcd_write_reg(0xD229, 0xDE);
		lcd_write_reg(0xD22A, 0x02);
		lcd_write_reg(0xD22B, 0xF2);
		lcd_write_reg(0xD22C, 0x02);
		lcd_write_reg(0xD22D, 0xFE);
		lcd_write_reg(0xD22E, 0x03);
		lcd_write_reg(0xD22F, 0x10);
		lcd_write_reg(0xD230, 0x03);
		lcd_write_reg(0xD231, 0x33);
		lcd_write_reg(0xD232, 0x03);
		lcd_write_reg(0xD233, 0x6D);
		lcd_write_reg(0xD300, 0x00);
		lcd_write_reg(0xD301, 0x33);
		lcd_write_reg(0xD302, 0x00);
		lcd_write_reg(0xD303, 0x34);
		lcd_write_reg(0xD304, 0x00);
		lcd_write_reg(0xD305, 0x3A);
		lcd_write_reg(0xD306, 0x00);
		lcd_write_reg(0xD307, 0x4A);
		lcd_write_reg(0xD308, 0x00);
		lcd_write_reg(0xD309, 0x5C);
		lcd_write_reg(0xD30A, 0x00);

		lcd_write_reg(0xD30B, 0x81);
		lcd_write_reg(0xD30C, 0x00);
		lcd_write_reg(0xD30D, 0xA6);
		lcd_write_reg(0xD30E, 0x00);
		lcd_write_reg(0xD30F, 0xE5);
		lcd_write_reg(0xD310, 0x01);
		lcd_write_reg(0xD311, 0x13);
		lcd_write_reg(0xD312, 0x01);
		lcd_write_reg(0xD313, 0x54);
		lcd_write_reg(0xD314, 0x01);
		lcd_write_reg(0xD315, 0x82);
		lcd_write_reg(0xD316, 0x01);
		lcd_write_reg(0xD317, 0xCA);
		lcd_write_reg(0xD318, 0x02);
		lcd_write_reg(0xD319, 0x00);
		lcd_write_reg(0xD31A, 0x02);
		lcd_write_reg(0xD31B, 0x01);
		lcd_write_reg(0xD31C, 0x02);
		lcd_write_reg(0xD31D, 0x34);
		lcd_write_reg(0xD31E, 0x02);
		lcd_write_reg(0xD31F, 0x67);
		lcd_write_reg(0xD320, 0x02);
		lcd_write_reg(0xD321, 0x84);
		lcd_write_reg(0xD322, 0x02);
		lcd_write_reg(0xD323, 0xA4);
		lcd_write_reg(0xD324, 0x02);
		lcd_write_reg(0xD325, 0xB7);
		lcd_write_reg(0xD326, 0x02);
		lcd_write_reg(0xD327, 0xCF);
		lcd_write_reg(0xD328, 0x02);
		lcd_write_reg(0xD329, 0xDE);
		lcd_write_reg(0xD32A, 0x02);
		lcd_write_reg(0xD32B, 0xF2);
		lcd_write_reg(0xD32C, 0x02);
		lcd_write_reg(0xD32D, 0xFE);
		lcd_write_reg(0xD32E, 0x03);
		lcd_write_reg(0xD32F, 0x10);
		lcd_write_reg(0xD330, 0x03);
		lcd_write_reg(0xD331, 0x33);
		lcd_write_reg(0xD332, 0x03);
		lcd_write_reg(0xD333, 0x6D);
		lcd_write_reg(0xD400, 0x00);
		lcd_write_reg(0xD401, 0x33);
		lcd_write_reg(0xD402, 0x00);
		lcd_write_reg(0xD403, 0x34);
		lcd_write_reg(0xD404, 0x00);
		lcd_write_reg(0xD405, 0x3A);
		lcd_write_reg(0xD406, 0x00);
		lcd_write_reg(0xD407, 0x4A);
		lcd_write_reg(0xD408, 0x00);
		lcd_write_reg(0xD409, 0x5C);
		lcd_write_reg(0xD40A, 0x00);
		lcd_write_reg(0xD40B, 0x81);

		lcd_write_reg(0xD40C, 0x00);
		lcd_write_reg(0xD40D, 0xA6);
		lcd_write_reg(0xD40E, 0x00);
		lcd_write_reg(0xD40F, 0xE5);
		lcd_write_reg(0xD410, 0x01);
		lcd_write_reg(0xD411, 0x13);
		lcd_write_reg(0xD412, 0x01);
		lcd_write_reg(0xD413, 0x54);
		lcd_write_reg(0xD414, 0x01);
		lcd_write_reg(0xD415, 0x82);
		lcd_write_reg(0xD416, 0x01);
		lcd_write_reg(0xD417, 0xCA);
		lcd_write_reg(0xD418, 0x02);
		lcd_write_reg(0xD419, 0x00);
		lcd_write_reg(0xD41A, 0x02);
		lcd_write_reg(0xD41B, 0x01);
		lcd_write_reg(0xD41C, 0x02);
		lcd_write_reg(0xD41D, 0x34);
		lcd_write_reg(0xD41E, 0x02);
		lcd_write_reg(0xD41F, 0x67);
		lcd_write_reg(0xD420, 0x02);
		lcd_write_reg(0xD421, 0x84);
		lcd_write_reg(0xD422, 0x02);
		lcd_write_reg(0xD423, 0xA4);
		lcd_write_reg(0xD424, 0x02);
		lcd_write_reg(0xD425, 0xB7);
		lcd_write_reg(0xD426, 0x02);
		lcd_write_reg(0xD427, 0xCF);
		lcd_write_reg(0xD428, 0x02);
		lcd_write_reg(0xD429, 0xDE);
		lcd_write_reg(0xD42A, 0x02);
		lcd_write_reg(0xD42B, 0xF2);
		lcd_write_reg(0xD42C, 0x02);
		lcd_write_reg(0xD42D, 0xFE);
		lcd_write_reg(0xD42E, 0x03);
		lcd_write_reg(0xD42F, 0x10);
		lcd_write_reg(0xD430, 0x03);
		lcd_write_reg(0xD431, 0x33);
		lcd_write_reg(0xD432, 0x03);
		lcd_write_reg(0xD433, 0x6D);
		lcd_write_reg(0xD500, 0x00);
		lcd_write_reg(0xD501, 0x33);
		lcd_write_reg(0xD502, 0x00);
		lcd_write_reg(0xD503, 0x34);
		lcd_write_reg(0xD504, 0x00);
		lcd_write_reg(0xD505, 0x3A);
		lcd_write_reg(0xD506, 0x00);
		lcd_write_reg(0xD507, 0x4A);
		lcd_write_reg(0xD508, 0x00);
		lcd_write_reg(0xD509, 0x5C);
		lcd_write_reg(0xD50A, 0x00);
		lcd_write_reg(0xD50B, 0x81);

		lcd_write_reg(0xD50C, 0x00);
		lcd_write_reg(0xD50D, 0xA6);
		lcd_write_reg(0xD50E, 0x00);
		lcd_write_reg(0xD50F, 0xE5);
		lcd_write_reg(0xD510, 0x01);
		lcd_write_reg(0xD511, 0x13);
		lcd_write_reg(0xD512, 0x01);
		lcd_write_reg(0xD513, 0x54);
		lcd_write_reg(0xD514, 0x01);
		lcd_write_reg(0xD515, 0x82);
		lcd_write_reg(0xD516, 0x01);
		lcd_write_reg(0xD517, 0xCA);
		lcd_write_reg(0xD518, 0x02);
		lcd_write_reg(0xD519, 0x00);
		lcd_write_reg(0xD51A, 0x02);
		lcd_write_reg(0xD51B, 0x01);
		lcd_write_reg(0xD51C, 0x02);
		lcd_write_reg(0xD51D, 0x34);
		lcd_write_reg(0xD51E, 0x02);
		lcd_write_reg(0xD51F, 0x67);
		lcd_write_reg(0xD520, 0x02);
		lcd_write_reg(0xD521, 0x84);
		lcd_write_reg(0xD522, 0x02);
		lcd_write_reg(0xD523, 0xA4);
		lcd_write_reg(0xD524, 0x02);
		lcd_write_reg(0xD525, 0xB7);
		lcd_write_reg(0xD526, 0x02);
		lcd_write_reg(0xD527, 0xCF);
		lcd_write_reg(0xD528, 0x02);
		lcd_write_reg(0xD529, 0xDE);
		lcd_write_reg(0xD52A, 0x02);
		lcd_write_reg(0xD52B, 0xF2);
		lcd_write_reg(0xD52C, 0x02);
		lcd_write_reg(0xD52D, 0xFE);
		lcd_write_reg(0xD52E, 0x03);
		lcd_write_reg(0xD52F, 0x10);
		lcd_write_reg(0xD530, 0x03);
		lcd_write_reg(0xD531, 0x33);
		lcd_write_reg(0xD532, 0x03);
		lcd_write_reg(0xD533, 0x6D);
		lcd_write_reg(0xD600, 0x00);
		lcd_write_reg(0xD601, 0x33);
		lcd_write_reg(0xD602, 0x00);
		lcd_write_reg(0xD603, 0x34);
		lcd_write_reg(0xD604, 0x00);
		lcd_write_reg(0xD605, 0x3A);
		lcd_write_reg(0xD606, 0x00);
		lcd_write_reg(0xD607, 0x4A);
		lcd_write_reg(0xD608, 0x00);
		lcd_write_reg(0xD609, 0x5C);
		lcd_write_reg(0xD60A, 0x00);
		lcd_write_reg(0xD60B, 0x81);

		lcd_write_reg(0xD60C, 0x00);
		lcd_write_reg(0xD60D, 0xA6);
		lcd_write_reg(0xD60E, 0x00);
		lcd_write_reg(0xD60F, 0xE5);
		lcd_write_reg(0xD610, 0x01);
		lcd_write_reg(0xD611, 0x13);
		lcd_write_reg(0xD612, 0x01);
		lcd_write_reg(0xD613, 0x54);
		lcd_write_reg(0xD614, 0x01);
		lcd_write_reg(0xD615, 0x82);
		lcd_write_reg(0xD616, 0x01);
		lcd_write_reg(0xD617, 0xCA);
		lcd_write_reg(0xD618, 0x02);
		lcd_write_reg(0xD619, 0x00);
		lcd_write_reg(0xD61A, 0x02);
		lcd_write_reg(0xD61B, 0x01);
		lcd_write_reg(0xD61C, 0x02);
		lcd_write_reg(0xD61D, 0x34);
		lcd_write_reg(0xD61E, 0x02);
		lcd_write_reg(0xD61F, 0x67);
		lcd_write_reg(0xD620, 0x02);
		lcd_write_reg(0xD621, 0x84);
		lcd_write_reg(0xD622, 0x02);
		lcd_write_reg(0xD623, 0xA4);
		lcd_write_reg(0xD624, 0x02);
		lcd_write_reg(0xD625, 0xB7);
		lcd_write_reg(0xD626, 0x02);
		lcd_write_reg(0xD627, 0xCF);
		lcd_write_reg(0xD628, 0x02);
		lcd_write_reg(0xD629, 0xDE);
		lcd_write_reg(0xD62A, 0x02);
		lcd_write_reg(0xD62B, 0xF2);
		lcd_write_reg(0xD62C, 0x02);
		lcd_write_reg(0xD62D, 0xFE);
		lcd_write_reg(0xD62E, 0x03);
		lcd_write_reg(0xD62F, 0x10);
		lcd_write_reg(0xD630, 0x03);
		lcd_write_reg(0xD631, 0x33);
		lcd_write_reg(0xD632, 0x03);
		lcd_write_reg(0xD633, 0x6D);
		//LV2 Page 0 enable
		lcd_write_reg(0xF000, 0x55);
		lcd_write_reg(0xF001, 0xAA);
		lcd_write_reg(0xF002, 0x52);
		lcd_write_reg(0xF003, 0x08);
		lcd_write_reg(0xF004, 0x00);
		//Display control
		lcd_write_reg(0xB100, 0xCC);
		lcd_write_reg(0xB101, 0x00);
		//Source hold time
		lcd_write_reg(0xB600, 0x05);
		//Gate EQ control
		lcd_write_reg(0xB700, 0x70);
		lcd_write_reg(0xB701, 0x70);
		//Source EQ control (Mode 2)
		lcd_write_reg(0xB800, 0x01);
		lcd_write_reg(0xB801, 0x03);
		lcd_write_reg(0xB802, 0x03);
		lcd_write_reg(0xB803, 0x03);
		//Inversion mode (2-dot)
		lcd_write_reg(0xBC00, 0x02);
		lcd_write_reg(0xBC01, 0x00);
		lcd_write_reg(0xBC02, 0x00);
		//Timing control 4H w/ 4-delay
		lcd_write_reg(0xC900, 0xD0);
		lcd_write_reg(0xC901, 0x02);
		lcd_write_reg(0xC902, 0x50);
		lcd_write_reg(0xC903, 0x50);
		lcd_write_reg(0xC904, 0x50);
		lcd_write_reg(0x3500, 0x00);
		lcd_write_reg(0x3A00, 0x55);  //16-bit/pixel
		lcd_wr_reg(0x1100);
		delay_us(120);
		lcd_wr_reg(0x2900);
    }

    //初始化完成以后,提速
//    if (lcddev.id == 0X9341 || lcddev.id == 0x8009 || lcddev.id == 0x5510)   //如果是这几个IC,则设置WR时序为最快
//    {
//        //重新配置写时序控制寄存器的时序
//        FSMC_Bank1E->BWTR[6] &= ~(0XF << 0); //地址建立时间(ADDSET)清零
//        FSMC_Bank1E->BWTR[6] &= ~(0XF << 8); //数据保存时间清零
//        FSMC_Bank1E->BWTR[6] |= 1 << 0; //地址建立时间(ADDSET)为2个HCLK =28ns
//        FSMC_Bank1E->BWTR[6] |= 1 << 8; //数据保存时间(DATAST)为13.8ns*2个HCLK=28ns
//    }

    bsp_lcd_disp_dir(0);        //默认为竖屏
    LCD_BACKLIGHT_ON;           //点亮背光
    bsp_lcd_clear(BLACK);
}

/* @brief 用指令颜色清LCD屏
 * @param color rgb565颜色值
 */
void bsp_lcd_clear(uint32_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;
    totalpoint *= lcddev.height;        //得到总点数

    if (lcddev.id == 0x8009)
        bsp_lcd_set_window(0x00, 0x0000, lcddev.width, lcddev.height);  //设置光标位置
    else
        bsp_lcd_set_cursor(0x00, 0x0000);
    bsp_lcd_write_ram_prepare();            //开始写入GRAM

    for (index = 0; index < totalpoint; index++)
    {
        lcd->ram = color;
    }
}

/* @brief 给LCD指定区域填充rgb565颜色
 * @param sx 区域起始x坐标
 * @param sy 区域起始y坐标
 * @param ex 区域结束x坐标
 * @param ey 区域结束y坐标
 * @param color rgb565颜色值

 */
void bsp_lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color)
{
    uint32_t xlen = ex - sx + 1;
    uint32_t ylen = ey - sy + 1;
    uint32_t totalpoint = xlen * ylen;
    uint32_t j = 0;
    
    bsp_lcd_set_window(sx, sy, ex, ey);  //设置光标位置
    bsp_lcd_write_ram_prepare();

    for (j = 0; j < totalpoint; j++) 
        lcd->ram = color;
}

/* @brief 给LCD指定区域填充rgb565颜色
 * @param sx 区域起始x坐标
 * @param sy 区域起始y坐标
 * @param ex 区域结束x坐标
 * @param ey 区域结束y坐标
 * @param color rgb565颜色值

 */
void bsp_lcd_fill_pcolor(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t* pcolor)
{
    uint32_t xlen = ex - sx + 1;
    uint32_t ylen = ey - sy + 1;
    uint32_t i = 0, j = 0;
    
    for (i = 0; i < ylen; i++) {
        bsp_lcd_set_cursor(sx, sy + i);
        bsp_lcd_write_ram_prepare();
        for (j = 0; j < xlen; j++)
            lcd->ram = pcolor[i * xlen + j];//写入数据 
    }
}
