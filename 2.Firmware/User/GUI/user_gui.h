#ifndef __USER_GUI_H
#define __USER_GUI_H

#include <stdint.h>

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
 * @description: 设定温度向上提高一个步进
 * @return {*}
 */
void target_temp_step_up(void);

/**
 * @description: 设定温度向下降低一个步进
 * @return {*}
 */
void target_temp_step_down(void);

/**
 * @description: ui 流程
 * @return {*}
 */
void UI_Main_Process(void);

#endif
