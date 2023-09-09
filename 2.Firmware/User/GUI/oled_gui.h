#ifndef __OLED_GUI_H
#define __OLED_GUI_H

#include <stdint.h>

/**
 * @description: 在oled上画一个点
 * @param {uint8_t} x:  点的x坐标
 * @param {uint8_t} y:  点的y坐标
 * @param {uint8_t} color:  点的颜色值 1:白 ; 0:黑色
 * @return {*}
 */
void GUI_DrawPoint(uint8_t x, uint8_t y, uint8_t color);

/**
 * @description: oled 填充一片区域
 * @param {uint8_t} x1: 指定区域的x起始坐标
 * @param {uint8_t} y1: 指定区域的y起始坐标
 * @param {uint8_t} x2: 指定区域的x结束坐标
 * @param {uint8_t} y2: 指定区域的y结束坐标
 * @param {uint8_t} color:  指定区域的颜色值  1-白 ; 0-黑色
 * @return {*}
 */
void GUI_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);

/**
 * @description: 在OLED上画一个水平线 
 * @param {uint16_t} x0:    水平线起点所在列的位置
 * @param {uint8_t} y0:     水平线起点所在行的位置    
 * @param {uint16_t} x1:    水平线终点所在列的位置
 * @param {uint8_t} color:  显示颜色    1-白 ; 0-黑色
 * @return {*}
 */
void GUI_HLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t color);

/**
 * @description:    在oled上画一个垂直线
 * @param {uint16_t} x0:    垂直线起点所在列的位置
 * @param {uint8_t} y0:     垂直线起点所在行的位置
 * @param {uint8_t} y1:     垂直线终点所在行的位置
 * @param {uint8_t} color:  显示颜色
 * @return {*}
 */
void GUI_RLine(uint16_t x0, uint8_t y0, uint8_t y1, uint8_t color);

/**
 * @description: 画任意两点之间的直线，并且可设置线的宽度
 * @param {uint32_t} x0:    直线起点的x坐标值
 * @param {uint32_t} y0:    直线起点的y坐标值
 * @param {uint32_t} x1:    直线终点的x坐标值
 * @param {uint32_t} y1:    直线终点的y坐标值
 * @param {uint8_t} with:   线宽(0-50)
 * @param {uint8_t} color:  显示颜色
 * @return {*}
 */
void GUI_LineWith(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t with, uint8_t color);

/** 
 * @description:    显示单个英文字符
 * @param {uint8_t} x:      字符显示位置的开始x坐标
 * @param {uint8_t} y:      字符显示位置的开始y坐标
 * @param {uint8_t} chr:    显示字符的ASCII码（0〜94）
 * @param {uint8_t} size:   显示字符的大小（8,16）
 * @param {uint8_t} mode:   0：白色背景和黑色字符   1：黑色背景和白色字符
 * @return {*}
 */
void GUI_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode);

/**
 * @description:    显示英文字符串
 * @param {uint8_t} x:  英文字符串的x坐标
 * @param {uint8_t} y:  英文字符串的y坐标
 * @param {uint8_t} *p: 英文字符串的指针
 * @param {uint8_t} size: 显示字符的大小（8,16）
 * @param {uint8_t} mode:   0：白色背景和黑色字符   1：黑色背景和白色字符
 * @return {*}
 */
void GUI_ShowString(uint8_t x, uint8_t y, const char *p, uint8_t size, uint8_t mode);

/**
 * @description: oled 显示数字
 * @param {uint8_t} x:  数字的起始x坐标
 * @param {uint8_t} y:  数字的起始y坐标
 * @param {uint32_t} num:   数字（0〜4294967295）
 * @param {uint8_t} len:    显示数字的长度
 * @param {uint8_t} size:   显示数字的大小（8,16）
 * @param {uint8_t} mode:   0：白色背景和黑色字符   1：黑色背景和白色字符
 * @return {*}
 */
void GUI_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode);

#endif
