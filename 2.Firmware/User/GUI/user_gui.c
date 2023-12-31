/*
 * @Description: gui
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-08-12 15:21:09
 * @LastEditors: tangwc
 * @LastEditTime: 2023-10-05 09:29:54
 * @FilePath: \2.Firmware\User\GUI\user_gui.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cw32f030_atim.h"
#include "system_cw32f030.h"

#include "oled.h"
#include "oled_gui.h"
#include "user_gui.h"
#include "user_font.h"

#include "flash.h"
#include "adc.h"
#include "elog.h"
#include "max6675.h"

#define TAG "USER_GUI"

#define Temp_step 5               // 温度步进
#define Temp_upper 420            // 温度上限
#define Temp_lower 100            // 温度下限
#define Temp_demarcation_line 800 // pwm分界线
#define Temp_pwm_high 999         // pwm上限
#define Temp_pwm_low 0            // pwm下限

#define USER_FLASH_ADDR 0x0000FE00          // 用户配置FLASH地址
#define USER_CONFIG_SECTOR FLASH_Sector_127 // 用户配置FLSHA扇叶

#define Default_SET_TEMP 300 // 默认设定温度
#define Default_SET_PID_P 10 // 默认设定 P
#define Default_SET_PID_I 0  // 默认设定 I
#define Default_SET_PID_D 0  // 默认设定 D

#define MENU_FONT_NUM 16 // 菜单字体大小
typedef enum
{
    PROCESS_INIT_UI = 0,
    PROCESS_MAIN_UI,
    PROCESS_MENU_UI,
} ui_process_type_t;

typedef enum
{
    CONFIG_PID_MENU = 0,
    SHOW_VERSION_MENU,
    MENU_1,
    MENU_2,
    MENU_3,
} menu_mode_type_t;
// 主界面显示相关参数
typedef struct
{
    uint8_t refresh_actual_temp; // 刷新实际温度
    uint8_t refresh_voltage;     // 刷新电源电压
    uint16_t actual_temp;        // 实际温度
    uint8_t volt_err;            // 电压异常状态错误
    uint8_t max6675_err;         // max6675错误
    int16_t set_pwm;             // 设置的pwm
    uint8_t heating_flag;        // 加热标志
    uint8_t flash_update;        // FLASH刷新
} user_main_ui_type_t;

typedef struct
{
    menu_mode_type_t menu_mode;
    char *str;
    uint8_t len;
} menu_list_type_t;

// 菜单界面显示相关参数
typedef struct
{
    int8_t list_x_now;       // 当前x位置
    int8_t list_x_tag;       // 目标x位置
    int8_t list_y_now;       // 当前y位置
    int8_t list_y_tag;       // 目标y位置
    int8_t list_top_line;    // 记录界面头部位置
    int8_t list_bottom_line; // 记录界面底部位置
    int8_t frame_list_index; // 记录选择第几个菜单
    int8_t frame_list_line;  // 记录当前页面的位置 // 最大为 (OLED_HEIGHT / MENU_FONT_NUM-1)
} user_menu_ui_type_t;

// 温度控制参数
typedef struct
{
    uint16_t target_temp; // 目标温度
    union
    {
        float P;          // P值
        uint32_t P_value; //
    } union_P;
    union
    {
        float I;          // I值
        uint32_t I_value; //
    } union_I;
    union
    {
        float D;          // D值
        uint32_t D_value; //
    } union_D;

} offline_data_type_t;

typedef struct
{
    int16_t new_err;  // 新误差
    int16_t last_err; // 旧误差
    int16_t err_sum;  // 误差和
    int16_t pid_out;  // pid计算结果
} pid_temp_type_t;

static ui_process_type_t main_ui_process = PROCESS_INIT_UI; // gui 流程状态

static user_main_ui_type_t user_main_para = {0}; // 用户主界面参数初始化均为0

static offline_data_type_t control_temp_para = {0}; //  控制参数由flash初始化

static pid_temp_type_t temp_control_para = {0}; // 温度pid计算中间量

static uint8_t key_flag[NUM_FLAG_T] = {0}; // 按键事件flag

static const menu_list_type_t menu_list[] = {
    {CONFIG_PID_MENU, "SET PID", 8},
    {SHOW_VERSION_MENU, "Version", 8},
    {MENU_1, "To be add", 10},
    {MENU_2, "To be add", 10},
    {MENU_3, "To be add", 10},
};

static const uint32_t menu_list_len = sizeof(menu_list) / sizeof(menu_list_type_t); // 菜单列表长度

static user_menu_ui_type_t user_menu_para = {0, 0, 0, 0, 0, OLED_HEIGHT / MENU_FONT_NUM, 0, 0}; // 用户菜单界面参数

/**
 * @description: 设置key外部事件标志flag
 * @param {key_flag_type_t} flag:
 * @return {*}
 */
