// 扫描RF信号的头文件
// 键值的定义
// 要发送的是16bit信号，包括格式头（5ms低电平）+8bit原码+8bit反码
// 按键原码中，包括3bit的长短按信息+5bit的键值信息
/*
    长短按中：
        00--短按
        01--长按
        10--持续
        11--长按后松开
        100--按键双击
*/
#ifndef __RF_SCAN_H
#define __RF_SCAN_H

#include "include.h" // 包含芯片官方提供的头文件

#include "my_config.h"

// 长按、短按等定义
enum
{
    KEY_PRESS_SHORT = 0x00,      // 按键短按
    KEY_PRESS_LONG = 0x01,       // 按键长按
    KEY_PRESS_CONTINUE = 0x02,   // 持续
    KEY_PRESS_LOOSE = 0x03,      // 按下后松开
    KEY_PRESS_DOUBLECLICK = 0x04 // 按键双击（触摸按键和遥控器按键的）
};

#if 0
// RF-433遥控器1（小） 按键键值
#define KEY1_POWER 0x03
#define KEY1_S_ADD 0x01

// RF-433遥控器2（大） 按键键值
#define KEY2_POWER 0x01
#define KEY2_ADD 0x03



// 测试用：
// RF-433遥控器1（小） 器件地址
#define RF433REMOTE_1_ADDR 0x28300 // 0b 0010 1000 0011 0000 0000
// RF-433遥控器2（大） 器件地址
#define RF433REMOTE_2_ADDR 0x28380 // 0b 0010 1000 0011 1000 0000
#endif

void rf_scan(void);

#endif // end of file
