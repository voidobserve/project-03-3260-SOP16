// 定时器TMR2的驱动头文件
#ifndef __TMR3_H
#define __TMR3_H

#include "include.h" // 使用芯片官方提供的头文件

extern volatile u32 tmr3_cnt; // 定时器TMR3的计数值（每次在中断服务函数中会加一）

// 定时器TMR3的初始化（初始化完成后，默认是关闭的）
void tmr3_config(void);

// 开启定时器TMR3，开始计时
void tmr3_enable(void);

// 关闭定时器，清空计数值
void tmr3_disable(void);

#endif
