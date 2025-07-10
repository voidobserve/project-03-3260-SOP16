#ifndef __TIMER0_H
#define __TIMER0_H

#include "include.h"


extern volatile bit flag_is_reinitialize_touch; // 是否需要重新初始化触摸按键 

void timer0_config(void);

// 清零触摸按键模块初始化的倒计时
void touch_cnt_down_clear(void);

#endif
