#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "oled.h"
#include "oled_gui.h"
#include "basics_font.h"

#include "elog.h"
#include "system_cw32f030.h"

#define TAG "OLED_GUI"

/**
 * @description: 在oled上画一个点
 * @param {uint8_t} x:  点的x坐标
 * @param {uint8_t} y:  点的y坐标
 * @param {uint8_t} color:  点的颜色值 1:白 ; 0:黑色
 * @return {*}
 */
void GUI_DrawPoint(uint8_t x, uint8_t y, uint8_t color)
{
    uint8_t pos, bx, temp = 0;
    uint8_t *p = Get_OLEDBuffer();
    if (x > 127 || y > 63)
    {
        // elog_w(TAG, "Out of range.........");
        return; // 超出范围了.
    }

    pos = 7 - y / 8;
    bx = y % 8;
    temp = 1 << (7 - bx);
    if (color)
        p[x + pos * OLED_WIDTH] |= temp;
    else
        p[x + pos * OLED_WIDTH] &= ~temp;
}

/**
 * @description: oled 填充一片区域
 * @param {uint8_t} x1: 指定区域的x起始坐标
 * @param {uint8_t} y1: 指定区域的y起始坐标
 * @param {uint8_t} x2: 指定区域的x结束坐标
 * @param {uint8_t} y2: 指定区域的y结束坐标
 * @param {uint8_t} color:  指定区域的颜色值  1-白 ; 0-黑色
 * @return {*}
 */
void GUI_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
    uint8_t x, y;
    for (x = x1; x <= x2; x++)
    {
        for (y = y1; y <= y2; y++)
            GUI_DrawPoint(x, y, color);
    }
    // OLED_Display(); // 更新显示
}

/**
 * @description: 在OLED上画一个水平线
 * @param {uint16_t} x0:    水平线起点所在列的位置
 * @param {uint8_t} y0:     水平线起点所在行的位置
 * @param {uint16_t} x1:    水平线终点所在列的位置
 * @param {uint8_t} color:  显示颜色    1-白 ; 0-黑色
 * @return {*}
 */
void GUI_HLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t color)
{
    uint8_t temp;
    if (x0 > x1) // 对x0、x1大小进行排列，以便画图
    {
        temp = x1;
        x1 = x0;
        x0 = temp;
    }
    do
    {
        GUI_DrawPoint(x0, y0, color); // 逐点显示，描出垂直线
        x0++;
    } while (x1 >= x0);
}

/**
 * @description:    在oled上画一个垂直线
 * @param {uint16_t} x0:    垂直线起点所在列的位置
 * @param {uint8_t} y0:     垂直线起点所在行的位置
 * @param {uint8_t} y1:     垂直线终点所在行的位置
 * @param {uint8_t} color:  显示颜色
 * @return {*}
 */
