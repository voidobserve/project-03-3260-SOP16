// GPIO配置的头文件
#ifndef __MY_GPIO_H
#define __MY_GPIO_H

#include "include.h" // 包含芯片官方提供的头文件

// 开发板上与LED正极连接的引脚
#define LED6        P01
#define LED7        P26

void p12_output_config(void); // P12配置为输出模式


void p01_output_config(void); // P01连接到了开发板的LED6的正极
void p26_output_config(void); // P26连接到了开发板的LED7的正极P26配置为输出模式

#endif
