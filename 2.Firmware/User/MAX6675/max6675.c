/*
 * @Description:
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-08-12 21:44:00
 * @LastEditors: tangwc
 * @LastEditTime: 2023-09-24 15:38:49
 * @FilePath: \2.Firmware\User\MAX6675\max6675.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <string.h>

#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"
#include "system_cw32f030.h"

#include "max6675.h"

#include "elog.h"

#define TAG "MAX6675"

//-----------------max6675端口定义----------------
#define MAX6675_CLK_GPIO GPIO_PIN_5  // PA5
#define MAX6675_MISO_GPIO GPIO_PIN_6 // PA6
#define MAX6675_CS_GPIO GPIO_PIN_7   // PA7

//-----------------OLED端口操作----------------
#define MAX6675_CLK_CLR() GPIO_WritePin(CW_GPIOA, MAX6675_CLK_GPIO, GPIO_Pin_RESET)
#define MAX6675_CLK_SET() GPIO_WritePin(CW_GPIOA, MAX6675_CLK_GPIO, GPIO_Pin_SET)

#define MAX6675_MISO_GET() GPIO_ReadPin(CW_GPIOA, MAX6675_MISO_GPIO)

#define MAX6675_CS_CLR() GPIO_WritePin(CW_GPIOA, MAX6675_CS_GPIO, GPIO_Pin_RESET)
#define MAX6675_CS_SET() GPIO_WritePin(CW_GPIOA, MAX6675_CS_GPIO, GPIO_Pin_SET)

/**
 * @description: MAX6675 gpio 初始化
 * @return {*}
 */
void MAX6675_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 使能GPIOA B的配置时钟
    __RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.IT = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pins = MAX6675_CLK_GPIO | MAX6675_CS_GPIO;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_Init(CW_GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.IT = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pins = MAX6675_MISO_GPIO;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_Init(CW_GPIOA, &GPIO_InitStruct);

    elog_i(TAG, "MAX6675 GPIO Init!");
}

/**
 * @description: 读取max6675 寄存器
 * @return {*}
 */
static uint16_t MAX6675_ReadReg(void)
{
    uint8_t i = 0;
    uint16_t dat = 0;

    MAX6675_CS_CLR(); // 置低，使能MAX6675
    MAX6675_CLK_CLR();

    for (i = 0; i < 16; i++)
    {
        MAX6675_CLK_SET();

        dat = dat << 1;
        if (MAX6675_MISO_GET() == GPIO_Pin_SET)
            dat = dat | 0x0001;
        MAX6675_CLK_CLR();
    }
    MAX6675_CS_SET(); // 关闭MAX6675

    return dat;
}


/**
 * @description: 读取max6675的温度
 * @param {uint16_t} *temp 整数部分温度
 * @return {*} 连接状态
 */
uint16_t MAX6675_Read(uint16_t *temp)
{
    uint16_t value = 0;
    uint16_t value_dec = 0;
    uint8_t connet_flag = 0;
    value = MAX6675_ReadReg(); // 读取寄存器

    // 读出数据的D2位是热电偶掉线标志位，该位为1表示掉线，该位为0表示连接
    connet_flag = value & 0x04;
    connet_flag = connet_flag >> 2;

    if (connet_flag)
    {
        elog_w(TAG, "max6675 is not connect");
        return 1;
    }
    else
    {
        value = value << 1; // 去掉第15位
        value = value >> 4; // 去掉第0~2位
        value_dec = value * 10;
        value = value / 4;         // MAX6675最大数值为1023.75，而AD精度为12位，即2的12次方为4096，转换对应数，故要除4；
        value_dec = value_dec / 4; // 小数 与上述同理
        if (value >= 1024)
            value = 1024;
    }

    *temp = value;
    // elog_i(TAG, "tempvalue:%d.%d", value, value_dec);
    return 0;
}