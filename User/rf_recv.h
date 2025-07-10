// RF-315MHz和RF-433MHz解码头文件
#ifndef __RF_RECORD_H
#define __RF_RECORD_H

#include "include.h" // 使用芯片官方提供的头文件

#include "my_config.h"

// #ifndef RFIN
// #define RFIN P02 // RF接收引脚
// #endif           // end def RFIN

#if 0

// 自定义RF-315MHz遥控器按键的键值
#define KEY_RF315_A ((unsigned char)0x02) // 0010
#define KEY_RF315_B ((unsigned char)0x08) // 1000
#define KEY_RF315_C ((unsigned char)0x01) // 0001

#define KEY_RF315_D ((unsigned char)0x04) // 0100
#define KEY_RF315_E ((unsigned char)0x0C) // 1100
#define KEY_RF315_F ((unsigned char)0x06) // 0110

#endif // end 自定义RF-315MHz遥控器按键的键值

extern volatile u32 rf_data;      // 用于存放接收到的24位数据
extern volatile int recv_rf_flag; // 是否接收到rf信号的标志位，0--未收到数据，1--收到数据

// 用于存放接收到的24位数据，接收n次，再判断这里面有没有器件地址重复地、并且是出现次数最多的
extern volatile u32 rf_data_buf[20];
extern unsigned char rf_data_buf_overflow; // 接收缓冲区溢出的标志位

void rfin_init(void); // RFIN引脚初始化（RF接收引脚初始化）

void rf_recv(void);         // RF接收函数（测试用）
void rf_recv_databuf(void); // 接收RF信号到缓冲区中，接收n次信号（n是数组支持的元素个数，可以根据需要修改）

// 判断数组中出现次数最多的元素
// 如果出现次数一样多，只会返回次数一样的第一个元素
void appear_themost(u32 *arr, u32 arr_len, u32 *element, u32 *index);

// void tmr0_enable(void);  // 开启定时器TMR0，开始计时
// void tmr0_disable(void); // 关闭定时器0，清空计数值

u8 rf_addr_isMatch(void);

#endif // end of file
