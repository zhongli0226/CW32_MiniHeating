/*
 * @Description: gui 刷新函数
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-08-12 15:21:09
 * @LastEditors: tangwc
 * @LastEditTime: 2023-09-09 21:44:57
 * @FilePath: \2.Firmware\User\GUI\user_gui.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <stdlib.h>
#include <stdio.h>

#include "cw32f030_atim.h"
#include "system_cw32f030.h"

#include "oled.h"
#include "oled_gui.h"
#include "user_gui.h"
#include "user_font.h"

#include "adc.h"
#include "elog.h"
#include "max6675.h"

#define TAG "USER_GUI"

#define Temp_step 5               // 温度步进
#define Temp_upper 420            // 温度上限
#define Temp_lower 100            // 温度下限
#define Temp_demarcation_line 800 // 分界线
#define Temp_pwm_high 999         // pwm上限
#define Temp_pwm_low 0            // pwm下限

typedef enum
{
    PROCESS_INIT_UI = 0,
    PROCESS_MAIN_UI,
    PROCESS_MENU_UI,
} ui_process_type_t;

typedef struct
{
    uint16_t target_temp; // 目标温度
    uint16_t actual_temp; // 实际温度
    int16_t set_pwm;      // 设置的pwm
} temp_type_t;

typedef struct
{
    uint8_t refresh_actual_temp; // 刷新实际温度
    uint8_t refresh_voltage;     // 刷新电源电压
    uint8_t volt_err;            // 电源电压错误
} gui_flag_type_t;

static ui_process_type_t main_ui_process = PROCESS_INIT_UI; // gui 流程状态

static gui_flag_type_t gui_flag_t = {0}; // gui 相关标记

static temp_type_t heat_temp_parameter = {300, 0, 999}; // 初始化目标温度为300

static int32_t disapper_temp = 0;
static int32_t come_temp = 8;
/**
 * @description:  logo 消失接口
 * @return {*} 0：正在消失，1：消失完成
 */
static uint32_t ui_disapper(void)
{
    uint32_t len = sizeof(MiniHeating_logo) / sizeof(MiniHeating_logo[0]);
    uint8_t *p = Get_OLEDBuffer();
    uint8_t return_flag = 0;
    for (uint32_t i = 0; i < len; i++)
    {
        p[i] = p[i] & (rand() % 0xff) >> disapper_temp; // rand()%0xff = 0 ~ 0xff
    }
    disapper_temp += 2;
    if (disapper_temp > 8)
    {
        return_flag = 1;
        disapper_temp = 0;
    }
    return return_flag ? 0 : 1;
}

/**
 * @description:  logo 显示接口
 * @return {*}0：正在显示，1：显示完成
 */
static uint32_t ui_come(void)
{
    uint32_t len = sizeof(MiniHeating_logo) / sizeof(MiniHeating_logo[0]);
    uint8_t *p = Get_OLEDBuffer();
    uint8_t return_flag = 0;
    for (uint32_t i = 0; i < len; i++)
    {
        p[i] = p[i] & (rand() % 0xff) >> come_temp; // rand()%0xff = 0 ~ 0xff
    }
    come_temp -= 2;
    if (come_temp < 0)
    {
        return_flag = 1;
        come_temp = 8;
    }
    return return_flag ? 0 : 1;
}

static uint8_t disapper_flag = 0;
static uint8_t come_flag = 0;

/**
 * @description: 初始logo过场动画
 * @return {*} 0：动画运行中，1：动画运行完成
 */
