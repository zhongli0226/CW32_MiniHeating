#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>

//-------------写命令和数据定义-------------------
#define OLED_CMD 0  // 写命令
#define OLED_DATA 1 // 写数据

//--------------OLED参数定义---------------------
#define PAGE_SIZE 8
#define XLevelL 0x00
#define XLevelH 0x10
#define YLevel 0xB0
#define Brightness 0xFF
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

//OLED控制用函数
/**
 * @description: 打开OLED显示屏
 * @return {*}
 */
void OLED_Display_On(void);

/**
 * @description:  关闭OLED显示屏
 * @return {*}
 */
void OLED_Display_Off(void);

/**
 * @description:  将软件缓存区的内容更新到oled上显示
 * @return {*}
 */
void OLED_Display(void);

/**
 * @description:  清屏函数
 * @return {*}
 */
void OLED_Clear(void);

/**
 * @description: oled 复位函数
 * @return {*}
 */
void OLED_Reset(void);

/**
 * @description: oled 初始化
 * @return {*}
 */
void OLED_Init(void);

//缓存相关函数
/**
 * @description: 获取缓存空间的地址
 * @return {*} 返回缓存空间起始地址
 */
uint8_t *Get_OLEDBuffer(void);



#endif