void GUI_RLine(uint16_t x0, uint8_t y0, uint8_t y1, uint8_t color)
{
    uint8_t temp;
    if (y0 > y1) // 对y0、y1大小进行排列，以便画图
    {
        temp = y1;
        y1 = y0;
        y0 = temp;
    }
    do
    {
        GUI_DrawPoint(x0, y0, color); // 逐点显示，描出垂直线
        y0++;
    } while (y1 >= y0);
}

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
void GUI_LineWith(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t with, uint8_t color)
{
    int32_t dx;    // 直线x轴差值变量
    int32_t dy;    // 直线y轴差值变量
    int8_t dx_sym; // x轴增长方向，为-1时减值方向，为1时增值方向
    int8_t dy_sym; // y轴增长方向，为-1时减值方向，为1时增值方向
    int32_t dx_x2; // dx*2值变量，用于加快运算速度
    int32_t dy_x2; // dy*2值变量，用于加快运算速度
    int32_t di;    // 决策变量

    int32_t wx, wy; // 线宽变量
    int32_t draw_a, draw_b;

    /* 参数过滤 */
    if (with == 0)
        return;
    if (with > 50)
        with = 50;

    dx = x1 - x0; // 求取两点之间的差值
    dy = y1 - y0;

    wx = with / 2;
    wy = with - wx - 1;

    /* 判断增长方向，或是否为水平线、垂直线、点 */
    if (dx > 0) // 判断x轴方向
    {
        dx_sym = 1; // dx>0，设置dx_sym=1
    }
    else
    {
        if (dx < 0)
        {
            dx_sym = -1; // dx<0，设置dx_sym=-1
        }
        else
        { /* dx==0，画垂直线，或一点 */
            wx = x0 - wx;
            if (wx < 0)
                wx = 0;
            wy = x0 + wy;

            while (1)
            {
                x0 = wx;
                GUI_RLine(x0, y0, y1, color);
                if (wx >= wy)
                    break;
                wx++;
            }

            return;
        }
    }

    if (dy > 0) // 判断y轴方向
    {
        dy_sym = 1; // dy>0，设置dy_sym=1
    }
    else
    {
        if (dy < 0)
        {
            dy_sym = -1; // dy<0，设置dy_sym=-1
        }
        else
        { /* dy==0，画水平线，或一点 */
            wx = y0 - wx;
            if (wx < 0)
                wx = 0;
            wy = y0 + wy;

            while (1)
            {
                y0 = wx;
                GUI_HLine(x0, y0, x1, color);
                if (wx >= wy)
                    break;
                wx++;
            }
            return;
        }
    }

    /* 将dx、dy取绝对值 */
    dx = dx_sym * dx;
    dy = dy_sym * dy;

    /* 计算2倍的dx及dy值 */
    dx_x2 = dx * 2;
    dy_x2 = dy * 2;

    /* 使用Bresenham法进行画直线 */
    if (dx >= dy) // 对于dx>=dy，则使用x轴为基准
    {
        di = dy_x2 - dx;
        while (x0 != x1)
        { /* x轴向增长，则宽度在y方向，即画垂直线 */
            draw_a = y0 - wx;
            if (draw_a < 0)
                draw_a = 0;
            draw_b = y0 + wy;
            GUI_RLine(x0, draw_a, draw_b, color);

            x0 += dx_sym;
            if (di < 0)
            {
                di += dy_x2; // 计算出下一步的决策值
            }
            else
            {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        draw_a = y0 - wx;
        if (draw_a < 0)
            draw_a = 0;
        draw_b = y0 + wy;
        GUI_RLine(x0, draw_a, draw_b, color);
    }
    else // 对于dx<dy，则使用y轴为基准
    {
        di = dx_x2 - dy;
        while (y0 != y1)
        { /* y轴向增长，则宽度在x方向，即画水平线 */
            draw_a = x0 - wx;
            if (draw_a < 0)
                draw_a = 0;
            draw_b = x0 + wy;
            GUI_HLine(draw_a, y0, draw_b, color);

            y0 += dy_sym;
            if (di < 0)
            {
                di += dx_x2;
            }
            else
            {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        draw_a = x0 - wx;
        if (draw_a < 0)
            draw_a = 0;
        draw_b = x0 + wy;
        GUI_HLine(draw_a, y0, draw_b, color);
    }
}

/**
 * @description:    显示单个英文字符
 * @param {int8_t} x:      字符显示位置的开始x坐标
 * @param {int8_t} y:      字符显示位置的开始y坐标
 * @param {uint8_t} chr:    显示字符的ASCII码（0〜94）
 * @param {uint8_t} size:   显示字符的大小（12,16）
 * @param {uint8_t} mode:   0：白色背景和黑色字符   1：黑色背景和白色字符
 * @return {*}
 */
void GUI_ShowChar(int8_t x, int8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    uint8_t temp, t, t1;
    int8_t y0 = y;
    chr = chr - ' ';                                                // 得到偏移后的值
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); // 得到字体一个字符对应点阵集所占的字节数
    for (t = 0; t < csize; t++)
    {
        if (size == 12)
            temp = oled_asc2_1206[chr][t]; // 调用1206字体
        else if (size == 16)
            temp = oled_asc2_1608[chr][t]; // 调用1608字体
        else if (size == 40)
        {
            // 40字体特殊处理
            if ((chr + ' ') >= '0' && (chr + ' ') <= '9')
                temp = oled_asc2_4020[(chr + ' ') - '0'][t]; // 调用4020字体
            else if ((chr + ' ') == 'E')
                temp = oled_asc2_4020[10][t]; // 调用4020字体
            else if ((chr + ' ') == 'R')
                temp = oled_asc2_4020[11][t]; // 调用4020字体
            else if ((chr + ' ') == ' ')
                temp = 0x00;
            else
                return;
        }
        else
            return;
        for (t1 = 0; t1 < 8; t1++)
        {
            if (y >= 0)
            {
                if (temp & 0x80)
                    GUI_DrawPoint((uint8_t)x, (uint8_t)y, mode);
                else
                    GUI_DrawPoint((uint8_t)x, (uint8_t)y, !mode);
            }
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

/**
 * @description:    显示英文字符串
 * @param {int8_t} x:  英文字符串的x坐标
 * @param {int8_t} y:  英文字符串的y坐标
 * @param {uint8_t} *p: 英文字符串的指针
 * @param {uint8_t} size: 显示字符的大小（8,16）
 * @param {uint8_t} mode:   0：白色背景和黑色字符   1：黑色背景和白色字符
 * @return {*}
 */
void GUI_ShowString(int8_t x, int8_t y, const char *p, uint8_t size, uint8_t mode)
{
    // #define MAX_CHAR_POSX 122
    // #define MAX_CHAR_POSX 126
    // #define MAX_CHAR_POSY 58
    while (*p != '\0')
    {
        //* 关闭限制
        // if (x > MAX_CHAR_POSX)
        // {
        //     x = 0;
        //     y += size;
        // }
        // if (y > MAX_CHAR_POSY)
        // {
        //     y = x = 0;
        //     OLED_Clear();
        // }
        GUI_ShowChar(x, y, *p, size, mode);
        x += size / 2;
        p++;
    }
}

/**
 * @description: 获取m的n次幂
 * @param {uint8_t} m: 乘数
 * @param {uint8_t} n: 幂
 * @return {*}
 */
static uint32_t mypow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while (n--)
        result *= m;
    return result;
}

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
void GUI_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / mypow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                GUI_ShowChar(x + (size / 2) * t, y, ' ', size, mode);
                continue;
            }
            else
                enshow = 1;
        }
        GUI_ShowChar(x + (size / 2) * t, y, temp + '0', size, mode);
    }
}

/**
 * @description: 画一个图片
 * @param {uint8_t} x:  图片的起始x坐标
 * @param {uint8_t} y:  图片的起始y坐标
 * @param {uint8_t} px：图片的x长度
 * @param {uint8_t} py：图片的y长度
 * @param {uint8_t} *bg 图片buff地址
 * @param {uint8_t} mode 0：白色背景和黑色字符   1：黑色背景和白色字符
 * @return {*}
 */
void GUI_ShowBMP(uint8_t x, uint8_t y, uint8_t px, uint8_t py, const uint8_t *bg, uint8_t mode)
{
    uint8_t temp, t1;
    uint16_t j, i;
    uint8_t y0 = y;

    if ((x + px > OLED_WIDTH) || (y + py > OLED_HEIGHT))
    {
        elog_w(TAG, "bmp over .....");
        return;
    }
    i = (uint16_t)(ceil((float)px / 2) * ceil((float)py / 4));

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
            if ((y - y0) == py)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

/**
 * @brief             图片渐变消失
 * @param[in]         x 图片的起始x坐标
 * @param[in]         y 图片的起始y坐标
 * @param[in]         px 图片的x长度
 * @param[in]         py 图片的y长度
 * @param[in]         bg 图片buff地址
 * @param[in]         mode 0：白色背景和黑色字符   1：黑色背景和白色字符
 * @retval            0 : 完成渐变效果
 * @retval            1 : 图片大小超出
 */
uint32_t GUI_DisapperBMP(uint8_t x, uint8_t y, uint8_t px, uint8_t py, const uint8_t *bg, uint8_t mode)
{
    uint32_t len;

    if ((x + px > OLED_WIDTH) || (y + py > OLED_HEIGHT))
    {
        elog_w(TAG, "bmp over .....");
        return 1;
    }
    len = (uint32_t)(ceil((float)px / 2) * ceil((float)py / 4));

    int32_t disapper_temp = 0;
    while (disapper_temp <= 8)
    {
        OLED_Clear();                        // 清除内部缓冲区
        GUI_ShowBMP(x, y, px, py, bg, mode); // 第一段输出位置

        uint8_t *p = Get_OLEDBuffer();
        for (uint32_t i = 0; i < len; i++)
        {
            p[i] = p[i] & (rand() % 0xff) >> disapper_temp; // rand()%0xff = 0 ~ 0xff
        }

        disapper_temp += 2;

        OLED_Display();
        delay1ms(100);
    }
    OLED_Display();
    // delay1ms(1000);
    return 0;
}

/**
 * @brief             图片渐变显示
 * @param[in]         x 图片的起始x坐标
 * @param[in]         y 图片的起始y坐标
 * @param[in]         px 图片的x长度
 * @param[in]         py 图片的y长度
 * @param[in]         bg 图片buff地址
 * @param[in]         mode 0：白色背景和黑色字符   1：黑色背景和白色字符
 * @retval            0 : 完成渐变效果
 * @retval            1 : 图片大小超出
 */
uint32_t GUI_ComeBMP(uint8_t x, uint8_t y, uint8_t px, uint8_t py, const uint8_t *bg, uint8_t mode)
{
    uint32_t len;

    if ((x + px > OLED_WIDTH) || (y + py > OLED_HEIGHT))
    {
        elog_w(TAG, "bmp over .....");
        return 2;
    }
    len = (uint32_t)(ceil((float)px / 2) * ceil((float)py / 4));

    int32_t come_temp = 8;
    while (come_temp >= 0)
    {
        OLED_Clear();                        // 清除内部缓冲区
        GUI_ShowBMP(x, y, px, py, bg, mode); // 第一段输出位置
        uint8_t *p = Get_OLEDBuffer();

        for (uint32_t i = 0; i < len; i++)
        {
            p[i] = p[i] & (rand() % 0xff) >> come_temp; // rand()%0xff = 0 ~ 0xff
        }

        come_temp -= 2;

        OLED_Display();
        delay1ms(100);
    }

    GUI_ShowBMP(x, y, px, py, bg, mode);
    OLED_Display();
    // delay1ms(1000);
    return 0;
}