static uint8_t Transitions_logo(void)
{
    uint8_t anim_state = 0;
    OLED_Clear();                                                    // 清除内部缓冲区
    GUI_ShowBMP(0, 0, OLED_WIDTH, OLED_HEIGHT, MiniHeating_logo, 0); // 第一段输出位置
    if (disapper_flag == 1)
    {
        if (ui_disapper() == 0)
        {
            disapper_flag = 0;
            anim_state = 1;
            elog_i(TAG, "Transition animation completion......");
        }
    }
    else
    {
        if (ui_come() == 0)
        {
            disapper_flag = 1;
            come_flag = 1;
            GUI_ShowBMP(0, 0, OLED_WIDTH, OLED_HEIGHT, MiniHeating_logo, 0);
        }
    }
    OLED_Display(); // transfer internal memory to the display
    if (come_flag == 1)
    {
        come_flag = 0;
        delay1ms(2000);
    }
    delay1ms(100);
    return anim_state;
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
 * @description: 刷新目标温度
 * @return {*}
 */
static void refresh_target_temp(void)
{
    uint16_t target_temp = heat_temp_parameter.target_temp;
    GUI_ShowNum(3, 53, target_temp, 3, 12, 0);
}

/**
 * @description: 刷新实际温度
 * @return {*}
 */
void set_actual_temp_flag(void)
{
    gui_flag_t.refresh_actual_temp = 1;
}

static void refresh_actual_temp(void)
{
    if (!gui_flag_t.volt_err)
    {
        if (gui_flag_t.refresh_actual_temp)
        {
            heat_temp_parameter.actual_temp = MAX6675_Read();
            GUI_ShowNum(0, 12, heat_temp_parameter.actual_temp, 3, 40, 0);
            gui_flag_t.refresh_actual_temp = 0;
        }
    }
    else
    {
        // 电压异常显示错误
        if (gui_flag_t.refresh_actual_temp)
        {
            GUI_ShowString(0, 12, "ERR", 40, 0);
            gui_flag_t.refresh_actual_temp = 0;
        }
    }
}
/**
 * @description: 刷新电源电压
 * @return {*}
 */
void set_pwr_volt_flag(void)
{
    gui_flag_t.refresh_voltage = 1;
}
static void refresh_pwr_Voltage(void)
{
    if (gui_flag_t.refresh_voltage)
    {
        float Voltage = GET_PWR_Value();
        // elog_i(TAG, "pwr:%d", (uint16_t)Voltage);
        if (Voltage < 20)
        {
            // 检测到电压异常
            gui_flag_t.volt_err = 1;
        }
        else
        {
            gui_flag_t.volt_err = 0;
        }
        uint16_t Voltage_int = (uint16_t)Voltage;
        uint16_t Voltage_float = (uint16_t)((Voltage_int * 10) % 10);
        GUI_ShowNum(98, 0, Voltage_int, 2, 12, 1);
        GUI_ShowChar(110, 0, '.', 12, 1);
        GUI_ShowNum(116, 0, Voltage_float, 1, 12, 1);
        GUI_ShowChar(122, 0, 'V', 12, 1);
        gui_flag_t.refresh_voltage = 0;
    }
}

/**
 * @description: 调整和显示功率占比
 * @return {*}
 */
static void refresh_pwm_prop(void)
{
    // 上下限处理
    if (gui_flag_t.volt_err)
    {
        // 出现异常不加热
        heat_temp_parameter.set_pwm = 0;
    }
    if (heat_temp_parameter.set_pwm > Temp_pwm_high)
    {
        heat_temp_parameter.set_pwm = Temp_pwm_high;
    }
    else if (heat_temp_parameter.set_pwm < Temp_pwm_low)
    {
        heat_temp_parameter.set_pwm = Temp_pwm_low;
    }
    // 调整pwm
    ATIM_SetCompare1B(heat_temp_parameter.set_pwm);
    uint16_t pwm_prop = (heat_temp_parameter.set_pwm * 100) / 999;
    GUI_ShowNum(106, 53, pwm_prop, 3, 12, 0);
    GUI_ShowChar(123, 53, '%', 12, 0);
}

/**
 * @description: 显示logo
 * @return {*}
 */
static void refresh_logo_type(void)
{
    if (gui_flag_t.volt_err)
    {
        GUI_ShowBMP(65, 27, 24, 24, err_logo, 0);
        GUI_ShowBMP(90, 36, 28, 14, err_text, 0);
    }
    else if (heat_temp_parameter.set_pwm > Temp_demarcation_line)
    {
        GUI_ShowBMP(65, 27, 24, 24, rise_temp_logo, 0);
        GUI_ShowBMP(90, 36, 28, 14, rise_temp_text, 0);
    }
    else
    {
        GUI_ShowBMP(65, 27, 24, 24, const_temp_logo, 0);
        GUI_ShowBMP(90, 36, 28, 14, const_temp_text, 0);
    }
}

/**
 * @description: ui 流程
 * @return {*}
 */
void UI_Main_Process(void)
{
    uint8_t ret = 0;
    switch (main_ui_process)
    {
    case PROCESS_INIT_UI:
        /* 初始化过场动画流程 */
        ret = Transitions_logo();
        if (ret)
        {
            GUI_ShowBMP(0, 0, OLED_WIDTH, OLED_HEIGHT, main_ui_bg, 0);
            main_ui_process = PROCESS_MAIN_UI;
        }
        break;
    case PROCESS_MAIN_UI:
        /* 主界面ui流程 */
        refresh_target_temp(); // 刷新目标温度显示    
        refresh_pwr_Voltage(); // 刷新供电电压显示
        refresh_actual_temp(); // 刷新实际温度显示
        refresh_pwm_prop();    // 刷新pwm占比显示
        refresh_logo_type();// 刷新logo显示
        break;
    case PROCESS_MENU_UI:
        /* code */
        break;
    default:
        break;
    }
    OLED_Display();
}