void set_key_flag(key_flag_type_t flag)
{
    if (flag >= NUM_FLAG_T)
    {
        elog_w(TAG, "set_key_flag error!");
        return;
    }
    key_flag[flag] = 1;
}

/**
 * @description:用户config 数据初始化
 * @return {*}
 */
static void user_parameter_init(void)
{
    elog_i(TAG, "user config data init!");
    Flash_read(USER_FLASH_ADDR, (uint8_t *)&control_temp_para, sizeof(control_temp_para));
    if (control_temp_para.target_temp == 0xFFFF)
    {
        // 刚出厂 默认为300
        control_temp_para.target_temp = Default_SET_TEMP;
        Flash_write(USER_FLASH_ADDR, (uint8_t *)&control_temp_para, sizeof(control_temp_para));
    }
    if (control_temp_para.union_P.P_value == 0xFFFFFFFF)
    {
        // 刚出厂 默认为1.5
        control_temp_para.union_P.P = Default_SET_PID_P;
        Flash_write(USER_FLASH_ADDR, (uint8_t *)&control_temp_para, sizeof(control_temp_para));
    }
    if (control_temp_para.union_I.I_value == 0xFFFFFFFF)
    {
        // 刚出厂 默认为1.5
        control_temp_para.union_I.I = Default_SET_PID_I;
        Flash_write(USER_FLASH_ADDR, (uint8_t *)&control_temp_para, sizeof(control_temp_para));
    }
    if (control_temp_para.union_D.D_value == 0xFFFFFFFF)
    {
        // 刚出厂 默认为1.5
        control_temp_para.union_D.D = Default_SET_PID_D;
        Flash_write(USER_FLASH_ADDR, (uint8_t *)&control_temp_para, sizeof(control_temp_para));
    }
    elog_i(TAG, "control_temp_para.target_temp = %d", control_temp_para.target_temp);
    elog_i(TAG, "P = %d,I = %d,D = %d", (uint8_t)(control_temp_para.union_P.P * 10), (uint8_t)(control_temp_para.union_I.I * 10), (uint8_t)(control_temp_para.union_D.D * 10));
}

static int32_t disapper_temp = 0;
static int32_t come_temp = 8;
/**
 * @description:  logo 消失接口
 * @return {*} 0：正在消失，1：消失完成
 */
