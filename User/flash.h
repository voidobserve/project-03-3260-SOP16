// оƬ�ڲ���flash��ز�����ͷ�ļ�
#ifndef __FLASH_H
#define __FLASH_H

#include "include.h" // ����оƬ�ٷ��ṩ��ͷ�ļ�

void flash_erase_sector(u8 addr);
void flash_write(u8 addr, u8 *p_data, u8 len);
void flash_read(u8 addr, u8 *p_data, u8 len);

void flash_test(void); // ���Ժ�����ʹ��ǰ��Ҫ�ȳ�ʼ��P12

#endif
