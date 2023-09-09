#include <stdint.h>
#include "cw32f030_gpio.h"
#include "cw32f030_rcc.h"

#include "multi_button.h"
#include "user_gui.h"
#include "ec11.h"
#include "elog.h"

#define TAG "EC11"

#define EC11_A GPIO_PIN_8       //PB8
#define EC11_B GPIO_PIN_9       //PB9 
#define EC11_KEY GPIO_PIN_7     //PB7

enum Button_IDs
{
    btn1_id,
};
static struct Button button_ec11;

static uint8_t EC11_flag = 0;
static uint8_t EC11_1_flag = 0;
static uint8_t EC11_2_flag = 0;

static uint8_t read_button_GPIO(uint8_t button_id)
{
    // you can share the GPIO read function with multiple Buttons
    switch (button_id)
    {
    case btn1_id:
        return GPIO_ReadPin(CW_GPIOB, EC11_KEY);
    default:
        return 0;
    }
}

static void button_callback(void *button)
{
    uint32_t btn_event_val;

    btn_event_val = get_button_event((struct Button *)button);

    switch (btn_event_val)
    {
        // case PRESS_DOWN:
        //     elog_i(TAG, "---> key1 press down! <---");
        //     break;

        // case PRESS_UP:
        //     elog_i(TAG, "***> key1 press up! <***");
        //     break;

        // case PRESS_REPEAT:
        //     elog_i(TAG, "---> key1 press repeat! <---");
        //     elog_i(TAG, "repeat count: %d", ((struct Button *)button)->repeat);
        //     break;

    case SINGLE_CLICK:
        elog_i(TAG, "---> key1 single click! <---");
        break;

    case DOUBLE_CLICK:
        elog_i(TAG, "***> key1 double click! <***");
        break;

    case LONG_PRESS_START:
        elog_i(TAG, "---> key1 long press start! <---");
        break;

    // case LONG_PRESS_HOLD:
    //     elog_i(TAG, "***> key1 long press hold! <***");
    //     break;
    default:
        elog_w(TAG, "Unregistered event");
        break;
    }
}

void EC11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 使能GPIOB的配置时钟
    __RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.IT = GPIO_IT_RISING | GPIO_IT_FALLING;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT_PULLUP;
    GPIO_InitStruct.Pins = EC11_A;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.IT = GPIO_IT_NONE;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT_PULLUP;
    GPIO_InitStruct.Pins = EC11_B | EC11_KEY;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_Init(CW_GPIOB, &GPIO_InitStruct);

    // 清除PB中断标记并使能NVIC
    GPIOB_INTFLAG_CLR(bv8);
    NVIC_EnableIRQ(GPIOB_IRQn);
    elog_i(TAG, "EC11 GPIO Init!");

    // 初始化按键对象
    button_init(&button_ec11, read_button_GPIO, 0, btn1_id);
    // 注册按钮事件回调函数
    //  button_attach(&button_1, PRESS_DOWN,       button_callback);
    //  button_attach(&button_1, PRESS_UP,         button_callback);
    //  button_attach(&button_1, PRESS_REPEAT,     button_callback);
    button_attach(&button_ec11, SINGLE_CLICK, button_callback);     //  单次按键事件
    button_attach(&button_ec11, DOUBLE_CLICK, button_callback);     //  双击按键事件
    button_attach(&button_ec11, LONG_PRESS_START, button_callback); //  长按按键事件
    // button_attach(&button_1, LONG_PRESS_HOLD,  button_callback);
    // 启动按键
    button_start(&button_ec11);

    elog_i(TAG, "EC11 Init!");
}

void EC11_HandlerCallBack(void)
{
    if (CW_GPIOB->ISR_f.PIN8)
    {
        // 只要处理一个脚的外部中断--上升沿&下降沿
        GPIO_PinState alv = GPIO_ReadPin(CW_GPIOB, EC11_A);
        GPIO_PinState blv = GPIO_ReadPin(CW_GPIOB, EC11_B);

        if (EC11_flag == 0 && alv == GPIO_Pin_RESET)
        {
            EC11_1_flag = blv;
            EC11_flag = 1;
        }

        if (EC11_flag && alv)
        {
            EC11_2_flag = !blv; // 取反是因为 alv,blv必然异步，一高一低。
            if (EC11_1_flag && EC11_2_flag)
            {
                target_temp_step_down();
                // elog_i(TAG, "right on");
            }
            if (EC11_1_flag == 0 && EC11_2_flag == 0)
            {
                target_temp_step_up();
                // elog_i(TAG, "left on");
            }
            EC11_flag = 0;
        }

        GPIOB_INTFLAG_CLR(bv8);
    }
}