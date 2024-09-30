// 扫描RF信号的源文件
#include "rf_scan.h"

#include "tmr3.h"     // 用来判断按键是短按、长按还是持续的定时器
#include "rf_learn.h" // 使用到了学习得到的、保存在flash中的地址
#include "rf_recv.h"  // RF信号接收
#include "send_key.h" // 发送带有键值的信号

#include "key_conversion.h" // 将键值转换成功能值（功能值唯一）

// 对RF信号接收引脚进行一次扫描，如果有信号，则进行相应的处理
void rf_scan(void)
{
    static u8 press_flag = 0;                // 按键状态标志位，0--未按下，1--短按，2--长按，3--持续
    static volatile u32 timer_total_cnt = 0; // 总计时值，用来判定短按、长按和持续

    // static u16 send_data = 0; // 存放待发送的数据（测试用/旧版程序使用）

    static u8 key = 0; // 存放接收到的键值

#if 0  // 测试能否接收和发送一个数据帧
        if (recv_rf_flag)
        {
            recv_rf_flag = 0;        // 清除标志位
            keyval = rf_data & 0x0F; // 记录键值

            send_data = rf_data & 0x0F;
            send_data |= KEY_PRESS_CONTINUE << 5;
            send_keyval(send_data);
            return;
        }
#endif // end 测试能否接收和发送一个数据帧

    if (recv_rf_flag && 0 == timer_total_cnt && 0 == press_flag)
    {
        // 如果未按下，记录本次按下的时间
        recv_rf_flag = 0; // 清除标志位

        if (1 == rf_addr_isMatch())
        {
            press_flag = 1; // 假设是短按

#ifdef DEVELOPMENT_BOARD // 开发板上最后只需要发送遥控器对应的键值
            // 记录一次键值
            key = rf_data & 0x3F;
#endif // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD // 目标电路板上需要将遥控器的键值转换成自定义的功能值
            // 记录一次键值（实际上是自定义的功能值）
            key = key_to_funval(rf_data >> 8, rf_data & 0x3F);
#endif // end ifdef CIRCUIT_BOARD

            tmr3_enable(); // 打开定时器，开始计数
        }
    }
    else if (timer_total_cnt < 75)
    {
        // 如果是短按的判定时间内（这里假设按下并松开了按键的时间是0~750ms之内）
        if (recv_rf_flag && tmr3_cnt <= 12)
        {
            // 如果收到了信号，并且两次信号的时间不超过120ms
            // （哪怕是短按，也会收到若干个信号，这些信号的间隔在13ms左右，
            // 一个信号持续时间是40ms左右，这里放宽了判断条件）
            recv_rf_flag = 0;

            if (1 == rf_addr_isMatch())
            {
#ifdef DEVELOPMENT_BOARD // 开发板上最后只需要发送遥控器对应的键值
                // 记录一次键值
                key = rf_data & 0x3F;
#endif // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD // 目标电路板上需要将遥控器的键值转换成自定义的功能值
                // 记录一次键值（实际上是自定义的功能值）
                key = key_to_funval(rf_data >> 8, rf_data & 0x3F);
#endif // end ifdef CIRCUIT_BOARD

                timer_total_cnt += tmr3_cnt; // 累计计数时间
                tmr3_cnt = 0;                // 清空计数值
            }
        }
        else if (tmr3_cnt > 12 && press_flag)
        {
            // 如果在120ms范围外，没有收到信号，说明此时已经松开手了，是短按
            static u8 old_key = 0; // 存放上一次接收的遥控器按键键值，用于辅助判断双击

            press_flag = 0;
            timer_total_cnt = 0;

            tmr3_disable();
            tmr3_cnt = 0;

            tmr4_cnt = 0;
            tmr4_enable();
            old_key = key; // 记录键值，以便下一次判断是否为双击（要两次键值相同）

            while (1)
            {
                if (recv_rf_flag && tmr4_cnt > 50 && tmr4_cnt < 110)
                {
                    // 如果两次按键的时间间隔在50ms~110ms之内，可能有双击
                    if (old_key == key)
                    {
                        // 如果两次按下的键值相等，说明是双击
                        send_status_keyval(KEY_PRESS_DOUBLECLICK, key);
                        delay_ms(220);    // 忽略第二次按下时连续发送过来的信号
                        recv_rf_flag = 0; // 清除标志位
                        key = 0;
                        old_key = 0;
                        return;
                    }
                    else
                    {
                        // 如果两次按下的键值不相等，说明不是双击
                        // 这里不清除标志位，可以留到下一次扫描
                        send_status_keyval(KEY_PRESS_SHORT, old_key);
                        key = 0;
                        old_key = 0;
                        return;
                    }
                }
                else if (recv_rf_flag && tmr4_cnt > 110)
                {
                    // 如果两次按下按键的时间间隔超出了110ms，说明不是双击
                    // 这里不清除标志位，可以留到下一次扫描
                    send_status_keyval(KEY_PRESS_SHORT, old_key); // 发送带有短按信息的键值
                    key = 0;
                    old_key = 0;
                    return;
                }
                else if (0 == recv_rf_flag && tmr4_cnt > 110)
                {
                    // 如果超过110ms都没有按下下一个按键，说明是短按
                    tmr4_disable(); // 关闭定时器
                    tmr4_cnt = 0;
                    send_status_keyval(KEY_PRESS_SHORT, old_key); // 发送带有短按信息的键值
                    key = 0;
                    old_key = 0;
                    return;
                }
            }
        }
    }
    else
    {
        // 长按和持续的处理
        if (1 == press_flag && timer_total_cnt >= 75 && timer_total_cnt < 90)
        {
            // 如果进入到这里，说明按下按键至少有750ms了
            // 发送一次带有长按标志的信号
            press_flag = 2;

#ifdef DEVELOPMENT_BOARD // 开发板上最后只需要发送遥控器对应的键值

            key = rf_data & 0x3F;

#endif // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD // 目标电路板上需要将遥控器的键值转换成自定义的功能值

            // 记录一次键值（实际上是自定义的功能值）
            key = key_to_funval(rf_data >> 8, rf_data & 0x3F);

#endif // end ifdef CIRCUIT_BOARD

            send_status_keyval(KEY_PRESS_LONG, key); // 发送带有长按信息的键值

            press_flag = 3;
        }

        if (3 == press_flag && recv_rf_flag && tmr3_cnt <= 12)
        {
            // 如果收到了信号，并且两次信号的时间不超过120ms
            // （哪怕是短按，也会收到若干个信号，这些信号的间隔在13ms左右，
            // 一个信号持续时间是40ms左右，这里放宽了判断条件）
            recv_rf_flag = 0;

            if (1 == rf_addr_isMatch())
            {
#ifdef DEVELOPMENT_BOARD              // 开发板上最后只需要发送遥控器对应的键值
                key = rf_data & 0x3F; // 记录键值
#endif                                // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD // 目标电路板上需要将遥控器的键值转换成自定义的功能值
                // 记录一次键值（实际上是自定义的功能值）
                key = key_to_funval(rf_data >> 8, rf_data & 0x3F);
#endif // end ifdef CIRCUIT_BOARD

                timer_total_cnt += tmr3_cnt; // 累计计数时间
                tmr3_cnt = 0;                // 清空计数值

                // 这里的时间间隔是以每个信号的周期为准，因为判断recv_rf_flag的时候，
                // 就是要有信号接收完成这个标志位才会置一，代码才能进入这里
                if (timer_total_cnt >= 90)
                {
                    // 清空累计计数时间，发送一次带有持续标志和按键键值的信号
                    // 接下来每到一定的时间就发送一次带有持续标志和按键键值的信号
                    send_status_keyval(KEY_PRESS_CONTINUE, key); // 发送带有持续按下消息的16位键值
                    timer_total_cnt = 75;
                }
            }
        }
        else if (tmr3_cnt > 12 && press_flag)
        {
            // 如果在120ms范围外，没有收到信号，说明此时已经松开手了
            // 这里可以发送一次长按后松开的信号
            send_status_keyval(KEY_PRESS_LOOSE, key); // 发送长按后松开按键的信号

            // 清除标志位和计数值
            press_flag = 0;
            timer_total_cnt = 0;

            tmr3_disable();
            tmr3_cnt = 0;

            key = 0;
            return;
        }
    }
}
