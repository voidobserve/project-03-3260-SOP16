// ɨ��RF�źŵ�ͷ�ļ�
// ��ֵ�Ķ���
// Ҫ���͵���16bit�źţ�������ʽͷ��5ms�͵�ƽ��+8bitԭ��+8bit����
// ����ԭ���У�����3bit�ĳ��̰���Ϣ+5bit�ļ�ֵ��Ϣ
/*
    ���̰��У�
        00--�̰�
        01--����
        10--����
        11--�������ɿ�
        100--����˫��
*/
#ifndef __RF_SCAN_H
#define __RF_SCAN_H

#include "include.h" // ����оƬ�ٷ��ṩ��ͷ�ļ�

#include "my_config.h"

// �������̰��ȶ���
enum
{
    KEY_PRESS_SHORT = 0x00,      // �����̰�
    KEY_PRESS_LONG = 0x01,       // ��������
    KEY_PRESS_CONTINUE = 0x02,   // ����
    KEY_PRESS_LOOSE = 0x03,      // ���º��ɿ�
    KEY_PRESS_DOUBLECLICK = 0x04 // ����˫��������������ң���������ģ�
};

#if 0
// RF-433ң����1��С�� ������ֵ
#define KEY1_POWER 0x03
#define KEY1_S_ADD 0x01

// RF-433ң����2���� ������ֵ
#define KEY2_POWER 0x01
#define KEY2_ADD 0x03



// �����ã�
// RF-433ң����1��С�� ������ַ
#define RF433REMOTE_1_ADDR 0x28300 // 0b 0010 1000 0011 0000 0000
// RF-433ң����2���� ������ַ
#define RF433REMOTE_2_ADDR 0x28380 // 0b 0010 1000 0011 1000 0000
#endif

void rf_scan(void);

#endif // end of file
