#include "cw32f030_uart.h"
#include "cw32f030_rcc.h"
#include "cw32f030_gpio.h"
#include "uart.h"

#include <stdio.h>
//==================================================================================================
//  实现功能: 串口初始化
//  函数说明: UART1_Iint 
//  函数备注: 
//--------------------------------------------------------------------------------------------------
//  |   -   |   -   |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   
//================================================================================================== 
void UART1_Iint(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
    __RCC_UART1_CLK_ENABLE(); 
    __RCC_GPIOA_CLK_ENABLE();

    PA08_AFx_UART1TXD();// 串口复用
	PA09_AFx_UART1RXD(); 
	
    GPIO_InitStructure.Pins = GPIO_PIN_8; //PA8
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
    GPIO_Init(CW_GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.Pins = GPIO_PIN_9; //PA9
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT_PULLUP;
    GPIO_Init(CW_GPIOA, &GPIO_InitStructure);
		

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_Over = USART_Over_16;
    USART_InitStructure.USART_Source = USART_Source_PCLK;
    USART_InitStructure.USART_UclkFreq = 64000000;
    USART_InitStructure.USART_StartBit = USART_StartBit_FE;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(CW_UART1, &USART_InitStructure); 
	
    //使能UARTx RC中断
    USART_ITConfig(CW_UART1, USART_IT_RC, ENABLE);
    //优先级，无优先级分组
    NVIC_SetPriority(UART1_IRQn, 0);
    //UARTx中断使能
    NVIC_EnableIRQ(UART1_IRQn);

}

/**
 * @brief Retargets the C library printf function to the USART.
 *
 */
#ifdef __GNUC__
int _write(int fd, char* ptr, int len)
{
    for(int i = 0; i < len; i++)
    {
        USART_SendData_8bit(CW_UART1, ptr[i]);
        while (USART_GetFlagStatus(CW_UART1, USART_FLAG_TXE) == RESET);
    }
    return len;
}
#endif

#ifdef __CC_ARM
int fputc(int ch, FILE *f)
{
    USART_SendData_8bit(CW_UART1, ((uint8_t*))ch);
    while (USART_GetFlagStatus(CW_UART1, USART_FLAG_TXE) == RESET);
    return ch;
}
#endif