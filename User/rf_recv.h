// RF-315MHz��RF-433MHz����ͷ�ļ�
#ifndef __RF_RECORD_H
#define __RF_RECORD_H

#include "include.h" // ʹ��оƬ�ٷ��ṩ��ͷ�ļ�

#include "my_config.h"

// #ifndef RFIN
// #define RFIN P02 // RF��������
// #endif           // end def RFIN

#if 0

// �Զ���RF-315MHzң���������ļ�ֵ
#define KEY_RF315_A ((unsigned char)0x02) // 0010
#define KEY_RF315_B ((unsigned char)0x08) // 1000
#define KEY_RF315_C ((unsigned char)0x01) // 0001

#define KEY_RF315_D ((unsigned char)0x04) // 0100
#define KEY_RF315_E ((unsigned char)0x0C) // 1100
#define KEY_RF315_F ((unsigned char)0x06) // 0110

#endif // end �Զ���RF-315MHzң���������ļ�ֵ

extern volatile u32 rf_data;      // ���ڴ�Ž��յ���24λ����
extern volatile int recv_rf_flag; // �Ƿ���յ�rf�źŵı�־λ��0--δ�յ����ݣ�1--�յ�����

// ���ڴ�Ž��յ���24λ���ݣ�����n�Σ����ж���������û��������ַ�ظ��ء������ǳ��ִ�������
extern volatile u32 rf_data_buf[20];
extern unsigned char rf_data_buf_overflow; // ���ջ���������ı�־λ

void rfin_init(void); // RFIN���ų�ʼ����RF�������ų�ʼ����

void rf_recv(void);         // RF���պ����������ã�
void rf_recv_databuf(void); // ����RF�źŵ��������У�����n���źţ�n������֧�ֵ�Ԫ�ظ��������Ը�����Ҫ�޸ģ�

// �ж������г��ִ�������Ԫ��
// ������ִ���һ���ֻ࣬�᷵�ش���һ���ĵ�һ��Ԫ��
void appear_themost(u32 *arr, u32 arr_len, u32 *element, u32 *index);

// void tmr0_enable(void);  // ������ʱ��TMR0����ʼ��ʱ
// void tmr0_disable(void); // �رն�ʱ��0����ռ���ֵ

u8 rf_addr_isMatch(void);

#endif // end of file
