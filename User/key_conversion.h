// 将遥控器按键的键值转换成唯一的功能值的头文件
#ifndef __KEY_CONVERSION_H
#define __KEY_CONVERSION_H

#include "include.h" // 包含芯片官方提供的头文件

// 较小的RF-433遥控器的器件地址
#define ADDR_SMALL_RM 0x2830

// 较大的RF-433遥控器的器件地址
#define ADDR_BIG_RM 0x2838

// 自定义的键值
// 实际要使用的触摸按键的键值
enum
{
    KEY_TK1 = 0x16, // KEY1的5位键值 0b 0001 0110
    KEY_TK2 = 0x17, // KEY2的5位键值 0b 0001 0111
    KEY_TK3 = 0x18, // KEY3的5位键值 0b 0001 1000
    KEY_TK4 = 0x19, // KEY4的5位键值 0b 0001 1001
    KEY_TK5 = 0x1A, // KEY5的5位键值 0x 0001 1010
    KEY_TK6 = 0x1B  // KEY6的5位键值 0x 0001 1011
};

// 较小的遥控器对应的实际键值
// 按键从左到右、从上到下
enum
{
    KEY_SMALL_RM_1 = 0x01,
    KEY_SMALL_RM_2 = 0x02,
    KEY_SMALL_RM_3 = 0x03,
    KEY_SMALL_RM_4 = 0x04,
    KEY_SMALL_RM_5 = 0x05,
    KEY_SMALL_RM_6 = 0x06,
    KEY_SMALL_RM_7 = 0x07,
    KEY_SMALL_RM_8 = 0x08,
    KEY_SMALL_RM_9 = 0x09,
    KEY_SMALL_RM_10 = 0x0A
};

// 较大的遥控器对应的实际键值
// 按键从左到右、从上到下
enum
{
    KEY_BIG_RM_1 = 0x01,
    KEY_BIG_RM_2 = 0x02,
    KEY_BIG_RM_3 = 0x03,
    KEY_BIG_RM_4 = 0x04,
    KEY_BIG_RM_5 = 0x05,
    KEY_BIG_RM_6 = 0x06,
    KEY_BIG_RM_7 = 0x07,
    KEY_BIG_RM_8 = 0x08,
    KEY_BIG_RM_9 = 0x09,
    KEY_BIG_RM_10 = 0x0A,

    KEY_BIG_RM_11 = 0x0B,
    KEY_BIG_RM_12 = 0x0C,
    KEY_BIG_RM_13 = 0x0D,
    KEY_BIG_RM_14 = 0x0E,
    KEY_BIG_RM_15 = 0x0F,
    KEY_BIG_RM_16 = 0x10
};

/*
    转换之后，每个按键对应一个功能，给这每个功能确定一个唯一的键值，
    相同功能的按键才有相同的键值
*/
enum
{
    KEY_FUNCTION_POWER = 0x01, // 电源开关
    KEY_FUNCTION_UP = 0x02,    // 功能--上
    KEY_FUNCTION_LEFT = 0x03,  // 功能--左
    KEY_FUNCTION_RIGHT = 0x04, // 功能--右
    KEY_FUNCTION_DOWN = 0x05,  // 功能--下

    KEY_FUNCTION_LIGHT = 0x06,      // 功能--亮度
    KEY_FUNCTION_MUSIC_ICON = 0x07, // 功能--音乐（按键上是个音乐图标）
    KEY_FUNCTION_AUTO = 0x08,       // 功能--自动
    KEY_FUNCTION_LIST = 0x09,       // 功能--列表
    KEY_FUNCTION_RAINBOW = 0x0A,    // 功能--彩虹（多种多样的）

    KEY_FUNCTION_MUTE = 0x0B,       // 功能--静音
    KEY_FUNCTION_PLAY_PAUSE = 0x0C, // 功能--播放暂停
    KEY_FUNCTION_MUSIC_FONT = 0x0D, // 功能--音乐（按钮上就是"MUSIC"字样）
    KEY_FUNCTION_MODE = 0x0E,       // 功能--模式
    KEY_FUNCTION_MOVIE = 0x0F,      // 功能--影视

    KEY_FUNCTION_TREB_ADD = 0x10, // 功能--高音+
    KEY_FUNCTION_TREB_SUB = 0x13, // 功能--高音-

    KEY_FUNCTION_NEWS = 0x11,    // 功能--新闻
    KEY_FUNCTION_DISPLAY = 0x14, // 功能--显示

    KEY_FUNCTION_BASS_ADD = 0x12, // 功能--低音+
    KEY_FUNCTION_BASS_SUB = 0x15  // 功能--低音-
};


// 根据器件地址和键值，转换成键值对应的功能值
u8 key_to_funval(u16 addr, u8 key);

#endif
