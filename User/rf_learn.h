// ѧϰ���ܣ�����������ַ��أ���ͷ�ļ�
#ifndef __RF_LEARN_H
#define __RF_LEARN_H

#include "include.h" // ����оƬ�ٷ��ṩ��ͷ�ļ�

#include "my_config.h"

// ��Ƭ����flash�п���д����׵�ַ �����оƬ��0x3F00��0x3F�Ѿ���flash��صĺ�����д���ˣ�
#ifndef FLASH_DEVICE_START_ADDR
#define FLASH_DEVICE_START_ADDR 0x00
#endif // end ifndef FLASH_DEVICE_START_ADDR

/*
    ѧϰ��ʽ
    1. ����n�������ĵ�ַ
    2. ����n�����������ĵ�ַ
    |--������ѧϰʱ���µľ��ǿ�����(ѧϰ����)�����ݲ�ͬ�ļ�ֵ�����ֲ�ͬ���͵�����

    ��صĺ�
    LEARN_BY_QUANTITY  // ����n�������ĵ�ַ
    LEARN_BY_TYPE      // ����n�����������ĵ�ַ��ÿ������ֻ����һ�������ĵ�ַ
*/
#ifdef LEARN_BY_QUANTITY
#ifndef ADDR_MAX_NUM
#define ADDR_MAX_NUM 2 // ֧�ֱ�������������ַ��Ŀ
#endif                 // end ifndef ADDR_MAX_NUM

// �����ַ���ͣ������û���ѧϰʱ���µľ��ǵ�Դ����(ѧϰ����)��
// ���ݼ�ֵ���ж��û�ʹ�õ��Ǵ�ҡ��������Сң����
enum
{
    REMOTE_TYPE_SMALL_RM, // Сң����
    REMOTE_TYPE_BIG_RM    // ��ң����
};

typedef struct addr_info
{
    // ��ɵ�������ַ��Ȩֵ��ֵԽ�󣬱�־��Ӧ��������ַ������д���
    // ֵΪ�㣬˵����Ӧ��������ַ���£����£�д��ģ������ǿյģ���Ϊ���оƬ��flash������Ĭ��ֵ��0��
    u8 weighted_val_buf[ADDR_MAX_NUM];
    u32 addr_buf[ADDR_MAX_NUM]; // ������ַ����16bit��
    u8 remote_type[ADDR_MAX_NUM]; // ��ַ���ͣ����������Ǵ�ҡ��������Сң����
} addr_info_t;

// ���Ժ������鿴flash�а���ౣ��n����ַ�������������ַ
void show_addr_info_save_by_nums(void);

#endif // end LEARN_BY_QUANTITY
// ============================================================
#ifdef LEARN_BY_TYPE // ������������������
#ifndef ADDR_MAX_TYPE_NUM
#define ADDR_MAX_TYPE_NUM 2 // ֧�ֵ��������������Ŀ
#endif                      // end ifndef ADDR_MAX_TYPE_NUM

typedef struct addr_into
{

    // ��ɵ�������ַ��Ȩֵ��ֵԽ�󣬱�־��Ӧ��������ַ������д���
    // ֵΪ�㣬˵����Ӧ��������ַ���£����£�д��ģ������ǿյģ���Ϊ���оƬ��flash������Ĭ��ֵ��0��
    u8 weighted_val_buf[ADDR_MAX_TYPE_NUM];
    // ����������͵����飨���������ɼ�ֵ�����֣�ǰ���Ѿ�������ѧϰʱ���µľ��ǿ�����(ѧϰ����)�����ݲ�ͬ�ļ�ֵ�����ֲ�ͬ���ͣ�
    u32 type_buf[ADDR_MAX_TYPE_NUM];
    u32 addr_buf[ADDR_MAX_TYPE_NUM]; // ���������ַ������
} addr_info_t;

// ���Ժ��������鿴flash�а����������������������ַ
void show_addr_info_save_by_type(void);

#endif // end ifdef LEARN_BY_TYPE

extern addr_info_t addr_info; // ����������ַ��Ӧ��Ϣ�Ľṹ�������ѧϰ����£�
void rf_learn(void);          // ѧϰ����
u8 rf_addr_isMatch(void);     // �жϵ�ǰ���յ����ݵĵ�ַ�Ƿ���flash�б���ĵ�ַƥ�䣨��Ҫ��ѧϰ����flash�ж������ݣ�

#endif // end of file   end of __RF_LEARN_H
