/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2023-07-28 14:07:25
 * @LastEditors: tangwc
 * @LastEditTime: 2023-07-29 14:30:35
 * @FilePath: \2.Firmware\Core\src\adc.c
 * 
 *  Copyright (c) 2023 by tangwc, All Rights Reserved. 
 */
#include "cw32f030_adc.h"
#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"
#include "adc.h"
#include "elog.h"

#define TAG "adc"

static uint16_t valueAdc;

void ADC_init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    ADC_WdtTypeDef ADC_WdtStructure;
    ADC_SingleChTypeDef ADC_SingleChStructure;

    /* Clocks Configuration */
    __RCC_ADC_CLK_ENABLE();
    __RCC_GPIOA_CLK_ENABLE();

    //set PA00 as AIN0 INPUT
    PA00_ANALOG_ENABLE();
    
    //ADC默认值初始化
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_ClkDiv = ADC_Clk_Div128; // pclk = 64mhz 500khz

    //ADC模拟看门狗通道初始化
    ADC_WdtInit(&ADC_WdtStructure);

    //配置单通道转换模式
    ADC_SingleChStructure.ADC_DiscardEn = ADC_DiscardNull;
    ADC_SingleChStructure.ADC_Chmux = ADC_ExInputCH0;    //选择ADC转换通道，AIN0:PA00
    ADC_SingleChStructure.ADC_InitStruct = ADC_InitStructure;
    ADC_SingleChStructure.ADC_WdtStruct = ADC_WdtStructure;

    /*!< 单输入通道连续采样模式 */
    ADC_SingleChOneModeCfg(&ADC_SingleChStructure);
    ADC_ITConfig(ADC_IT_EOC, ENABLE);

    ADC_EnableIrq(ADC_INT_PRIORITY);
    ADC_ClearITPendingAll();

    //ADC使能
    ADC_Enable();
    ADC_SoftwareStartConvCmd(ENABLE);
    elog_i(TAG,"ADC INIT.....");
}


void ADC_IRQHandlerCallBack(void)
{
    if (ADC_GetITStatus(ADC_IT_EOC))
    {   
        valueAdc = ADC_GetConversionValue();
        elog_i(TAG,"get ADC value: %d",valueAdc);
        // ADC_SoftwareStartConvCmd(ENABLE);	//启动下一次adc转换
        ADC_ClearITPendingBit(ADC_IT_EOC);
    }
}

