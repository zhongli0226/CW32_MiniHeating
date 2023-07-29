/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2023-07-28 15:52:46
 * @LastEditors: tangwc
 * @LastEditTime: 2023-07-29 14:56:29
 * @FilePath: \2.Firmware\Core\src\gpio.c
 * 
 *  Copyright (c) 2023 by tangwc, All Rights Reserved. 
 */
#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"
#include "gpio.h"
#include "elog.h"



#define TAG "GPIO"
void GPIO_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 使能GPIO的配置时钟  
    __RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.IT = GPIO_IT_RISING | GPIO_IT_FALLING;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pins = GPIO_PIN_3;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_Init(CW_GPIOB, &GPIO_InitStruct);
    // 清除PB中断标记并使能NVIC
    GPIOB_INTFLAG_CLR(bv3);
    NVIC_EnableIRQ(GPIOB_IRQn);
    elog_i(TAG,"GPIOB PIN3 init");
}


void GPIOB_IRQHandlerCallBack(void)
{
    if (CW_GPIOB->ISR_f.PIN3)
    {
        elog_i(TAG,"GPIOB PIN3 IT");
        GPIOB_INTFLAG_CLR(bv3);
    }
}