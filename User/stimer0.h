#ifndef __STIMER0_H
#define __STIMER0_H

#include "include.h"

#define STMR0_PEROID_VAL (SYSCLK / 1 / 1000 - 1) // 周期值=系统时钟/分频/频率 - 1

extern volatile bit flag_is_reinitialize_touch; // 是否需要重新初始化触摸按键

void stimer0_config(void);

// 清零触摸按键模块初始化的倒计时
void touch_cnt_down_clear(void);

#endif
