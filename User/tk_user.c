/**
 ******************************************************************************
 * @file    User/tk_user.c
 * @author  HUGE-IC Application Team
 * @version V1.0.0
 * @date    05-20-2022
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2022 TAIXIN-IC</center></h2>
 *
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "my_gpio.h"  // 自定义的、使用到的引脚
#include "rf_recv.h"  // RF315解码和RF接收引脚相关的函数
#include "send_key.h" // 发送键值的引脚和相关函数

#include "rf_learn.h" // rf学习相关的函数

#include "tmr2.h"
#include "tmr3.h"
#include "rf_scan.h" // 包含了长短按信息的定义

#include "key_conversion.h" // 包含了触摸按键对应的键值

/** @addtogroup Template_Project
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Touchkey  debug uart init function
 * @param  None
 * @retval None
 */
xdata u16 led_data[1];                           // LED显示buff
#define LED_DIS_FREQ (1000000 / 16 / 8 / 60 - 1) // 一个CON的显示时间60Hz    60-1=1M/(LED_COMCON*16*LED_TIME_CON)

unsigned int xdata display_data = 0;

/**
 * @brief  调试接口IO初始化函数.
 * @param  None
 * @retval None
 */
#if TK_DEBUG_EN
void debug_gpio_config(void)
{
#if 1
    P2_MD0 &= ~GPIO_P21_MODE_SEL(0x03);
    P2_MD0 |= GPIO_P21_MODE_SEL(0x01);
    FOUT_S21 = GPIO_FOUT_UART1_TX;
#else
    P2_MD1 &= ~GPIO_P27_MODE_SEL(0x03);
    P2_MD1 |= GPIO_P27_MODE_SEL(0x01);
    FOUT_S27 = GPIO_FOUT_UART1_TX;
#endif
}
#endif

/**
 * @brief  LED INIT FUNCTION
 * @param  None
 * @retval None
 */
// 在开发板上：
// P2_7连接到四个蓝色LED的负极的公共端
// P0_3连接到LED1正极
// P3_0连接到LED2正极
// P1_0连接到LED3正极
// P0_7连接到LED4正极
void led_init(void)
{
    // 配置COM0   P27为输出模式
    // P2_MD1，P2的工作模式寄存器1
    P2_MD1 &= ~GPIO_P27_MODE_SEL(0x3); // 清零P2_MD1寄存器的6-7位
    P2_MD1 |= GPIO_P27_MODE_SEL(0x1);  // P2_MD1的6~7位设置为0x01，这样会将P2的第7个IO设置为输出模式

    // 配置IO的驱动能力
    // P2的第7个IO的驱动电流为8mA
    P2_DRV7 = GPIO_P27_DRV_SEL(0x1);

    // 配置SEG0-SEG4 P03\P30\P10\P07-输出模式
    // P0_3、P3_0、P1_0和P0_1配置为输出模式
    // 下面是先清零，再置位的操作
    P0_MD0 &= ~GPIO_P03_MODE_SEL(0x3);
    P3_MD0 &= ~GPIO_P30_MODE_SEL(0x3);
    P1_MD0 &= ~GPIO_P10_MODE_SEL(0x3);
    P0_MD1 &= ~GPIO_P07_MODE_SEL(0x3);

    P0_MD0 |= GPIO_P03_MODE_SEL(0x1);
    P3_MD0 |= GPIO_P30_MODE_SEL(0x1);
    P1_MD0 |= GPIO_P10_MODE_SEL(0x1);
    P0_MD1 |= GPIO_P07_MODE_SEL(0x1);

    // SEG0-SEG4 P03\P30\P10\P07
    // 配置P0_3、P3_0、P1_0和P0_1的输出功能，驱动LED（应该是连接到了片上外设LED模块，这个模块是个控制器）
    FOUT_S03 = GPIO_FOUT_LED_SEG0;
    FOUT_S30 = GPIO_FOUT_LED_SEG1;
    FOUT_S10 = GPIO_FOUT_LED_SEG2;
    FOUT_S07 = GPIO_FOUT_LED_SEG3;

    // 配置P0_3、P3_0、P1_0和P0_1的输出电流，8mA
    P0_DRV3 = GPIO_P03_DRV_SEL(0x1);
    P3_DRV0 = GPIO_P30_DRV_SEL(0x1);
    P1_DRV0 = GPIO_P10_DRV_SEL(0x1);
    P0_DRV7 = GPIO_P07_DRV_SEL(0x1);

    // COM0 P27
    // 配置P2_7的输出功能，连接到LED的公共端0
    FOUT_S27 = GPIO_FOUT_LED_COM0;

    CLK_CON2 |= CLK_LED_EN(0x1); // 打开LED模块的时钟（打开片上外设LED模块的时钟）

    // 0x55表示允许访问和设置 wdt_psr（在WDT_CON中，psr表示看门狗定时时间）
    WDT_KEY = 0x55;                // 操作IO_MAP寄存器需要写KEY
    IO_MAP |= MAP_LED_DMA_EN(0x1); // LED DMA使能
    WDT_KEY = 0xBB;                // 写入一个错误的数值，恢复该寄存器的写保护

    // 打开LED模块的时钟
    CLK_CON2 |= 0x10;

    LED_SEGCONL = LED_SEG0_EN(0x1) | // SEG0 使能
                  LED_SEG1_EN(0x1) | // SEG0 使能
                  LED_SEG2_EN(0x1) | // SEG0 使能
                  LED_SEG3_EN(0x1);  // SEG0 使能
    LED_COMCON = LED_COM0_EN(0x1);   // COM1 使能

    LED_DMAADRH = LED_DMA_ADDR_H((((u16)led_data) >> 8) & 0xFF); // LED数据储存首地址的高8位
    LED_DMAADRL = LED_DMA_ADDR_L((((u16)led_data) >> 0) & 0xFF); // LED数据储存首地址的低8位
    LED_TIMECON = LED_SCAN_TIME(LED_DIS_FREQ);                   // 扫描到某个com或seg时点亮的时间,步长为32微秒
    LED_CON = LED_COM_SEG_SEL(0x0) |                             // 扫描方式选择com扫描
              LED_EN(0x1);                                       // LED使能,使能之后从DMAADDRH和DMAADDRL的地址开始拿数据扫描点亮,用户将数据写到对应的地址即可

    led_data[0] = 0x00000f00; // 一开始让四个LED全部点亮
    // led_data[0] = 0x00000000; // 一开始让四个LED全部熄灭
}

