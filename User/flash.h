// 芯片内部的flash相关操作的头文件
#ifndef __FLASH_H
#define __FLASH_H

#include "include.h" // 包含芯片官方提供的头文件

void flash_erase_sector(u8 addr);
void flash_write(u8 addr, u8 *p_data, u8 len);
void flash_read(u8 addr, u8 *p_data, u8 len);

void flash_test(void); // 测试函数，使用前需要先初始化P12

#endif
