#include "timer0.h"
#include "my_config.h"
#include "rf_recv.h"

// 触摸按键模块初始化的倒计时
static volatile u32 touch_init_cnt = 0;

volatile bit flag_is_reinitialize_touch = 0; // 是否需要重新初始化触摸按键

// 清零触摸按键模块初始化的倒计时
void touch_cnt_down_clear(void)
{
    touch_init_cnt = 0;
    flag_is_reinitialize_touch = 0;
}

/**
 * @brief 配置定时器TMR0，定时器默认关闭
 */
void timer0_config(void)
{
    __EnableIRQ(TMR0_IRQn); // 使能timer0中断
    IE_EA = 1;              // 使能总中断

#define PEROID_VAL (SYSCLK / 1 / 10000 - 1) // 周期值=系统时钟/分频/频率 - 1

    TMR_ALLCON = TMR0_CNT_CLR(0x1);                        // 清除计数值
    TMR0_PRH = TMR_PERIOD_VAL_H((PEROID_VAL >> 8) & 0xFF); // 周期值
    TMR0_PRL = TMR_PERIOD_VAL_L((PEROID_VAL >> 0) & 0xFF);
    TMR0_CONH = TMR_PRD_PND(0x1) | TMR_PRD_IRQ_EN(0x1);                           // 计数等于周期时允许发生中断
    TMR0_CONL = TMR_SOURCE_SEL(0x7) | TMR_PRESCALE_SEL(0x00) | TMR_MODE_SEL(0x1); // 选择系统时钟，1分频，计数模式
}

// TMR0中断服务函数
void TIMR0_IRQHandler(void) interrupt TMR0_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR0_IRQn);

    // ---------------- 用户函数处理 -------------------

    // 周期中断
    if (TMR0_CONH & TMR_PRD_PND(0x1))
    {
        TMR0_CONH |= TMR_PRD_PND(0x1); // 清除pending

#if 1 // rf信号接收 （100us调用一次）
        {
            static volatile u8 rf_bit_cnt;            // RF信号接收的数据位计数值
            static volatile u32 __rf_data;            // 定时器中断使用的接收缓冲区，避免直接覆盖全局的数据接收缓冲区
            static volatile u8 flag_is_enable_recv;   // 是否使能接收的标志位，要接收到 5ms+ 的低电平才开始接收
            static volatile u8 __flag_is_recved_data; // 表示中断服务函数接收到了rf数据

            static volatile u8 low_level_cnt;  // RF信号低电平计数值
            static volatile u8 high_level_cnt; // RF信号高电平计数值

            // 在定时器 中扫描端口电平
            if (0 == RFIN_PIN)
            {
                // 如果RF接收引脚为低电平，记录低电平的持续时间
                low_level_cnt++;

                /*
                    下面的判断条件是避免部分遥控器或接收模块只发送24位数据，最后不拉高电平的情况
                */
                if (low_level_cnt >= 30 && rf_bit_cnt == 23) // 如果低电平大于3000us，并且已经接收了23位数据
                {
                    if (high_level_cnt >= 6 && high_level_cnt < 20)
                    {
                        __rf_data |= 0x01;
                    }
                    else if (high_level_cnt >= 1 && high_level_cnt < 6)
                    {
                    }

                    __flag_is_recved_data = 1; // 接收完成标志位置一
                    flag_is_enable_recv = 0;
                }
            }
            else
            {
                if (low_level_cnt > 0)
                {
                    // 如果之前接收到了低电平信号，现在遇到了高电平，判断是否接收完成了一位数据
                    if (low_level_cnt > 50)
                    {
                        // 如果低电平持续时间大于50 * 100us（5ms），准备下一次再读取有效信号
                        __rf_data = 0;  // 清除接收的数据帧
                        rf_bit_cnt = 0; // 清除用来记录接收的数据位数

                        flag_is_enable_recv = 1;
                    }
                    else if (flag_is_enable_recv &&
                             low_level_cnt >= 2 && low_level_cnt < 7 &&
                             high_level_cnt >= 6 && high_level_cnt < 20)
                    {
                        // 如果低电平持续时间在360us左右，高电平持续时间在760us左右，说明接收到了1
                        __rf_data |= 0x01;
                        rf_bit_cnt++;
                        if (rf_bit_cnt != 24)
                        {
                            __rf_data <<= 1; // 用于存放接收24位数据的变量左移一位
                        }
                    }
                    else if (flag_is_enable_recv &&
                             low_level_cnt >= 7 && low_level_cnt < 20 &&
                             high_level_cnt >= 1 && high_level_cnt < 6)
                    {
                        // 如果低电平持续时间在840us左右，高电平持续时间在360us左右，说明接收到了0
                        __rf_data &= ~1;
                        rf_bit_cnt++;
                        if (rf_bit_cnt != 24)
                        {
                            __rf_data <<= 1; // 用于存放接收24位数据的变量左移一位
                        }
                    }
                    else
                    {
                        // 如果低电平持续时间不符合0和1的判断条件，说明此时没有接收到信号
                        __rf_data = 0;
                        rf_bit_cnt = 0;
                        flag_is_enable_recv = 0;
                    }

                    low_level_cnt = 0; // 无论是否接收到一位数据，遇到高电平时，先清除之前的计数值
                    high_level_cnt = 0;

                    if (24 == rf_bit_cnt)
                    {
                        // 如果接收成了24位的数据
                        __flag_is_recved_data = 1; // 接收完成标志位置一
                        flag_is_enable_recv = 0;
                    }
                }
                else
                {
                    // 如果接收到高电平后，低电平的计数为0

                    if (0 == flag_is_enable_recv)
                    {
                        __rf_data = 0;
                        rf_bit_cnt = 0;
                        flag_is_enable_recv = 0;
                    }
                }

                // 如果RF接收引脚为高电平，记录高电平的持续时间
                high_level_cnt++;
            }

            if (__flag_is_recved_data) //
            {
                rf_bit_cnt = 0;
                __flag_is_recved_data = 0;
                low_level_cnt = 0;
                high_level_cnt = 0;

                // if (rf_data != 0)
                // if (0 == flag_is_recved_rf_data) /* 如果之前未接收到数据 或是 已经处理完上一次接收到的数据 */
                {
                    // 现在改为只要收到新的数据，就覆盖rf_data
                    rf_data = __rf_data;
                    recv_rf_flag = 1;
                }
                // else
                // {
                //     __rf_data = 0;
                // }
            }
        }
#endif // rf信号接收 （100us调用一次）

        {
            static u8 cnt = 0;
            cnt++;
            if (cnt >= 10) // 10 * 100us，1ms进入一次
            {
                cnt = 0;

                if (touch_init_cnt < 4294967296 - 1) // 防止计数溢出 （2^32）
                {
                    touch_init_cnt++;

                    if (touch_init_cnt >= 5000)
                    {
                        touch_init_cnt = 0;
                        flag_is_reinitialize_touch = 1;

                        // P15 = ~P15; // 测试用
                    }
                }
            }
        }
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR0_IRQn);
}