static uint32_t logo_ui_disapper(void)
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
static uint32_t logo_ui_come(void)
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
        if (logo_ui_disapper() == 0)
        {
            disapper_flag = 0;
            anim_state = 1;
            elog_i(TAG, "Transition animation completion......");
        }
    }
    else
    {
        if (logo_ui_come() == 0)
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
 * @description: 实际温度刷新标记
 * @return {*}
 */
void set_actual_temp_flag(void)
{
    user_main_para.refresh_actual_temp = 1;
}

/**
 * @description: 电源电压刷新标记
 * @return {*}
 */
void set_pwr_volt_flag(void)
{
    user_main_para.refresh_voltage = 1;
}

/**
 * @description: flash 刷新标记
 * @return {*}
 */
void set_flash_update_flag(void)
{
    user_main_para.flash_update = 1;
}

/**
 * @description: 更新flash 记录
 * @return {*}
 */
static void updata_user_parameter(void)
{
    if (user_main_para.flash_update)
    {
        offline_data_type_t old_user_data;
        Flash_read(USER_FLASH_ADDR, (uint8_t *)&old_user_data, sizeof(old_user_data));
        if (old_user_data.target_temp != control_temp_para.target_temp)
        {
            Flash_erase(USER_CONFIG_SECTOR);
            Flash_write(USER_FLASH_ADDR, (uint8_t *)&control_temp_para, sizeof(control_temp_para));
        }
        user_main_para.flash_update = 0;
    }
}

/**
 * @description: 温度控制
 * @return {*}
 */
static void const_temp_control(void)
{
    uint16_t err_diff;
    float I_out;
    temp_control_para.new_err = control_temp_para.target_temp - user_main_para.actual_temp; // 误差
    err_diff = temp_control_para.new_err - temp_control_para.last_err;

    temp_control_para.err_sum += temp_control_para.new_err; // 偏差之和

    // 计算PID的比例和微分输出
    temp_control_para.pid_out = control_temp_para.union_P.P * temp_control_para.new_err + control_temp_para.union_D.D * err_diff;

    if (fabs(temp_control_para.new_err) < 5) // 如果温度相差小于3度则计入PID积分输出
    {

        if (temp_control_para.err_sum > 500)
            temp_control_para.err_sum = 500;
        if (temp_control_para.err_sum < -500)
            temp_control_para.err_sum = -500; // 防止积分过大

        I_out = control_temp_para.union_I.I * temp_control_para.err_sum; // 积分输出

        temp_control_para.pid_out += I_out;
    }
    else
    {
        temp_control_para.err_sum = 0;
    }

    if (temp_control_para.pid_out > 999)
        temp_control_para.pid_out = 999;
    if (temp_control_para.pid_out < 0)
        temp_control_para.pid_out = 0;

    if (temp_control_para.new_err > 10) // 温差超过10直接拉满
        temp_control_para.pid_out = 999;

    if (temp_control_para.new_err < 4) // 温差小于4 直接消除不调了
        temp_control_para.pid_out = 0;

    // 更新pwm
    user_main_para.set_pwm = temp_control_para.pid_out;
    // 温差更新
    temp_control_para.last_err = temp_control_para.new_err;
}

/**
 * @description: 主界面刷新和逻辑处理
 * @return {*}
 */
static void ui_main_refresh(void)
{
    // 刷新目标温度显示
    GUI_ShowNum(3, 53, control_temp_para.target_temp, 3, 12, 0);

    // 刷新供电电压显示
    if (user_main_para.refresh_voltage)
    {
        float Voltage = GET_PWR_Value();
        // elog_i(TAG, "pwr:%d", (uint16_t)Voltage);
        if (Voltage < 20)
        {
            // 检测到电压异常
            user_main_para.volt_err = 1;
        }
        else
        {
            user_main_para.volt_err = 0;
        }
        uint16_t Voltage_int = (uint16_t)Voltage;
        uint16_t Voltage_float = (uint16_t)((Voltage_int * 10) % 10);
        GUI_ShowNum(98, 0, Voltage_int, 2, 12, 1);
        GUI_ShowChar(110, 0, '.', 12, 1);
        GUI_ShowNum(116, 0, Voltage_float, 1, 12, 1);
        GUI_ShowChar(122, 0, 'V', 12, 1);
        user_main_para.refresh_voltage = 0;
    }

    // 刷新实际温度显示
    if (user_main_para.refresh_actual_temp)
    {
        if (MAX6675_Read(&user_main_para.actual_temp))
        {
            // max6675异常
            user_main_para.max6675_err = 1;
            GUI_ShowString(0, 12, "ERR", 40, 0);
        }
        else
        {
            user_main_para.max6675_err = 0;
            if (!user_main_para.volt_err)
            {
                GUI_ShowNum(0, 12, user_main_para.actual_temp, 3, 40, 0);
            }
            else
            {
                GUI_ShowString(0, 12, "ERR", 40, 0);
            }
        }
        user_main_para.refresh_actual_temp = 0;
    }

    // 温度控制pid
    const_temp_control();
    // 刷新pwm 和pwm占比显示
    if ((user_main_para.volt_err || user_main_para.max6675_err) || (!user_main_para.heating_flag)) // 异常或者加热标记未置起
    {
        // 不加热
        user_main_para.set_pwm = 0;
    }
    // 上下限处理
    if (user_main_para.set_pwm > Temp_pwm_high)
    {
        user_main_para.set_pwm = Temp_pwm_high;
    }
    else if (user_main_para.set_pwm < Temp_pwm_low)
    {
        user_main_para.set_pwm = Temp_pwm_low;
    }
    // 调整pwm
    ATIM_SetCompare1B(user_main_para.set_pwm);
    uint16_t pwm_prop = (user_main_para.set_pwm * 100) / 999;
    GUI_ShowNum(106, 53, pwm_prop, 3, 12, 0);
    GUI_ShowChar(123, 53, '%', 12, 0);

    // 刷新logo显示
    if ((user_main_para.volt_err || user_main_para.max6675_err))
    {
        GUI_ShowBMP(65, 27, 24, 24, err_logo, 0);
        GUI_ShowBMP(90, 36, 28, 14, err_text, 0);
    }
    else if (!user_main_para.heating_flag)
    {
        GUI_ShowBMP(65, 27, 24, 24, standby_temp_logo, 0);
        GUI_ShowBMP(90, 36, 28, 14, standby_temp_text, 0);
    }
    else if (user_main_para.set_pwm > Temp_demarcation_line)
    {
        GUI_ShowBMP(65, 27, 24, 24, rise_temp_logo, 0);
        GUI_ShowBMP(90, 36, 28, 14, rise_temp_text, 0);
    }
    else
    {
        GUI_ShowBMP(65, 27, 24, 24, const_temp_logo, 0);
        GUI_ShowBMP(90, 36, 28, 14, const_temp_text, 0);
    }
    // 更新flash中目标温度
    updata_user_parameter();
}

// todo
/**
 * @description: 菜单界面刷新
 * @return {*}
 */
static void menu_ui_refresh(void)
{
    OLED_Clear();

    for (uint32_t i = 0; i < menu_list_len; i++)
    {
        // if ((user_menu_para.list_y_now + i * MENU_FONT_NUM) >= 0)
        if (user_menu_para.frame_list_index == i)
        {
            GUI_ShowString(user_menu_para.list_x_now, user_menu_para.list_y_now + i * MENU_FONT_NUM, menu_list[i].str, MENU_FONT_NUM, 0);
        }
        else
        {
            GUI_ShowString(user_menu_para.list_x_now, user_menu_para.list_y_now + i * MENU_FONT_NUM, menu_list[i].str, MENU_FONT_NUM, 1);
        }
    }

    // 菜单滑动
    if (user_menu_para.list_y_now < user_menu_para.list_y_tag)
    {
        user_menu_para.list_y_now += 1;
    }
    else if (user_menu_para.list_y_now > user_menu_para.list_y_tag)
    {
        user_menu_para.list_y_now -= 1;
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
            user_parameter_init(); // 读取flash中数据
            GUI_ComeBMP(0, 0, OLED_WIDTH, OLED_HEIGHT, main_ui_bg, 0);
            main_ui_process = PROCESS_MAIN_UI;
        }
        break;
    case PROCESS_MAIN_UI:
        /* 主界面ui流程 */
        ui_main_refresh();
        break;
    case PROCESS_MENU_UI:
        /* 菜单界面流程 */
        menu_ui_refresh();
        break;
    default:
        break;
    }
    OLED_Display();
}

