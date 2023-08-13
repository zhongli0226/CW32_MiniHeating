/*
 * @Description:
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-08-13 13:15:54
 * @LastEditors: tangwc
 * @LastEditTime: 2023-08-13 14:08:54
 * @FilePath: \2.Firmware\User\Temp_control\temp_control.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <stdio.h>

#include "max6675.h"
#include "temp_control.h"

#include "elog.h"

#define TAG "Temp_control"

#define Temp_step 5 // 温度步进
#define Temp_upper 420 // 温度上限
#define Temp_lower 100 // 温度下限

typedef struct
{
    uint16_t target_temp; // 目标温度
    uint16_t actual_temp; // 实际温度
} temp_types;

static temp_types heat_temp = {300, 0};//初始化目标温度为300

/**
 * @description: 获得实际温度
 * @return {*}  uint16_t 光耦上实际温度
 */
uint16_t Get_actual_temp(void)
{
    heat_temp.actual_temp = MAX6675_Read();
    return heat_temp.actual_temp;
}

/**
 * @description: 获得目标温度
 * @return {*}  uint16_t 内部设置要达到的温度
 */
uint16_t Get_target_temp(void)
{
    return heat_temp.target_temp;
}

/**
 * @description: 设定温度向上提高一个步进
 * @return {*}
 */
void target_temp_step_up(void)
{
    heat_temp.target_temp += Temp_step;
    if(heat_temp.target_temp > Temp_upper)
    {
        heat_temp.target_temp = Temp_upper;
    }
}

/**
 * @description: 设定温度向下降低一个步进
 * @return {*}
 */
void target_temp_step_down(void)
{
    heat_temp.target_temp -= Temp_step;
    if(heat_temp.target_temp < Temp_lower)
    {
        heat_temp.target_temp = Temp_lower;
    }
}