/**
 * @brief  用户代码初始化函数接口.
 * @param  None
 * @retval None
 */
void user_init(void)
{
    // led_init(); // 初始化LED相关的引脚

    rfin_init(); // RF315接收引脚初始化，这里也初始化了tmr0

    // p12_output_config(); // 测试用，P12初始化，配置为输出模式

    send_keyval_pin_init();   // 初始化键值的发送引脚
    send_keyval_timer_init(); // 初始化发送键值的引脚所使用到的定时器，定时器默认关闭

    tmr0_enable(); // 打开采集RF信号的定时器
    // tmr1_enable(); // 打开发送键值的引脚所使用到的定时器，测试用，看看定时器中断是否按配置的时间触发

    tmr2_config(); // 上电5s内的"学习"所使用的定时器
    tmr3_config(); // 配置定时器，每10ms产生一次中断，对应的计数值+1，用来判断按键的短按、长按和持续
    tmr4_config(); // 打开识别遥控器双击所需要的定时器

    // p01_output_config(); // 开发板LED6对应的引脚初始化
    // p26_output_config(); // 开发板LED7对应的引脚初始化
}

void led_display(void)
{
    led_data[0] = display_data;
}

#ifdef DEVELOPMENT_BOARD // 开发板上对应额触摸按键函数接口
/**
 * @brief  用户代码循环执行函数接口.（触摸按键扫描函数接口）
 * @param  None
 * @retval None
 */
