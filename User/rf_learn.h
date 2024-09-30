// 学习功能（保存器件地址相关）的头文件
#ifndef __RF_LEARN_H
#define __RF_LEARN_H

#include "include.h" // 包含芯片官方提供的头文件

#include "my_config.h"

// 单片机的flash中可以写入的首地址 （这款芯片是0x3F00，0x3F已经在flash相关的函数中写死了）
#ifndef FLASH_DEVICE_START_ADDR
#define FLASH_DEVICE_START_ADDR 0x00
#endif // end ifndef FLASH_DEVICE_START_ADDR

/*
    学习方式
    1. 保存n个器件的地址
    2. 保存n种类型器件的地址
    |--假设在学习时按下的就是开机键(学习按键)，根据不同的键值来区分不同类型的器件

    相关的宏
    LEARN_BY_QUANTITY  // 保存n个器件的地址
    LEARN_BY_TYPE      // 保存n种类型器件的地址，每个类型只保存一个器件的地址
*/
#ifdef LEARN_BY_QUANTITY
#ifndef ADDR_MAX_NUM
#define ADDR_MAX_NUM 2 // 支持保存的最大器件地址数目
#endif                 // end ifndef ADDR_MAX_NUM

// 定义地址类型，假设用户在学习时按下的就是电源按键(学习按键)，
// 根据键值来判断用户使用的是大摇控器还是小遥控器
enum
{
    REMOTE_TYPE_SMALL_RM, // 小遥控器
    REMOTE_TYPE_BIG_RM    // 大遥控器
};

typedef struct addr_info
{
    // 最旧的器件地址的权值，值越大，标志对应的器件地址是最早写入的
    // 值为零，说明对应的器件地址是新（较新）写入的，或者是空的（因为这个芯片的flash擦除后默认值是0）
    u8 weighted_val_buf[ADDR_MAX_NUM];
    u32 addr_buf[ADDR_MAX_NUM]; // 器件地址（低16bit）
    u8 remote_type[ADDR_MAX_NUM]; // 地址类型，用来区分是大摇控器还是小遥控器
} addr_info_t;

// 测试函数，查看flash中按最多保存n个地址来保存的器件地址
void show_addr_info_save_by_nums(void);

#endif // end LEARN_BY_QUANTITY
// ============================================================
#ifdef LEARN_BY_TYPE // 根据器件类型来保存
#ifndef ADDR_MAX_TYPE_NUM
#define ADDR_MAX_TYPE_NUM 2 // 支持的最大器件类型数目
#endif                      // end ifndef ADDR_MAX_TYPE_NUM

typedef struct addr_into
{

    // 最旧的器件地址的权值，值越大，标志对应的器件地址是最早写入的
    // 值为零，说明对应的器件地址是新（较新）写入的，或者是空的（因为这个芯片的flash擦除后默认值是0）
    u8 weighted_val_buf[ADDR_MAX_TYPE_NUM];
    // 存放器件类型的数组（器件类型由键值来区分，前面已经假设在学习时按下的就是开机键(学习按键)，根据不同的键值来区分不同类型）
    u32 type_buf[ADDR_MAX_TYPE_NUM];
    u32 addr_buf[ADDR_MAX_TYPE_NUM]; // 存放器件地址的数组
} addr_info_t;

// 测试函数，，查看flash中按器件类型来保存的器件地址
void show_addr_info_save_by_type(void);

#endif // end ifdef LEARN_BY_TYPE

extern addr_info_t addr_info; // 保存器件地址对应信息的结构体变量（学习后更新）
void rf_learn(void);          // 学习功能
u8 rf_addr_isMatch(void);     // 判断当前接收的数据的地址是否与flash中保存的地址匹配（需要先学习，从flash中读出数据）

#endif // end of file   end of __RF_LEARN_H
