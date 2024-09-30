#ifndef __TMR4_H
#define __TMR4_H

#include "include.h" // 使用芯片官方提供的头文件

extern volatile u32 tmr4_cnt; // 定时器TMR4的计数值（每次在中断服务函数中会加一）

void tmr4_config(void);  // 定时器TMR4的初始化（初始化完成后，默认是关闭的）
void tmr4_enable(void);  // 开启定时器TMR4，开始计时
void tmr4_disable(void); // 关闭定时器，清空计数值

#endif
