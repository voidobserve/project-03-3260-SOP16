// 定时器TMR2的驱动头文件
#ifndef __TMR2_H
#define __TMR2_H

#include "include.h" // 包含芯片官方提供的头文件

// 定时器TMR2的计时周期，也是中断触发周期（每隔多久触发一次中断）
// 计时周期不能大于65535，因为TMR2存放计时周期的寄存器只有16位
// 现在让定时器TMR2每5ms触发一次中断
#define TMR2_CNT_TIME ((7619))           // 5000us / 0.65625us == 7462，这里调试得出是7500，最接近5ms

extern volatile unsigned char tmr2_flag; // tmr2中断服务函数中会置位的标志位
extern volatile u32 tmr2_cnt; // 定时器TMR2的计数值（每次在中断服务函数中会加一）

void tmr2_config(void); // 定时器tmr2的初始化
void tmr2_enable(void); // 开启定时器TMR2，开始计时
void tmr2_disable(void); // 关闭定时器TMR2，清空计数值

#endif