/**
 * @description: key 主流程
 * @return {*}
 */
void key_Main_process(void)
{
    switch (main_ui_process)
    {
    case PROCESS_INIT_UI:
        // 过度界面中 按键不作处理
        for (key_flag_type_t i = 0; i < NUM_FLAG_T; i++)
        {
            if (key_flag[i])
            {
                key_flag[i] = 0;
            }
        }
        break;
    case PROCESS_MAIN_UI:
        if (key_flag[SINGLE_FLAG])
        {
            // 单点
            user_main_para.heating_flag = !user_main_para.heating_flag;
            key_flag[SINGLE_FLAG] = 0;
        }
        else if (key_flag[DOUBLE_FLAG])
        {
            // 双点
            key_flag[DOUBLE_FLAG] = 0;
        }
        else if (key_flag[LONG_FLAG])
        {
            // 长按
            key_flag[LONG_FLAG] = 0;
            OLED_Clear();
            main_ui_process = PROCESS_MENU_UI; // 状态切换
        }
        else if (key_flag[RIGHT_FLAG])
        {
            // 右转
            control_temp_para.target_temp += Temp_step;
            if (control_temp_para.target_temp > Temp_upper)
            {
                control_temp_para.target_temp = Temp_upper;
            }

            key_flag[RIGHT_FLAG] = 0;
        }
        else if (key_flag[LEFT_FLAG])
        {
            // 左转
            control_temp_para.target_temp -= Temp_step;
            if (control_temp_para.target_temp < Temp_lower)
            {
                control_temp_para.target_temp = Temp_lower;
            }
            key_flag[LEFT_FLAG] = 0;
        }
        break;
    case PROCESS_MENU_UI:
        if (key_flag[SINGLE_FLAG])
        {
            // 单点
            key_flag[SINGLE_FLAG] = 0;
        }
        else if (key_flag[DOUBLE_FLAG])
        {
            // 双点
            key_flag[DOUBLE_FLAG] = 0;
        }
        else if (key_flag[LONG_FLAG])
        {
            // 长按
            key_flag[LONG_FLAG] = 0;
            GUI_ShowBMP(0, 0, OLED_WIDTH, OLED_HEIGHT, main_ui_bg, 0);
            main_ui_process = PROCESS_MAIN_UI; // 状态切换
        }
        else if (key_flag[RIGHT_FLAG])
        {
            // 右转
            // 菜单滑动
            user_menu_para.frame_list_index += 1;
            user_menu_para.frame_list_line += 1;
            if (user_menu_para.frame_list_line > (OLED_HEIGHT / MENU_FONT_NUM - 1))
            {
                user_menu_para.frame_list_line = (OLED_HEIGHT / MENU_FONT_NUM - 1);

                user_menu_para.list_top_line += 1;
                user_menu_para.list_bottom_line += 1;
                // 菜单滑动限制
                if (user_menu_para.list_bottom_line > menu_list_len)
                {
                    user_menu_para.list_bottom_line = menu_list_len;
                    user_menu_para.list_top_line = menu_list_len - (OLED_HEIGHT / MENU_FONT_NUM);
                }
                else
                {
                    user_menu_para.list_y_tag -= MENU_FONT_NUM;
                }
                if (user_menu_para.frame_list_index > menu_list_len - 1)
                {
                    user_menu_para.frame_list_index = menu_list_len - 1;
                }
            }

            key_flag[RIGHT_FLAG] = 0;
        }
        else if (key_flag[LEFT_FLAG])
        {
            // 左转
            // 菜单滑动
            user_menu_para.frame_list_index -= 1;
            user_menu_para.frame_list_line -= 1;
            if (user_menu_para.frame_list_line < 0)
            {
                user_menu_para.frame_list_line = 0;

                user_menu_para.list_top_line -= 1;
                user_menu_para.list_bottom_line -= 1;
                // 菜单滑动限制
                if (user_menu_para.list_top_line < 0)
                {
                    user_menu_para.list_top_line = 0;
                    user_menu_para.list_bottom_line = OLED_HEIGHT / MENU_FONT_NUM;
                }
                else
                {
                    user_menu_para.list_y_tag += MENU_FONT_NUM;
                }

                if (user_menu_para.frame_list_index < 0)
                {
                    user_menu_para.frame_list_index = 0;
                }
            }

            key_flag[LEFT_FLAG] = 0;
        }
        break;
    }
}
