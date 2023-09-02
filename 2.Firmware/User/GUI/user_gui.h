#ifndef __USER_GUI_H
#define __USER_GUI_H

#include <stdint.h>


/**
 * @description: 初始logo过场动画
 * @return {*} 0：动画运行中，1：动画运行完成 
 */
uint8_t Transitions_logo(void);

/**
 * @description: 主界面背景显示
 * @return {*}
 */
void main_gui_show(void);

/**
 * @description: 刷新目标温度
 * @return {*}
 */
void refresh_target_temp(void);
/**
 * @description: 刷新实际温度
 * @return {*}
 */
void set_actual_temp_flag(void);
void refresh_actual_temp(void);

/**
 * @description: 刷新电源电压
 * @return {*}
 */
void set_pwr_Volt_flag(void);
void refresh_pwr_Voltage(void);
/**
 * @description: 显示功率占比
 * @return {*}
 */
void refresh_pwm_prop(void);

#endif
