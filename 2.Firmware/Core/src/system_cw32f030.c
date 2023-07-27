

/******************************************************************************
 * Include files                                                              *
 ******************************************************************************/
#include "cw32f030.h"
#include "system_cw32f030.h"
#include "cw32f030_rcc.h"
#include "cw32f030_flash.h"

#define FLASH_MEMORY_BASE 0x00000000UL /*!< FLASH(up to 64 KB) base address in the alias region  */
#define SRAM_MEMORY_BASE 0x20000000UL  /*!< SRAM1(8 KB) base address in the alias region         */
#define VECT_TAB_OFFSET 0x00002000UL

/******************************************************************************
 ** System Clock Frequency (Core Clock) Variable according CMSIS
 ******************************************************************************/

uint32_t SystemCoreClock = 8000000;

void SystemCoreClockUpdate(void) // Update SystemCoreClock variable
{
    SystemCoreClock = 8000000;
}

/**
 ******************************************************************************
 ** \brief  Setup the microcontroller system. Initialize the System and update
 ** the SystemCoreClock variable.
 **
 ** \param  none
 ** \return none
 ******************************************************************************/
void SystemInit(void)
{
    // Load TrimCode in this.
    //...
#ifdef VECT_TAB_REMAP
    SCB->VTOR = FLASH_MEMORY_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif
    SystemCoreClockUpdate();
    CW_SYSCTRL->HSI_f.TRIM = *((volatile uint16_t *)RCC_HSI_TRIMCODEADDR);
    CW_SYSCTRL->LSI_f.TRIM = *((volatile uint16_t *)RCC_LSI_TRIMCODEADDR);
    // Init Hide thing.
    //...
}

/******************************************************************************
 * Internal Funcation                                                          *
 ******************************************************************************/

/**
 * \brief   delay1ms
 *          delay approximately 1ms.
 * \param   [in]  u32Cnt
 * \retval  void
 */
void delay1ms(uint32_t u32Cnt)
{
    uint32_t u32end;

    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

    while (u32Cnt-- > 0)
    {
        SysTick->VAL = 0;
        u32end = 0x1000000 - SystemCoreClock / 1000;
        while (SysTick->VAL > u32end)
        {
            ;
        }
    }

    SysTick->CTRL = (SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk));
}

/**
 * \brief   delay100us
 *          delay approximately 100us.
 * \param   [in]  u32Cnt
 * \retval  void
 */
void delay100us(uint32_t u32Cnt)
{
    uint32_t u32end;

    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

    while (u32Cnt-- > 0)
    {
        SysTick->VAL = 0;

        u32end = 0x1000000 - SystemCoreClock / 10000;
        while (SysTick->VAL > u32end)
        {
            ;
        }
    }

    SysTick->CTRL = (SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk));
}

/**
 * \brief   delay10us
 *          delay approximately 10us.
 * \param   [in]  u32Cnt
 * \retval  void
 */
void delay10us(uint32_t u32Cnt)
{
    uint32_t u32end;

    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

    while (u32Cnt-- > 0)
    {
        SysTick->VAL = 0;

        u32end = 0x1000000 - SystemCoreClock / 100000;
        while (SysTick->VAL > u32end)
        {
            ;
        }
    }

    SysTick->CTRL = (SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk));
}

void FirmwareDelay(uint32_t DlyCnt)
{
    volatile uint32_t thisCnt = DlyCnt;
    while (thisCnt--)
    {
        ;
    }
}

/**
 * \brief   clear memory
 *
 * \param   [in]  start addr
 * \param   [in]  memory size(byte)
 * \retval  void
 */
void MemClr(void *pu8Address, uint32_t u32Count)
{
    uint8_t *pu8Addr = (uint8_t *)pu8Address;

    if (NULL == pu8Addr)
    {
        return;
    }

    while (u32Count--)
    {
        *pu8Addr++ = 0;
    }
}

void RCC_Configuration(void)
{
    // 内部高速时钟使能

    // /* 0. HSI使能并校准 */
    // RCC_HSI_Enable(RCC_HSIOSC_DIV6);

    // /* 1. 设置HCLK和PCLK的分频系数　*/
    // RCC_HCLKPRS_Config(RCC_HCLK_DIV1);
    // RCC_PCLKPRS_Config(RCC_PCLK_DIV1);

    // /* 2. 使能PLL，通过PLL倍频到64MHz */
    // RCC_PLL_Enable(RCC_PLLSOURCE_HSI, 8000000, 8);     // HSI 默认输出频率8MHz
    // // RCC_PLL_OUT();  //PC13脚输出PLL时钟

    // ///< 当使用的时钟源HCLK大于24M,小于等于48MHz：设置FLASH 读等待周期为2 cycle
    // ///< 当使用的时钟源HCLK大于48MHz：设置FLASH 读等待周期为3 cycle
    //
    // FLASH_SetLatency(FLASH_Latency_3);

    // /* 3. 时钟切换到PLL */
    // RCC_SysClk_Switch(RCC_SYSCLKSRC_PLL);
    // RCC_SystemCoreClockUpdate(64000000);

    /* 1.开启外部高速时钟HSE 根据实际情况填写*/
    RCC_HSE_Enable(RCC_HSE_MODE_OSC, 8000000, RCC_HSE_DRIVER_NORMAL, RCC_HSE_FLT_CLOSE);

    /* 2.使能PLL，PLL时钟来源为HSE*/
    RCC_PLL_Enable(RCC_PLLSOURCE_HSEOSC, 8000000, RCC_PLL_MUL_8);
    // RCC_PLL_OUT();  //PC13脚输出PLL时钟

    ///< 当使用的时钟源HCLK大于24M,小于等于48MHz：设置FLASH 读等待周期为2 cycle
    ///< 当使用的时钟源HCLK大于48MHz：设置FLASH 读等待周期为3 cycle
    __RCC_FLASH_CLK_ENABLE();
    FLASH_SetLatency(FLASH_Latency_3);

    // 延迟足够，保证时钟稳定
    FirmwareDelay(100000);

    /* 切换系统时钟成功*/
    if (RCC_SysClk_Switch(RCC_SYSCLKSRC_PLL) == 0x0U)
    {
        RCC_HSI_Disable(); // 切换时钟到PLL后关闭源时钟HSI
        FirmwareDelay(400000);
    }
    RCC_SystemCoreClockUpdate(64000000);
}