void user_handle(void)
{
    volatile unsigned long int KeyOnOutput = __tk_key_flag; // __tk_key_flag单次按键标志

    static volatile unsigned long int KeyOld = 0; // 用于存放长按时，对应的按键

    // 是否长按的标志，0--否，1--是
    // 需要保存长按的状态，表示定时器一直在发送键值
    static unsigned char long_touch_flag = 0; // 上一次扫描到的按键是否为长按的标志

    static volatile u32 cnt = 0; // 长按计数值

    u32 i = 0; // 循环计数值

    u16 send_data = 0; // 要发送的带有键值的数据

    if (KeyOnOutput && 0 == long_touch_flag && cnt < TK_LONG_KEY_TIME) // 如果有按键按下（并且之前扫描到的不是长按）
    {
        // 通过循环判断是否为长按
        for (i = 0; i < 500000; i++) // 实际上用不到这么大的数字，到了设定的长按时间就会退出
        {
            // 按键扫描函数，使用了库里面的接口（闭源库）
            // 这个函数会更新__tk_key_flag的值
            delay_ms(10);
            __tk_scan();

            if (KeyOnOutput == __tk_key_flag)
            {
                // 如果按键键值一致，说明按键一直被按下，未松开
                cnt++;
            }
            else
            {
                // 如果按键键值不一致或是松开了按键
                // cnt = 0;
                break;
            }
            if (cnt > TK_LONG_KEY_TIME) // 如果长按超过了这里设置的时间
            {
                long_touch_flag = 1; // 标记为长按
                break;
            }
        }

        if (cnt > TK_LONG_KEY_TIME)
        {
            long_touch_flag = 1; // 标记为长按
        }
        else
        {
            // 如果是短按
            cnt = 0; // 清除长按的计数值

            // 判断当前触摸的键值，做对应的操作
            if (TK_CH0_VALIB == KeyOnOutput)
            {
                // 如果开发板的TK1被触摸
                display_data ^= 0x0100; // 对应的LED状态取反，后面会调用led_display()刷新
                for (i = 0; i < 300; i++)
                {
                    delay_ms(1);
                    __tk_scan();
                    KeyOnOutput = __tk_key_flag;

                    if (TK_CH0_VALIB == KeyOnOutput)
                    {
                        // 如果检测到是同一个按键按下，说明是双击
                        // 等待手指松开按键，不然发送完带有双击状态的键值后会再发送一次单击状态的键值
                        while (0 != KeyOnOutput) 
                        {
                            __tk_scan();
                            KeyOnOutput = __tk_key_flag;
                        }

                        send_status_keyval(KEY_PRESS_DOUBLECLICK, KEY_TK1);
                        return;
                    }
                }

                // 如果不是同一个按键按下或是没有值，说明是短按
                send_status_keyval(KEY_PRESS_SHORT, KEY_TK1); // 发送带有状态的键值
            }
            else if (TK_CH4_VALIB == KeyOnOutput)
            {
                // 如果开发板的TK2被触摸
                display_data ^= 0x0200; // 对应的LED状态取反，后面会调用led_display()刷新
                for (i = 0; i < 300; i++)
                {
                    delay_ms(1);
                    __tk_scan();
                    KeyOnOutput = __tk_key_flag;

                    if (TK_CH4_VALIB == KeyOnOutput)
                    {
                        // 如果检测到是同一个按键按下，说明是双击
                        // 等待手指松开按键，不然发送完带有双击状态的键值后会再发送一次单击状态的键值
                        while (0 != KeyOnOutput) 
                        {
                            __tk_scan();
                            KeyOnOutput = __tk_key_flag;
                        }
                        
                        send_status_keyval(KEY_PRESS_DOUBLECLICK, KEY_TK2);
                        return;
                    }
                }

                // 如果不是同一个按键按下或是没有值，说明是短按
                send_status_keyval(KEY_PRESS_SHORT, KEY_TK2); // 发送带有状态的键值
            }
            else if (TK_CH5_VALIB == KeyOnOutput)
            {
                // 如果开发板的TK3被触摸
                display_data ^= 0x0400; // 对应的LED状态取反，后面会调用led_display()刷新
                for (i = 0; i < 300; i++)
                {
                    delay_ms(1);
                    __tk_scan();
                    KeyOnOutput = __tk_key_flag;

                    if (TK_CH5_VALIB == KeyOnOutput)
                    {
                        // 如果检测到是同一个按键按下，说明是双击
                        // 等待手指松开按键，不然发送完带有双击状态的键值后会再发送一次单击状态的键值
                        while (0 != KeyOnOutput) 
                        {
                            __tk_scan();
                            KeyOnOutput = __tk_key_flag;
                        }

                        send_status_keyval(KEY_PRESS_DOUBLECLICK, KEY_TK3);
                        return;
                    }
                }

                // 如果不是同一个按键按下或是没有值，说明是短按
                send_status_keyval(KEY_PRESS_SHORT, KEY_TK3); // 发送带有状态的键值
            }
            else if (TK_CH6_VALIB == KeyOnOutput)
            {
                // 如果开发板的TK4被触摸
                display_data ^= 0x0800; // 对应的LED状态取反，后面会调用led_display()刷新
                for (i = 0; i < 300; i++)
                {
                    delay_ms(1);
                    __tk_scan();
                    KeyOnOutput = __tk_key_flag;

                    if (TK_CH6_VALIB == KeyOnOutput)
                    {
                        // 如果检测到是同一个按键按下，说明是双击
                        // 等待手指松开按键，不然发送完带有双击状态的键值后会再发送一次单击状态的键值
                        while (0 != KeyOnOutput) 
                        {
                            __tk_scan();
                            KeyOnOutput = __tk_key_flag;
                        }

                        send_status_keyval(KEY_PRESS_DOUBLECLICK, KEY_TK4);
                        return;
                    }
                }

                // 如果不是同一个按键按下或是没有值，说明是短按
                send_status_keyval(KEY_PRESS_SHORT, KEY_TK4); // 发送带有状态的键值
            }
        }
    }

    if (1 == long_touch_flag && 0 != KeyOnOutput)
    {
        KeyOld = KeyOnOutput;

        // 如果之前是长按，现在还未松开
        // 判断当前触摸的键值，做对应的操作
        if (TK_CH0_VALIB == KeyOnOutput)
        {
            // 如果开发板的TK1被触摸
            if (cnt >= TK_LONG_KEY_TIME && cnt < 90)
            {
                display_data ^= 0x0100; // 对应的LED状态取反，后面会调用led_display()刷新
                led_display();          // LED状态更新显示
                send_status_keyval(KEY_PRESS_LONG, KEY_TK1);
            }

            // 若还不松开手，则每隔150ms发送一次带有持续状态信息的键值信号
            while (0 != KeyOnOutput)
            {
                delay_ms(10);
                cnt++;
                __tk_scan();
                KeyOnOutput = __tk_key_flag;

                if (cnt >= 87) // 原本是填90的，但是软件延时有误差，这里只能慢慢调试
                {
                    cnt = TK_LONG_KEY_TIME;
                    send_status_keyval(KEY_PRESS_CONTINUE, KEY_TK1);
                }
            }

            // display_data ^= 0x0100; // 对应的LED状态取反，后面会调用led_display()刷新
            // led_display();          // LED状态更新显示
        }
        else if (TK_CH4_VALIB == KeyOnOutput)
        {
            // 如果开发板的TK2被触摸
            if (cnt >= TK_LONG_KEY_TIME && cnt < 90)
            {
                display_data ^= 0x0200; // 对应的LED状态取反，后面会调用led_display()刷新
                led_display();          // LED状态更新显示
                send_status_keyval(KEY_PRESS_LONG, KEY_TK2);
            }

            // 若还不松开手，则每隔150ms发送一次带有持续状态信息的键值信号
            while (0 != KeyOnOutput)
            {
                delay_ms(10);
                cnt++;
                __tk_scan();
                KeyOnOutput = __tk_key_flag;

                if (cnt >= 87) // 原本是填90的，但是软件延时有误差，这里只能慢慢调试
                {
                    cnt = TK_LONG_KEY_TIME;
                    send_status_keyval(KEY_PRESS_CONTINUE, KEY_TK2);
                }
            }

            // display_data ^= 0x0200; // 对应的LED状态取反，后面会调用led_display()刷新
            // led_display();          // LED状态更新显示
        }
        else if (TK_CH5_VALIB == KeyOnOutput)
        {
            // 如果开发板的TK3被触摸
            if (cnt >= TK_LONG_KEY_TIME && cnt < 90)
            {
                display_data ^= 0x0400; // 对应的LED状态取反，后面会调用led_display()刷新
                led_display();          // LED状态更新显示
                send_status_keyval(KEY_PRESS_LONG, KEY_TK3);
            }

            // 若还不松开手，则每隔150ms发送一次带有持续状态信息的键值信号
            while (0 != KeyOnOutput)
            {
                delay_ms(10);
                cnt++;
                __tk_scan();
                KeyOnOutput = __tk_key_flag;

                if (cnt >= 87) // 原本是填90的，但是软件延时有误差，这里只能慢慢调试
                {
                    cnt = TK_LONG_KEY_TIME;
                    send_status_keyval(KEY_PRESS_CONTINUE, KEY_TK3);
                }
            }

            // display_data ^= 0x0400; // 对应的LED状态取反，后面会调用led_display()刷新
            // led_display();          // LED状态更新显示
        }
        else if (TK_CH6_VALIB == KeyOnOutput)
        {
            // 如果开发板的TK4被触摸
            if (cnt >= TK_LONG_KEY_TIME && cnt < 90)
            {
                display_data ^= 0x0800; // 对应的LED状态取反，后面会调用led_display()刷新
                led_display();          // LED状态更新显示
                send_status_keyval(KEY_PRESS_LONG, KEY_TK4);
            }

            // 若还不松开手，则每隔150ms发送一次带有持续状态信息的键值信号
            while (0 != KeyOnOutput)
            {
                delay_ms(10);
                cnt++;
                __tk_scan();
                KeyOnOutput = __tk_key_flag;

                if (cnt >= 87) // 原本是填90的，但是软件延时有误差，这里只能慢慢调试
                {
                    cnt = TK_LONG_KEY_TIME;
                    send_status_keyval(KEY_PRESS_CONTINUE, KEY_TK4);
                }
            }

            // display_data ^= 0x0800; // 对应的LED状态取反，后面会调用led_display()刷新
            // led_display();          // LED状态更新显示
        }
    }
    else if (1 == long_touch_flag && 0 == KeyOnOutput)
    {
        // 如果之前是长按，现在却松开了按键
        switch (KeyOld)
        {
        case TK_CH0_VALIB:
            send_status_keyval(KEY_PRESS_LOOSE, KEY_TK1);
            break;

        case TK_CH4_VALIB:
            send_status_keyval(KEY_PRESS_LOOSE, KEY_TK2);
            break;

        case TK_CH5_VALIB:
            send_status_keyval(KEY_PRESS_LOOSE, KEY_TK3);
            break;

        case TK_CH6_VALIB:
            send_status_keyval(KEY_PRESS_LOOSE, KEY_TK4);
            break;
        }

        long_touch_flag = 0; // 清除标志位
        cnt = 0;
        KeyOld = 0;
    }

#if 0
    if (1 == __tk_long_key_flag) // 如果长按按键（这个功能测试发现不行，不能用）
    {
        __tk_long_key_flag = 0;

        // 这里可以自己处理，添加自己需要的功能

        P11 = 0;
    }
#endif

    led_display(); // LED状态更新显示
}
#endif // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD // 目标电路板对应的触摸按键函数接口
/**
 * @brief  用户代码循环执行函数接口.（触摸按键扫描函数接口）
 * @param  None
 * @retval None
 */


