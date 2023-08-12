/*
 * @Description: oled 底层驱动库
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-08-03 08:53:51
 * @LastEditors: tangwc
 * @LastEditTime: 2023-08-12 15:43:54
 * @FilePath: \2.Firmware\User\OLED\oled.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <string.h>
#include "oled.h"

#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"
#include "system_cw32f030.h"
#include "elog.h"

#define TAG "OLED"

//-----------------OLED端口定义----------------
#define OLED_CLK_GPIO GPIO_PIN_5
#define OLED_MOSI_GPIO GPIO_PIN_6
#define OLED_RST_GPIO GPIO_PIN_7
#define OLED_DC_GPIO GPIO_PIN_8
#define OLED_CS_GPIO GPIO_PIN_9

//-----------------OLED端口操作----------------
#define OLED_CLK_CLR GPIO_WritePin(CW_GPIOB, OLED_CLK_GPIO, GPIO_Pin_RESET)
#define OLED_CLK_SET GPIO_WritePin(CW_GPIOB, OLED_CLK_GPIO, GPIO_Pin_SET)

#define OLED_MOSI_CLR GPIO_WritePin(CW_GPIOB, OLED_MOSI_GPIO, GPIO_Pin_RESET)
#define OLED_MOSI_SET GPIO_WritePin(CW_GPIOB, OLED_MOSI_GPIO, GPIO_Pin_SET)

#define OLED_RST_Clr GPIO_WritePin(CW_GPIOB, OLED_RST_GPIO, GPIO_Pin_RESET)
#define OLED_RST_Set GPIO_WritePin(CW_GPIOB, OLED_RST_GPIO, GPIO_Pin_SET)

#define OLED_DC_Clr GPIO_WritePin(CW_GPIOB, OLED_DC_GPIO, GPIO_Pin_RESET)
#define OLED_DC_Set GPIO_WritePin(CW_GPIOB, OLED_DC_GPIO, GPIO_Pin_SET)

#define OLED_CS_Clr GPIO_WritePin(CW_GPIOB, OLED_CS_GPIO, GPIO_Pin_RESET)
#define OLED_CS_Set GPIO_WritePin(CW_GPIOB, OLED_CS_GPIO, GPIO_Pin_SET)

//--------------OLED buff缓存区域---------------------
static uint8_t OLED_GRAM[OLED_WIDTH * (OLED_HEIGHT / 8)] = {0};

/**
 * @description: oled gpio 初始化
 * @return {*}
 */
static void OLED_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 使能GPIOB的配置时钟
    __RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.IT = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pins = OLED_CLK_GPIO | OLED_MOSI_GPIO | OLED_RST_GPIO | OLED_DC_GPIO | OLED_CS_GPIO;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

    elog_i(TAG, "OLED GPIO Init!");
}

/**
 * @description: 软件模拟spi写入
 * @param {uint8_t} Data: 要写入的数据
 * @return {*}
 */
static void SPI_WriteByte(uint8_t Data)
{
    uint8_t i = 0;
    for (i = 8; i > 0; i--)
    {
        if (Data & 0x80)
        {
            OLED_MOSI_SET; // 写数据1
        }
        else
        {
            OLED_MOSI_CLR; // 写数据0
        }
        OLED_CLK_CLR; // 将时钟拉低拉高
        OLED_CLK_SET; // 发送1bit数据
        Data <<= 1;
    }
}

/**
 * @description: 将一个字节写入OLED屏幕
 * @param {uint8_t} dat: 要编写的内容
 * @param {uint8_t} cmd: 0：写命令 1：写数据
 * @return {*}
 */
static void OLED_WR_Byte(uint8_t dat, uint8_t cmd)
{
    if (cmd)
    {
        OLED_DC_Set;
    }
    else
    {
        OLED_DC_Clr;
    }
    OLED_CS_Clr;
    SPI_WriteByte(dat);
    OLED_CS_Set;
}

/**
 * @description: 打开OLED显示屏
 * @return {*}
 */
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD); // SET DCDC命令
    OLED_WR_Byte(0X14, OLED_CMD); // DCDC ON
    OLED_WR_Byte(0XAF, OLED_CMD); // DISPLAY ON
}

