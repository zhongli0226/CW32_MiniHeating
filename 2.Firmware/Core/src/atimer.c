#include "cw32f030_atim.h"
#include "cw32f030_gpio.h"
#include "atimer.h"
#include "elog.h"


#define TAG "atimer"
//==================================================================================================
//  实现功能: 高级定时器
//  函数说明: ATIMER_init
//  函数备注:
//--------------------------------------------------------------------------------------------------
//  |   -   |   -   |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9
//==================================================================================================

void ATIMER_init(void)
{
    ATIM_InitTypeDef ATIM_InitStruct;
    ATIM_OCInitTypeDef ATIM_OCInitStruct;

    /* GPIO Configuration */
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.IT = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pins = GPIO_PIN_13;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

    PB13_AFx_ATIMCH1B();

    GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

    /* NVIC Configuration */
    __disable_irq();
    NVIC_EnableIRQ(ATIM_IRQn);
    __enable_irq();

    elog_i(TAG, "ATIM Initial...");
    
    ATIM_InitStruct.BufferState = DISABLE;
    ATIM_InitStruct.ClockSelect = ATIM_CLOCK_PCLK;
    ATIM_InitStruct.CounterAlignedMode = ATIM_COUNT_MODE_EDGE_ALIGN;
    ATIM_InitStruct.CounterDirection = ATIM_COUNTING_UP;
    ATIM_InitStruct.CounterOPMode = ATIM_OP_MODE_REPETITIVE;
    ATIM_InitStruct.OverFlowMask = DISABLE;
    ATIM_InitStruct.Prescaler = ATIM_Prescaler_DIV8; // 计数时钟1MHz
    ATIM_InitStruct.ReloadValue = 9999;              // 溢出周期10ms
    ATIM_InitStruct.RepetitionCounter = 0;
    ATIM_InitStruct.UnderFlowMask = DISABLE;

    ATIM_Init(&ATIM_InitStruct);

    ATIM_OCInitStruct.BufferState = ENABLE;
    ATIM_OCInitStruct.OCDMAState = DISABLE;
    ATIM_OCInitStruct.OCInterruptSelect = ATIM_OC_IT_UP_COUNTER;
    ATIM_OCInitStruct.OCInterruptState = ENABLE;
    ATIM_OCInitStruct.OCMode = ATIM_OCMODE_PWM1;
    ATIM_OCInitStruct.OCPolarity = ATIM_OCPOLARITY_NONINVERT;
    ATIM_OC1BInit(&ATIM_OCInitStruct);

    ATIM_ITConfig(ATIM_CR_IT_OVE, ENABLE);
    ATIM_CH1Config(ATIM_CHxB_CIE, ENABLE);
    ATIM_SetCompare1B(500);
    ATIM_Cmd(ENABLE);
    ATIM_CtrlPWMOutputs(ENABLE);
    elog_i(TAG,"ATIM is running.");
}