#define send_sp (100)
void user_handle(void)
{
    volatile unsigned long int KeyOnOutput = __tk_key_flag; // __tk_key_flag单次按键标志

    static volatile unsigned long int KeyOld = 0; // 用于存放长按时，对应的按键

    // 是否长按的标志，0--否，1--是
    // 需要保存长按的状态，表示定时器一直在发送键值
    static unsigned char long_touch_flag = 0; // 上一次扫描到的按键是否为长按的标志

    static volatile u32 cnt = 0; // 长按计数值

    u32 i = 0; // 循环计数值

    u16 send_data = 0; // 要发送的带有键值的数据

    if (KeyOnOutput && 0 == long_touch_flag && cnt < TK_LONG_KEY_TIME) // 如果有按键按下（并且之前扫描到的不是长按）
    {
        // 通过循环判断是否为长按
        for (i = 0; i < 500000; i++) // 实际上用不到这么大的数字，到了设定的长按时间就会退出
        {
            // 按键扫描函数，使用了库里面的接口（闭源库）
            // 这个函数会更新__tk_key_flag的值
            delay_ms(10);
            __tk_scan();

            if (KeyOnOutput == __tk_key_flag)
            {
                // 如果按键键值一致，说明按键一直被按下，未松开
                cnt++;
            }
            else
            {
                // 如果按键键值不一致或是松开了按键
                // cnt = 0;
                break;
            }
            if (cnt > TK_LONG_KEY_TIME) // 如果长按超过了这里设置的时间
            {
                long_touch_flag = 1; // 标记为长按
                break;
            }
        }

        if (cnt > TK_LONG_KEY_TIME)
        {
            long_touch_flag = 1; // 标记为长按
        }
        else
        {
            // 如果是短按
            cnt = 0; // 清除长按的计数值

            // 判断当前触摸的键值，做对应的操作
            if (TK_CH24_VALIB == KeyOnOutput)
            {
                // 如果KEY1被触摸
                for (i = 0; i < send_sp; i++)
                {
                    delay_ms(1);
                    __tk_scan();
                    KeyOnOutput = __tk_key_flag;

                    if (TK_CH24_VALIB == KeyOnOutput)
                    {
                        // 如果检测到是同一个按键按下，说明是双击
                        // 等待手指松开按键，不然发送完带有双击状态的键值后会再发送一次单击状态的键值
                        while (0 != KeyOnOutput) 
                        {
                            __tk_scan();
                            KeyOnOutput = __tk_key_flag;
                        }

                        send_status_keyval(KEY_PRESS_DOUBLECLICK, KEY_TK1);
                        return;
                    }
                }

                // 如果不是同一个按键按下或是没有值，说明是短按
                send_status_keyval(KEY_PRESS_SHORT, KEY_TK1); // 发送带有状态的键值
            }
            else if (TK_CH3_VALIB == KeyOnOutput)
            {
                // 如果KEY2被触摸
                for (i = 0; i < send_sp; i++)
                {
                    delay_ms(1);
                    __tk_scan();
                    KeyOnOutput = __tk_key_flag;

                    if (TK_CH3_VALIB == KeyOnOutput)
                    {
                        // 如果检测到是同一个按键按下，说明是双击
                        // 等待手指松开按键，不然发送完带有双击状态的键值后会再发送一次单击状态的键值
                        while (0 != KeyOnOutput) 
                        {
                            __tk_scan();
                            KeyOnOutput = __tk_key_flag;
                        }

                        send_status_keyval(KEY_PRESS_DOUBLECLICK, KEY_TK2);
                        return;
                    }
                }

                // 如果不是同一个按键按下或是没有值，说明是短按
                send_status_keyval(KEY_PRESS_SHORT, KEY_TK2); // 发送带有状态的键值
            }
            else if (TK_CH2_VALIB == KeyOnOutput)
            {
                // 如果KEY3被触摸
                for (i = 0; i < send_sp; i++)
                {
                    delay_ms(1);
                    __tk_scan();
                    KeyOnOutput = __tk_key_flag;

                    if (TK_CH2_VALIB == KeyOnOutput)
                    {
                        // 如果检测到是同一个按键按下，说明是双击
                        // 等待手指松开按键，不然发送完带有双击状态的键值后会再发送一次单击状态的键值
                        while (0 != KeyOnOutput) 
                        {
                            __tk_scan();
                            KeyOnOutput = __tk_key_flag;
                        }

                        send_status_keyval(KEY_PRESS_DOUBLECLICK, KEY_TK3);
                        return;
                    }
                }

                // 如果不是同一个按键按下或是没有值，说明是短按
                send_status_keyval(KEY_PRESS_SHORT, KEY_TK3); // 发送带有状态的键值
            }
            else if (TK_CH0_VALIB == KeyOnOutput)
            {
                // 如果KEY4被触摸
                for (i = 0; i < send_sp; i++)
                {
                    delay_ms(1);
                    __tk_scan();
                    KeyOnOutput = __tk_key_flag;

                    if (TK_CH0_VALIB == KeyOnOutput)
                    {
                        // 如果检测到是同一个按键按下，说明是双击
                        // 等待手指松开按键，不然发送完带有双击状态的键值后会再发送一次单击状态的键值
                        while (0 != KeyOnOutput) 
                        {
                            __tk_scan();
                            KeyOnOutput = __tk_key_flag;
                        }

                        send_status_keyval(KEY_PRESS_DOUBLECLICK, KEY_TK4);
                        return;
                    }
                }

                // 如果不是同一个按键按下或是没有值，说明是短按
                send_status_keyval(KEY_PRESS_SHORT, KEY_TK4); // 发送带有状态的键值
            }
            else if (TK_CH9_VALIB == KeyOnOutput)
            {
                // 如果KEY5被触摸
                for (i = 0; i < send_sp; i++)
                {
                    delay_ms(1);
                    __tk_scan();
                    KeyOnOutput = __tk_key_flag;

                    if (TK_CH9_VALIB == KeyOnOutput)
                    {
                        // 如果检测到是同一个按键按下，说明是双击
                        // 等待手指松开按键，不然发送完带有双击状态的键值后会再发送一次单击状态的键值
                        while (0 != KeyOnOutput) 
                        {
                            __tk_scan();
                            KeyOnOutput = __tk_key_flag;
                        }

                        send_status_keyval(KEY_PRESS_DOUBLECLICK, KEY_TK5);
                        return;
                    }
                }

                // 如果不是同一个按键按下或是没有值，说明是短按
                send_status_keyval(KEY_PRESS_SHORT, KEY_TK5); // 发送带有状态的键值
            }
            else if (TK_CH10_VALIB == KeyOnOutput)
            {
                // 如果KEY6被触摸
                for (i = 0; i < send_sp; i++)
                {
                    delay_ms(1);
                    __tk_scan();
                    KeyOnOutput = __tk_key_flag;

                    if (TK_CH10_VALIB == KeyOnOutput)
                    {
                        // 如果检测到是同一个按键按下，说明是双击
                        // 等待手指松开按键，不然发送完带有双击状态的键值后会再发送一次单击状态的键值
                        while (0 != KeyOnOutput) 
                        {
                            __tk_scan();
                            KeyOnOutput = __tk_key_flag;
                        }

                        send_status_keyval(KEY_PRESS_DOUBLECLICK, KEY_TK6);
                        return;
                    }
                }

                // 如果不是同一个按键按下或是没有值，说明是短按
                send_status_keyval(KEY_PRESS_SHORT, KEY_TK6); // 发送带有状态的键值
            }
        }
    }

    if (1 == long_touch_flag && 0 != KeyOnOutput)
    {
        KeyOld = KeyOnOutput;

        // 如果之前是长按，现在还未松开
        // 判断当前触摸的键值，做对应的操作
        if (TK_CH24_VALIB == KeyOnOutput)
        {
            // 如果KEY1被触摸
            if (cnt >= TK_LONG_KEY_TIME && cnt < 90)
            {
                send_status_keyval(KEY_PRESS_LONG, KEY_TK1);
            }

            // 若还不松开手，则每隔150ms发送一次带有持续状态信息的键值信号
            while (0 != KeyOnOutput)
            {
                delay_ms(10);
                cnt++;
                __tk_scan();
                KeyOnOutput = __tk_key_flag;

                if (cnt >= 87) // 原本是填90的，但是软件延时有误差，这里只能慢慢调试
                {
                    cnt = TK_LONG_KEY_TIME;
                    send_status_keyval(KEY_PRESS_CONTINUE, KEY_TK1);
                }
            }
        }
        else if (TK_CH3_VALIB == KeyOnOutput)
        {
            // 如果KEY2被触摸
            if (cnt >= TK_LONG_KEY_TIME && cnt < 90)
            {
                send_status_keyval(KEY_PRESS_LONG, KEY_TK2);
            }

            // 若还不松开手，则每隔150ms发送一次带有持续状态信息的键值信号
            while (0 != KeyOnOutput)
            {
                delay_ms(10);
                cnt++;
                __tk_scan();
                KeyOnOutput = __tk_key_flag;

                if (cnt >= 87) // 原本是填90的，但是软件延时有误差，这里只能慢慢调试
                {
                    cnt = TK_LONG_KEY_TIME;
                    send_status_keyval(KEY_PRESS_CONTINUE, KEY_TK2);
                }
            }

            // display_data ^= 0x0200; // 对应的LED状态取反，后面会调用led_display()刷新
            // led_display();          // LED状态更新显示
        }
        else if (TK_CH2_VALIB == KeyOnOutput)
        {
            // 如果KEY3被触摸
            if (cnt >= TK_LONG_KEY_TIME && cnt < 90)
            {
                // display_data ^= 0x0400; // 对应的LED状态取反，后面会调用led_display()刷新
                // led_display();          // LED状态更新显示
                send_status_keyval(KEY_PRESS_LONG, KEY_TK3);
            }

            // 若还不松开手，则每隔150ms发送一次带有持续状态信息的键值信号
            while (0 != KeyOnOutput)
            {
                delay_ms(10);
                cnt++;
                __tk_scan();
                KeyOnOutput = __tk_key_flag;

                if (cnt >= 87) // 原本是填90的，但是软件延时有误差，这里只能慢慢调试
                {
                    cnt = TK_LONG_KEY_TIME;
                    send_status_keyval(KEY_PRESS_CONTINUE, KEY_TK3);
                }
            }

            // display_data ^= 0x0400; // 对应的LED状态取反，后面会调用led_display()刷新
            // led_display();          // LED状态更新显示
        }
        else if (TK_CH0_VALIB == KeyOnOutput)
        {
            // 如果KEY4被触摸
            if (cnt >= TK_LONG_KEY_TIME && cnt < 90)
            {
                // display_data ^= 0x0800; // 对应的LED状态取反，后面会调用led_display()刷新
                // led_display();          // LED状态更新显示
                send_status_keyval(KEY_PRESS_LONG, KEY_TK4);
            }

            // 若还不松开手，则每隔150ms发送一次带有持续状态信息的键值信号
            while (0 != KeyOnOutput)
            {
                delay_ms(10);
                cnt++;
                __tk_scan();
                KeyOnOutput = __tk_key_flag;

                if (cnt >= 87) // 原本是填90的，但是软件延时有误差，这里只能慢慢调试
                {
                    cnt = TK_LONG_KEY_TIME;
                    send_status_keyval(KEY_PRESS_CONTINUE, KEY_TK4);
                }
            }

            // display_data ^= 0x0800; // 对应的LED状态取反，后面会调用led_display()刷新
            // led_display();          // LED状态更新显示
        }
        else if (TK_CH9_VALIB == KeyOnOutput)
        {
            // 如果KEY5被触摸
            if (cnt >= TK_LONG_KEY_TIME && cnt < 90)
            {
                // display_data ^= 0x0800; // 对应的LED状态取反，后面会调用led_display()刷新
                // led_display();          // LED状态更新显示
                send_status_keyval(KEY_PRESS_LONG, KEY_TK5);
            }

            // 若还不松开手，则每隔150ms发送一次带有持续状态信息的键值信号
            while (0 != KeyOnOutput)
            {
                delay_ms(10);
                cnt++;
                __tk_scan();
                KeyOnOutput = __tk_key_flag;

                if (cnt >= 87) // 原本是填90的，但是软件延时有误差，这里只能慢慢调试
                {
                    cnt = TK_LONG_KEY_TIME;
                    send_status_keyval(KEY_PRESS_CONTINUE, KEY_TK5);
                }
            }

            // display_data ^= 0x0800; // 对应的LED状态取反，后面会调用led_display()刷新
            // led_display();          // LED状态更新显示
        }

        else if (TK_CH10_VALIB == KeyOnOutput)
        {
            // 如果KEY6被触摸
            if (cnt >= TK_LONG_KEY_TIME && cnt < 90)
            {
                // display_data ^= 0x0800; // 对应的LED状态取反，后面会调用led_display()刷新
                // led_display();          // LED状态更新显示
                send_status_keyval(KEY_PRESS_LONG, KEY_TK6);
            }

            // 若还不松开手，则每隔150ms发送一次带有持续状态信息的键值信号
            while (0 != KeyOnOutput)
            {
                delay_ms(10);
                cnt++;
                __tk_scan();
                KeyOnOutput = __tk_key_flag;

                if (cnt >= 87) // 原本是填90的，但是软件延时有误差，这里只能慢慢调试
                {
                    cnt = TK_LONG_KEY_TIME;
                    send_status_keyval(KEY_PRESS_CONTINUE, KEY_TK6);
                }
            }

            // display_data ^= 0x0800; // 对应的LED状态取反，后面会调用led_display()刷新
            // led_display();          // LED状态更新显示
        }
    }
    else if (1 == long_touch_flag && 0 == KeyOnOutput)
    {
        // 如果之前是长按，现在却松开了按键
        switch (KeyOld)
        {
        case TK_CH24_VALIB:
            send_status_keyval(KEY_PRESS_LOOSE, KEY_TK1);
            break;

        case TK_CH3_VALIB:
            send_status_keyval(KEY_PRESS_LOOSE, KEY_TK2);
            break;

        case TK_CH2_VALIB:
            send_status_keyval(KEY_PRESS_LOOSE, KEY_TK3);
            break;

        case TK_CH0_VALIB:
            send_status_keyval(KEY_PRESS_LOOSE, KEY_TK4);
            break;

        case TK_CH9_VALIB:
            send_status_keyval(KEY_PRESS_LOOSE, KEY_TK5);
            break;

        case TK_CH10_VALIB:
            send_status_keyval(KEY_PRESS_LOOSE, KEY_TK6);
            break;
        }

        long_touch_flag = 0; // 清除标志位
        cnt = 0;
        KeyOld = 0;
    }

#if 0
    if (1 == __tk_long_key_flag) // 如果长按按键（这个功能测试发现不行，不能用）
    {
        __tk_long_key_flag = 0;

        // 这里可以自己处理，添加自己需要的功能

        P11 = 0;
    }
#endif

    // led_display(); // LED状态更新显示
}
#endif // end of #ifdef CIRCUIT_BOARD
/*************************** (C) COPYRIGHT 2022 TAIXIN-IC ***** END OF FILE *****/
