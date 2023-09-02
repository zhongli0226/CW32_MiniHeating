/*
 * @Description:
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-08-12 15:21:09
 * @LastEditors: tangwc
 * @LastEditTime: 2023-09-02 13:30:55
 * @FilePath: \2.Firmware\User\GUI\user_gui.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <stdlib.h>

#include "oled.h"
#include "oled_gui.h"
#include "user_gui.h"
#include "user_font.h"
#include "temp_control.h"
#include "system_cw32f030.h"

#include "adc.h"

#include "elog.h"

#define TAG "USER_GUI"

/**
 * @description: 刷新全屏背景图片
 * @param {uint8_t*} bg 背景图片地址
 * @param {uint8_t} mode 0：白色背景和黑色字符   1：黑色背景和白色字符
 * @return {*}
 */
static void Show_fullscreen_bg(const uint8_t *bg, uint8_t mode)
{
    uint8_t temp, t1;
    uint16_t j, i;
    uint8_t y = 0, y0 = 0, x = 0;

    i = (OLED_WIDTH / 2) * (OLED_HEIGHT / 4);

    for (j = 0; j < i; j++)
    {
        temp = bg[j]; // 调用图片
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                GUI_DrawPoint(x, y, mode);
            else
                GUI_DrawPoint(x, y, !mode);
            temp <<= 1;
            y++;
            if ((y - y0) == OLED_HEIGHT)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

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
uint8_t Transitions_logo(void)
{
    uint8_t anim_state = 0;
    OLED_Clear();                            // 清除内部缓冲区
    Show_fullscreen_bg(MiniHeating_logo, 0); // 第一段输出位置
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
            Show_fullscreen_bg(MiniHeating_logo, 0);
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
 * @description: 主界面背景显示
 * @return {*}
 */
void main_gui_show(void)
{
    Show_fullscreen_bg(main_ui_bg, 0);
}

/**
 * @description: 刷新目标温度
 * @return {*}
 */
void refresh_target_temp(void)
{
    uint16_t target_temp = Get_target_temp();
    GUI_ShowNum(3, 53, target_temp, 3, 12, 0);
}
/**
 * @description: 刷新电源电压
 * @return {*}
 */
void refresh_pwr_Voltage(void)
{
    float Voltage = GET_PWR_Value();
    elog_i(TAG, "pwr:%f", Voltage);

    uint16_t Voltage_int = (uint16_t)Voltage;
    uint16_t Voltage_float = (uint16_t)((Voltage_int * 10) % 10);
    GUI_ShowNum(98, 0, Voltage_int, 2, 12, 1);
    GUI_ShowChar(110,0,'.',12,1); 
    GUI_ShowNum(116, 0, Voltage_float, 1, 12, 1);
    GUI_ShowChar(122,0,'V',12,1); 
}
