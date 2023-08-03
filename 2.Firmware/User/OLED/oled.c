#include <string.h>
#include "oled.h"
#include "oledfont.h"

#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"
#include "system_cw32f030.h"
#include "elog.h"


#define TAG "OLED"

//-----------------OLED端口定义----------------
#define OLED_CLK_GPIO GPIO_PIN_5
#define OLED_MOSI_GPIO GPIO_PIN_6
#define OLED_RST_GPIO GPIO_PIN_7
#define OLED_DC_GPIO GPIO_PIN_8
#define OLED_CS_GPIO GPIO_PIN_9

//-----------------OLED端口操作----------------
#define OLED_CLK_CLR GPIO_WritePin(CW_GPIOB, OLED_CLK_GPIO, GPIO_Pin_RESET)
#define OLED_CLK_SET GPIO_WritePin(CW_GPIOB, OLED_CLK_GPIO, GPIO_Pin_SET)

#define OLED_MOSI_CLR GPIO_WritePin(CW_GPIOB, OLED_MOSI_GPIO, GPIO_Pin_RESET)
#define OLED_MOSI_SET GPIO_WritePin(CW_GPIOB, OLED_MOSI_GPIO, GPIO_Pin_SET)

#define OLED_RST_Clr GPIO_WritePin(CW_GPIOB, OLED_RST_GPIO, GPIO_Pin_RESET)
#define OLED_RST_Set GPIO_WritePin(CW_GPIOB, OLED_RST_GPIO, GPIO_Pin_SET)

#define OLED_DC_Clr GPIO_WritePin(CW_GPIOB, OLED_DC_GPIO, GPIO_Pin_RESET)
#define OLED_DC_Set GPIO_WritePin(CW_GPIOB, OLED_DC_GPIO, GPIO_Pin_SET)

#define OLED_CS_Clr GPIO_WritePin(CW_GPIOB, OLED_CS_GPIO, GPIO_Pin_RESET)
#define OLED_CS_Set GPIO_WritePin(CW_GPIOB, OLED_CS_GPIO, GPIO_Pin_SET)


//-------------写命令和数据定义-------------------
#define OLED_CMD 0  //写命令
#define OLED_DATA 1 //写数据


//--------------OLED参数定义---------------------
#define PAGE_SIZE 8
#define XLevelL 0x00
#define XLevelH 0x10
#define YLevel 0xB0
#define Brightness 0xFF
#define WIDTH 128
#define HEIGHT 64

//--------------OLED buff缓存区域---------------------
static uint8_t OLED_GRAM[128][8] = {0};

/**
 * @description: oled gpio 初始化
 * @return {*}
 */
static void OLED_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 使能GPIOB的配置时钟  
    __RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.IT = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pins = OLED_CLK_GPIO | OLED_MOSI_GPIO | OLED_RST_GPIO | OLED_DC_GPIO | OLED_CS_GPIO;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

    elog_i(TAG,"OLED GPIO Init!");
}

/**
 * @description: 软件模拟spi写入
 * @param {uint8_t} Data: 要写入的数据
 * @return {*}
 */
static void SPI_WriteByte(uint8_t Data)
{
    uint8_t i = 0;
    for (i = 8; i > 0; i--)
    {
        if (Data & 0x80)
        {
            OLED_MOSI_SET; //写数据1
        }
        else
        {
            OLED_MOSI_CLR; //写数据0
        }
        OLED_CLK_CLR; //将时钟拉低拉高
        OLED_CLK_SET; //发送1bit数据
        Data <<= 1;
    }
}

/**
 * @description: 将一个字节写入OLED屏幕
 * @param {uint8_t} dat: 要编写的内容
 * @param {uint8_t} cmd: 0：写命令 1：写数据
 * @return {*}
 */
static void OLED_WR_Byte(uint8_t dat, uint8_t cmd)
{
    if (cmd)
    {
        OLED_DC_Set;
    }
    else
    {
        OLED_DC_Clr;
    }
    OLED_CS_Clr;
    SPI_WriteByte(dat);
    OLED_CS_Set;
}

/**
 * @description: 打开OLED显示屏
 * @return {*}
 */
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD); //SET DCDC命令
    OLED_WR_Byte(0X14, OLED_CMD); //DCDC ON
    OLED_WR_Byte(0XAF, OLED_CMD); //DISPLAY ON
}

/**
 * @description:  关闭OLED显示屏
 * @return {*}
 */
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD); //SET DCDC命令
    OLED_WR_Byte(0X10, OLED_CMD); //DCDC OFF
    OLED_WR_Byte(0XAE, OLED_CMD); //DISPLAY OFF
}

/**
 * @description:  将软件缓存区的内容更新到oled上显示
 * @return {*}
 */
void OLED_Display(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WR_Byte(0xb0 + i, OLED_CMD); //设置页地址（0~7）
        OLED_WR_Byte(0x00, OLED_CMD);     //设置显示位置—列低地址
        OLED_WR_Byte(0x10, OLED_CMD);     //设置显示位置—列高地址
        for (n = 0; n < 128; n++)
            OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
    } //更新显示
}

