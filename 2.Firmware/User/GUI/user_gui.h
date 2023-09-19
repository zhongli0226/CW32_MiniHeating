/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2023-08-12 15:21:18
 * @LastEditors: tangwc
 * @LastEditTime: 2023-09-17 20:52:57
 * @FilePath: \2.Firmware\User\GUI\user_gui.h
 * 
 *  Copyright (c) 2023 by tangwc, All Rights Reserved. 
 */
#ifndef __USER_GUI_H
#define __USER_GUI_H

#include <stdint.h>

typedef enum
{
    SINGLE_FLAG = 0,
    DOUBLE_FLAG,
    LONG_FLAG,
    RIGHT_FLAG,
    LEFT_FLAG,
    NUM_FLAG_T, // 标志位总数
} key_flag_type_t;

/**
 * @description: 设置key外部事件标志flag
 * @param {key_flag_type_t} flag:
 * @return {*}
 */
void set_key_flag(key_flag_type_t flag);

/**
 * @description: 设置刷新flag
 * @return {*}
 */
void set_actual_temp_flag(void);

/**
 * @description: 设置刷新flag
 * @return {*}
 */
void set_pwr_volt_flag(void);

/**
 * @description: flash更新
 * @return {*}
 */
void set_flash_update_flag(void);


/**
 * @description: ui 流程
 * @return {*}
 */
void UI_Main_Process(void);

/**
 * @description: key 主流程
 * @return {*}
 */
void key_Main_process(void);

#endif
