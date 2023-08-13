#ifndef __TEMP_CONTROL_H
#define __TEMP_CONTROL_H

#include <stdint.h>

/**
 * @description: 获得实际温度
 * @return {*}  uint16_t 光耦上实际温度
 */
uint16_t Get_actual_temp(void);

/**
 * @description: 获得目标温度
 * @return {*}  uint16_t 内部设置要达到的温度
 */
uint16_t Get_target_temp(void);

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

#endif
