/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2023-07-28 13:46:14
 * @LastEditors: tangwc
 * @LastEditTime: 2023-09-02 13:30:47
 * @FilePath: \2.Firmware\Core\src\app_main.c
 * 
 *  Copyright (c) 2023 by tangwc, All Rights Reserved. 
 */
#include "app_main.h"
#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"
#include "cw32f030_adc.h"

#include "uart.h"
#include "atimer.h"
#include "adc.h"
#include "ec11.h"
#include "btimer.h"
#include "oled.h"
#include "user_gui.h"
#include "max6675.h"
#include "elog.h"

#define TAG "app_main"


typedef struct
{
    char szVersion[32];    // 软件版本
    char szBuildDate[32];  // 程序编译日期
    char szBuildTime[32];  // 程序编译时间
}AppInfo_t;


const AppInfo_t sg_tAppInfo =
{
    "MiniHeating_V0.1.0",
    __DATE__,
    __TIME__,
};

static void elog_system_init(void)
{
    /*init 初始化*/
    elog_init();
    /*配置不同输出基本的输出信息*/
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_DIR | ELOG_FMT_FUNC | ELOG_FMT_LINE);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_DIR | ELOG_FMT_FUNC | ELOG_FMT_LINE);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_DIR | ELOG_FMT_FUNC | ELOG_FMT_LINE);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL | ELOG_FMT_TAG);
    /*elog开始*/
    elog_start();

    /*打印版本信息*/
    elog_i(TAG,"Version  : %s",sg_tAppInfo.szVersion);
    elog_i(TAG,"buildDate  : %s",sg_tAppInfo.szBuildDate);
    elog_i(TAG,"buildTime  : %s",sg_tAppInfo.szBuildTime);
}

static void bsp_init(void)
{   
    // 初始化外部高速晶振 主频64mhz
    RCC_Configuration();
    // 串口log 初始化
    UART1_Iint();
    elog_system_init();
    // pwm外设 初始化
    ATIMER_init();
    // 定时器 初始化
    BTIMER_init();
    // adc 初始化
    ADC_init();
    // ec11 初始化
    EC11_Init();
    // oled 初始化
    OLED_Init();
    // max6675 初始化
    MAX6675_Init();
}

int main(void)
{
    bsp_init();
    while (Transitions_logo() == 0);
    main_gui_show();
    OLED_Display();
    while (1)
    {   
        refresh_target_temp();
        refresh_pwr_Voltage();
        OLED_Display();
        ADC_SoftwareStartConvCmd(ENABLE);	//启动下一次adc转换
        delay1ms(500);
    }
    return 0;
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
