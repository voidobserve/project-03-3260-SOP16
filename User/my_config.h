// my_config.h -- 自定义的配置文件
#ifndef __MY_CONFIG_H
#define __MY_CONFIG_H

#include "include.h" // 使用芯片官方提供的头文件

// 开发平台
// DEVELOPMENT_BOARD -- 开发板
// CIRCUIT_BOARD -- 电路板
#define                             CIRCUIT_BOARD

// ============================================================
// 如果需要修改使用的触摸按键数量和位置，需要去tk_config.h中修改
// 并在tk_user.c中添加/修改对应的功能
// 触摸按键的数量和通道位置必须要一一对应
// ============================================================

// 开发板上使用P02作为RF信号的接收引脚
// 目标电路板上使用的是P13
// 如果需要修改引脚,还需要去rf_recv.c下修改引脚的初始化配置,
// 以及相应的中断服务函数和rf_recv_databuf()函数中对应的内容
#define RFIN                        P13  // RF接收引脚

// 开发板上使用P11作为发送键值信号的引脚
// 目标电路板上使用的是P21
// 如果需要修改，要去send_key.c中修改引脚相关的初始化配置函数
#define SEND_KEY_VAL_PIN            P31 // 发送键值信号的引脚

// ============================================================

// 单片机的flash中可以写入的首地址 （这款芯片是0x3F00，0x3F已经在flash相关的函数中写死了）
#define FLASH_DEVICE_START_ADDR     0x00

// 学习方式
/*
    学习方式
    1. 保存n个器件的地址（按不同的器件地址来保存）
    2. 保存n种类型器件的地址（按不同的器件类型来保存）
    |---假设在学习时按下的就是开机键(学习按键)，根据不同的键值来区分不同类型的器件

    相关的宏
    LEARN_BY_QUANTITY  // 保存n个器件的地址
    LEARN_BY_TYPE      // 保存n种类型器件的地址，每个类型只保存一个器件的地址
*/
#define LEARN_BY_QUANTITY // 在这里选择要使用的学习方式 

// 是否使用取消配对
// 1--使用取消配对，在上电时若长按已经学习过的按键，会从flash中删除对应的地址
// 0--不使用取消配对
#define USE_RF_UNPAIR               0 

#ifdef LEARN_BY_QUANTITY
// 保存n个器件的地址的情况下，支持保存的最大器件地址数目
#define  ADDR_MAX_NUM               2 
#endif // end ifdef LEARN_BY_QUANTITY

#ifdef LEARN_BY_TYPE
// 根据器件类型来保存的情况下，支持保存的最大器件类型数目
#define ADDR_MAX_TYPE_NUM           3
#endif // end ifdef LEARN_BY_TYPE

#include "tmr4.h"

#endif // end file
