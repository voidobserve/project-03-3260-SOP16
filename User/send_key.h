// 用于发送键值的头文件
#ifndef __SEND_KEY_H
#define __SEND_KEY_H

#include "include.h" // 使用芯片官方提供的头文件

#include "my_config.h"

#ifndef SEND_KEY_VAL_PIN
#define SEND_KEY_VAL_PIN        P11 // 发送键值信号的引脚
#endif // end of #ifndef SEND_KEY_VAL_PIN

// 定时器TMR1的定时周期，设置为每200us产生一次中断
// （实际上有误差，有可能是200us，有可能是240us，有可能是160us，200us的次数会多很多，
// 因为这个数字是调试出来的，一开始跟实际的计算有出入，所以后面就用调试的方法）
// 在定时器中断服务函数中，一位二进制数分成四个周期来发送，一个周期就是200us左右
// 如果修改修改周期，可以调整下面这个宏的值
// #define TMR1_CNT_TIME           (1800 / 3)
#define TMR1_CNT_TIME           305 // 305 * 0.65625 us 约等于 200 us

extern volatile char send_keyval_flag; // 是否要发送键值的标志位，0--不发送，1--发送

void send_keyval_pin_init(void);   // 发送键值的引脚进行初始化
void send_keyval_timer_init(void); // 发送键值的引脚要搭配使用到的定时器进行初始化，定时器初始化后为关闭状态

void tmr1_enable(void);  // 开启定时器TMR1
void tmr1_disable(void); // 关闭定时器TMR1，清空计数值

void send_keyval(unsigned short send_data); // 发送一次按键键值（总共16bit）
void send_status_keyval(u8 status, u8 key); // 发送带有按键状态信息的16位数据

#endif // end of file
