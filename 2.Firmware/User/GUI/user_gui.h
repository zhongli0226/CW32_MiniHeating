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

#endif