/**
 * @description:  关闭OLED显示屏
 * @return {*}
 */
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD); // SET DCDC命令
    OLED_WR_Byte(0X10, OLED_CMD); // DCDC OFF
    OLED_WR_Byte(0XAE, OLED_CMD); // DISPLAY OFF
}

/**
 * @description:  将软件缓存区的内容更新到oled上显示
 * @return {*}
 */
void OLED_Display(void)
{
    uint8_t i, n;
    for (i = 0; i < OLED_HEIGHT / 8; i++)
    {
        OLED_WR_Byte(0xb0 + i, OLED_CMD); // 设置页地址（0~7）
        OLED_WR_Byte(0x00, OLED_CMD);     // 设置显示位置—列低地址
        OLED_WR_Byte(0x10, OLED_CMD);     // 设置显示位置—列高地址
        for (n = 0; n < OLED_WIDTH; n++)
            OLED_WR_Byte(OLED_GRAM[n + i * OLED_WIDTH], OLED_DATA);
    } // 更新显示
}

/**
 * @description:  清屏函数
 * @return {*}
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
        for (n = 0; n < 128; n++)
            OLED_GRAM[n + i * OLED_WIDTH] = 0x00;
    // OLED_Display(); // 更新显示
}

/**
 * @description: oled 复位函数
 * @return {*}
 */
void OLED_Reset(void)
{
    OLED_RST_Set;
    delay1ms(100);
    OLED_RST_Clr;
    delay1ms(100);
    OLED_RST_Set;
}

/**
 * @description: oled 初始化
 * @return {*}
 */
void OLED_Init(void)
{
    OLED_GPIO_Init();
    delay1ms(200);
    OLED_Reset(); // 复位OLED

    /**************初始化SSD1306*****************/
    OLED_WR_Byte(0xAE, OLED_CMD); // 关闭显示
    OLED_WR_Byte(0xD5, OLED_CMD); // 设置时钟分频因子,震荡频率
    OLED_WR_Byte(0x80, OLED_CMD); //[3:0],分频因子;[7:4],震荡频率
    OLED_WR_Byte(0xA8, OLED_CMD); // 设置驱动路数
    OLED_WR_Byte(0X3F, OLED_CMD); // 默认0X3F(1/64)
    OLED_WR_Byte(0xD3, OLED_CMD); // 设置显示偏移
    OLED_WR_Byte(0X00, OLED_CMD); // 默认为0

    OLED_WR_Byte(0x40, OLED_CMD); // 设置显示开始行 [5:0],行数.

    OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵设置
    OLED_WR_Byte(0x14, OLED_CMD); // bit2，开启/关闭
    OLED_WR_Byte(0x20, OLED_CMD); // 设置内存地址模式
    OLED_WR_Byte(0x02, OLED_CMD); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
    OLED_WR_Byte(0xA1, OLED_CMD); // 段重定义设置,bit0:0,0->0;1,0->127;
    OLED_WR_Byte(0xC0, OLED_CMD); // 设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
    OLED_WR_Byte(0xDA, OLED_CMD); // 设置COM硬件引脚配置
    OLED_WR_Byte(0x12, OLED_CMD); //[5:4]配置

    OLED_WR_Byte(0x81, OLED_CMD); // 对比度设置
    OLED_WR_Byte(0xEF, OLED_CMD); // 1~255;默认0X7F (亮度设置,越大越亮)
    OLED_WR_Byte(0xD9, OLED_CMD); // 设置预充电周期
    OLED_WR_Byte(0xf1, OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
    OLED_WR_Byte(0xDB, OLED_CMD); // 设置VCOMH 电压倍率
    OLED_WR_Byte(0x30, OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

    OLED_WR_Byte(0xA4, OLED_CMD); // 全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
    OLED_WR_Byte(0xA6, OLED_CMD); // 设置显示方式;bit0:1,反相显示;0,正常显示
    OLED_WR_Byte(0xAF, OLED_CMD); // 开启显示
    OLED_Clear();

    elog_i(TAG, "OLED Init!");
}

/**
 * @description: 获取缓存空间的地址
 * @return {*} 返回缓存空间起始地址
 */
uint8_t *Get_OLEDBuffer(void)
{
    return (uint8_t *)OLED_GRAM;
}