/**
 * @description:  清屏函数
 * @return {*}
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
        for (n = 0; n < 128; n++)
            OLED_GRAM[n][i] = 0X00;
    OLED_Display(); //更新显示
}

/**
 * @description: oled 复位函数
 * @return {*}
 */
void OLED_Reset(void)
{
    OLED_RST_Set;
    delay1ms(100);
    OLED_RST_Clr;
    delay1ms(100);
    OLED_RST_Set;
}

/**
 * @description: oled 初始化
 * @return {*}
 */
void OLED_Init(void)
{
    OLED_GPIO_Init();
    delay1ms(200);
    OLED_Reset(); //复位OLED

    /**************初始化SSD1306*****************/
    OLED_WR_Byte(0xAE, OLED_CMD); //关闭显示
    OLED_WR_Byte(0xD5, OLED_CMD); //设置时钟分频因子,震荡频率
    OLED_WR_Byte(0x80, OLED_CMD); //[3:0],分频因子;[7:4],震荡频率
    OLED_WR_Byte(0xA8, OLED_CMD); //设置驱动路数
    OLED_WR_Byte(0X3F, OLED_CMD); //默认0X3F(1/64)
    OLED_WR_Byte(0xD3, OLED_CMD); //设置显示偏移
    OLED_WR_Byte(0X00, OLED_CMD); //默认为0

    OLED_WR_Byte(0x40, OLED_CMD); //设置显示开始行 [5:0],行数.

    OLED_WR_Byte(0x8D, OLED_CMD); //电荷泵设置
    OLED_WR_Byte(0x14, OLED_CMD); //bit2，开启/关闭
    OLED_WR_Byte(0x20, OLED_CMD); //设置内存地址模式
    OLED_WR_Byte(0x02, OLED_CMD); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
    OLED_WR_Byte(0xA1, OLED_CMD); //段重定义设置,bit0:0,0->0;1,0->127;
    OLED_WR_Byte(0xC0, OLED_CMD); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
    OLED_WR_Byte(0xDA, OLED_CMD); //设置COM硬件引脚配置
    OLED_WR_Byte(0x12, OLED_CMD); //[5:4]配置

    OLED_WR_Byte(0x81, OLED_CMD); //对比度设置
    OLED_WR_Byte(0xEF, OLED_CMD); //1~255;默认0X7F (亮度设置,越大越亮)
    OLED_WR_Byte(0xD9, OLED_CMD); //设置预充电周期
    OLED_WR_Byte(0xf1, OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
    OLED_WR_Byte(0xDB, OLED_CMD); //设置VCOMH 电压倍率
    OLED_WR_Byte(0x30, OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

    OLED_WR_Byte(0xA4, OLED_CMD); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
    OLED_WR_Byte(0xA6, OLED_CMD); //设置显示方式;bit0:1,反相显示;0,正常显示
    OLED_WR_Byte(0xAF, OLED_CMD); //开启显示
    OLED_Clear();

    elog_i(TAG,"OLED Init!");
}

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
    if (x > 131 || y > 63)
        return; //超出范围了.
    pos = 7 - y / 8;
    bx = y % 8;
    temp = 1 << (7 - bx);
    if (color)
        OLED_GRAM[x][pos] |= temp;
    else
        OLED_GRAM[x][pos] &= ~temp;
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
    OLED_Display(); //更新显示
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
 * @param {uint8_t} x:      字符显示位置的开始x坐标
 * @param {uint8_t} y:      字符显示位置的开始y坐标
 * @param {uint8_t} chr:    显示字符的ASCII码（0〜94）
 * @param {uint8_t} size:   显示字符的大小（8,16）
 * @param {uint8_t} mode:   0：白色背景和黑色字符   1：黑色背景和白色字符
 * @return {*}
 */
void GUI_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    chr = chr - ' '; //得到偏移后的值
    for (t = 0; t < size; t++)
    {
        if (size == 12)
            temp = oled_asc2_1206[chr][t]; //调用1206字体
        else
            temp = oled_asc2_1608[chr][t]; //调用1608字体
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                GUI_DrawPoint(x, y, mode);
            else
                GUI_DrawPoint(x, y, !mode);
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
 * @param {uint8_t} x:  英文字符串的x坐标
 * @param {uint8_t} y:  英文字符串的y坐标
 * @param {uint8_t} *p: 英文字符串的指针
 * @param {uint8_t} size: 显示字符的大小（8,16）
 * @param {uint8_t} mode:   0：白色背景和黑色字符   1：黑色背景和白色字符
 * @return {*}
 */
void GUI_ShowString(uint8_t x, uint8_t y, const uint8_t *p, uint8_t size, uint8_t mode)
{
    // #define MAX_CHAR_POSX 122
#define MAX_CHAR_POSX 126
#define MAX_CHAR_POSY 58
    while (*p != '\0')
    {
        if (x > MAX_CHAR_POSX)
        {
            x = 0;
            y += size;
        }
        if (y > MAX_CHAR_POSY)
        {
            y = x = 0;
            OLED_Clear();
        }
        GUI_ShowChar(x, y, *p, size, mode);
        x += size/2;
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
