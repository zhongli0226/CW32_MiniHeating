/*
 * @Description: 温度控制函数，包括pid算法等
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-08-13 13:15:54
 * @LastEditors: tangwc
 * @LastEditTime: 2023-09-02 23:30:27
 * @FilePath: \2.Firmware\User\Temp_control\temp_control.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <stdio.h>

#include "cw32f030_atim.h"

#include "max6675.h"
#include "temp_control.h"
#include "elog.h"

#define TAG "Temp_control"

#define Temp_step 5       // 温度步进
#define Temp_upper 420    // 温度上限
#define Temp_lower 100    // 温度下限
#define Temp_pwm_high 999 // pwm上限
#define Temp_pwm_low 0    // pwm下限

typedef struct
{
    uint16_t target_temp; // 目标温度
    uint16_t actual_temp; // 实际温度
    uint16_t set_pwm;     // 设置的pwm
} temp_types;

static temp_types heat_temp_parameter = {300, 0, 999}; // 初始化目标温度为300

/**
 * @description: 获得实际温度
 * @return {*}  uint16_t 光耦上实际温度
 */
uint16_t Get_actual_temp(void)
{
    heat_temp_parameter.actual_temp = MAX6675_Read();
    return heat_temp_parameter.actual_temp;
}

/**
 * @description: 获得目标温度
 * @return {*}  uint16_t 内部设置要达到的温度
 */
uint16_t Get_target_temp(void)
{
    return heat_temp_parameter.target_temp;
}

/**
 * @description: 设定温度向上提高一个步进
 * @return {*}
 */
void target_temp_step_up(void)
{
    heat_temp_parameter.target_temp += Temp_step;
    if (heat_temp_parameter.target_temp > Temp_upper)
    {
        heat_temp_parameter.target_temp = Temp_upper;
    }
}

/**
 * @description: 设定温度向下降低一个步进
 * @return {*}
 */
void target_temp_step_down(void)
{
    heat_temp_parameter.target_temp -= Temp_step;
    if (heat_temp_parameter.target_temp < Temp_lower)
    {
        heat_temp_parameter.target_temp = Temp_lower;
    }
}
/**
 * @description: 获得pwm百分比
 * @return {*}
 */
uint16_t GET_pwm_prop(void)
{
    uint16_t pwm_prop = (heat_temp_parameter.set_pwm * 100) / 999;
    return pwm_prop;
}