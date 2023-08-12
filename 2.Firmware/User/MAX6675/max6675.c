#include <string.h>

#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"
#include "system_cw32f030.h"

#include "max6675.h"

#include "elog.h"

#define TAG "MAX6675"

//-----------------max6675端口定义----------------
#define MAX6675_CLK_GPIO GPIO_PIN_10  // PA10
#define MAX6675_MISO_GPIO GPIO_PIN_11 // PA11
#define MAX6675_CS_GPIO GPIO_PIN_12   // PA12

//-----------------OLED端口操作----------------
#define MAX6675_CLK_CLR() GPIO_WritePin(CW_GPIOA, MAX6675_CLK_GPIO, GPIO_Pin_RESET)
#define MAX6675_CLK_SET() GPIO_WritePin(CW_GPIOA, MAX6675_CLK_GPIO, GPIO_Pin_SET)

#define MAX6675_MISO_GET() GPIO_ReadPin(CW_GPIOA, MAX6675_MISO_GPIO)

#define MAX6675_CS_CLR() GPIO_WritePin(CW_GPIOA, MAX6675_CS_GPIO, GPIO_Pin_RESET)
#define MAX6675_CS_SET() GPIO_WritePin(CW_GPIOA, MAX6675_CS_GPIO, GPIO_Pin_SET)

/**
 * @description: MAX6675 gpio 初始化
 * @return {*}
 */
void MAX6675_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 使能GPIOA B的配置时钟
    __RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.IT = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pins = MAX6675_CLK_GPIO | MAX6675_CS_GPIO;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_Init(CW_GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.IT = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pins = MAX6675_MISO_GPIO;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_Init(CW_GPIOA, &GPIO_InitStruct);

    elog_i(TAG, "MAX6675 GPIO Init!");
}

/**
 * @description: 读取max6675的温度
 * @return {*} 整数部分温度
 */
uint16_t MAX6675_Read(void)
{
    uint16_t value = 0;
    uint16_t value_dec = 0;
    MAX6675_CS_SET(); // 关闭MAX6675
    FirmwareDelay(10);
    MAX6675_CS_CLR(); // 置低，使能MAX6675
    FirmwareDelay(10);
    MAX6675_CLK_SET();
    for (uint32_t i = 16; i > 0; i--) // 获取16位MSB
    {
        FirmwareDelay(10);
        MAX6675_CLK_CLR();
        value = value << 1; // 左移
        if (MAX6675_MISO_GET() == GPIO_Pin_SET)
        {
            value |= 0x0001;
        }
        else
        {
            value &= 0xffff;
        }
        MAX6675_CLK_SET();
    }
    MAX6675_CS_SET();   // 关闭MAX6675
    value = value << 1; // 去掉第15位
    value = value >> 4; // 去掉第0~2位
    value_dec = value * 10;
    value = value / 4;         // MAX6675最大数值为1023.75，而AD精度为12位，即2的12次方为4096，转换对应数，故要除4；
    value_dec = value_dec / 4; // 小数 与上述同理
    if (value >= 1024)
        value = 1024;
    elog_i(TAG, "tempvalue:%d.%d", value, value_dec);
    return value;
}