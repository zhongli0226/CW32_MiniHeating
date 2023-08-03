#include "cw32f030_btim.h"
#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"

#include "btimer.h"
#include "elog.h"

#define TAG "btimer"

void BTIMER_init(void)
{
    BTIM_TimeBaseInitTypeDef BTIM_TimeBaseInitStruct;
    /* System Clocks Configuration */
    __RCC_BTIM_CLK_ENABLE();

    /* NVIC Configuration */
    NVIC_EnableIRQ(BTIM1_IRQn);

    BTIM_TimeBaseInitStruct.BTIM_Mode = BTIM_Mode_TIMER;
    BTIM_TimeBaseInitStruct.BTIM_Period = 4999;//5ms 进一次中断
    BTIM_TimeBaseInitStruct.BTIM_Prescaler = BTIM_PRS_DIV64;// pclk  64MHz 1mhz进行计数

    BTIM_TimeBaseInit(CW_BTIM1, &BTIM_TimeBaseInitStruct);
    BTIM_ITConfig(CW_BTIM1, BTIM_IT_OV, ENABLE);
    BTIM_Cmd(CW_BTIM1, ENABLE);
}