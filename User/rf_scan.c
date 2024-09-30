// ɨ��RF�źŵ�Դ�ļ�
#include "rf_scan.h"

#include "tmr3.h"     // �����жϰ����Ƕ̰����������ǳ����Ķ�ʱ��
#include "rf_learn.h" // ʹ�õ���ѧϰ�õ��ġ�������flash�еĵ�ַ
#include "rf_recv.h"  // RF�źŽ���
#include "send_key.h" // ���ʹ��м�ֵ���ź�

#include "key_conversion.h" // ����ֵת���ɹ���ֵ������ֵΨһ��

// ��RF�źŽ������Ž���һ��ɨ�裬������źţ��������Ӧ�Ĵ���
void rf_scan(void)
{
    static u8 press_flag = 0;                // ����״̬��־λ��0--δ���£�1--�̰���2--������3--����
    static volatile u32 timer_total_cnt = 0; // �ܼ�ʱֵ�������ж��̰��������ͳ���

    // static u16 send_data = 0; // ��Ŵ����͵����ݣ�������/�ɰ����ʹ�ã�

    static u8 key = 0; // ��Ž��յ��ļ�ֵ

#if 0  // �����ܷ���պͷ���һ������֡
        if (recv_rf_flag)
        {
            recv_rf_flag = 0;        // �����־λ
            keyval = rf_data & 0x0F; // ��¼��ֵ

            send_data = rf_data & 0x0F;
            send_data |= KEY_PRESS_CONTINUE << 5;
            send_keyval(send_data);
            return;
        }
#endif // end �����ܷ���պͷ���һ������֡

    if (recv_rf_flag && 0 == timer_total_cnt && 0 == press_flag)
    {
        // ���δ���£���¼���ΰ��µ�ʱ��
        recv_rf_flag = 0; // �����־λ

        if (1 == rf_addr_isMatch())
        {
            press_flag = 1; // �����Ƕ̰�

#ifdef DEVELOPMENT_BOARD // �����������ֻ��Ҫ����ң������Ӧ�ļ�ֵ
            // ��¼һ�μ�ֵ
            key = rf_data & 0x3F;
#endif // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD // Ŀ���·������Ҫ��ң�����ļ�ֵת�����Զ���Ĺ���ֵ
            // ��¼һ�μ�ֵ��ʵ�������Զ���Ĺ���ֵ��
            key = key_to_funval(rf_data >> 8, rf_data & 0x3F);
#endif // end ifdef CIRCUIT_BOARD

            tmr3_enable(); // �򿪶�ʱ������ʼ����
        }
    }
    else if (timer_total_cnt < 75)
    {
        // ����Ƕ̰����ж�ʱ���ڣ�������谴�²��ɿ��˰�����ʱ����0~750ms֮�ڣ�
        if (recv_rf_flag && tmr3_cnt <= 12)
        {
            // ����յ����źţ����������źŵ�ʱ�䲻����120ms
            // �������Ƕ̰���Ҳ���յ����ɸ��źţ���Щ�źŵļ����13ms���ң�
            // һ���źų���ʱ����40ms���ң�����ſ����ж�������
            recv_rf_flag = 0;

            if (1 == rf_addr_isMatch())
            {
#ifdef DEVELOPMENT_BOARD // �����������ֻ��Ҫ����ң������Ӧ�ļ�ֵ
                // ��¼һ�μ�ֵ
                key = rf_data & 0x3F;
#endif // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD // Ŀ���·������Ҫ��ң�����ļ�ֵת�����Զ���Ĺ���ֵ
                // ��¼һ�μ�ֵ��ʵ�������Զ���Ĺ���ֵ��
                key = key_to_funval(rf_data >> 8, rf_data & 0x3F);
#endif // end ifdef CIRCUIT_BOARD

                timer_total_cnt += tmr3_cnt; // �ۼƼ���ʱ��
                tmr3_cnt = 0;                // ��ռ���ֵ
            }
        }
        else if (tmr3_cnt > 12 && press_flag)
        {
            // �����120ms��Χ�⣬û���յ��źţ�˵����ʱ�Ѿ��ɿ����ˣ��Ƕ̰�
            static u8 old_key = 0; // �����һ�ν��յ�ң����������ֵ�����ڸ����ж�˫��

            press_flag = 0;
            timer_total_cnt = 0;

            tmr3_disable();
            tmr3_cnt = 0;

            tmr4_cnt = 0;
            tmr4_enable();
            old_key = key; // ��¼��ֵ���Ա���һ���ж��Ƿ�Ϊ˫����Ҫ���μ�ֵ��ͬ��

            while (1)
            {
                if (recv_rf_flag && tmr4_cnt > 50 && tmr4_cnt < 110)
                {
                    // ������ΰ�����ʱ������50ms~110ms֮�ڣ�������˫��
                    if (old_key == key)
                    {
                        // ������ΰ��µļ�ֵ��ȣ�˵����˫��
                        send_status_keyval(KEY_PRESS_DOUBLECLICK, key);
                        delay_ms(220);    // ���Եڶ��ΰ���ʱ�������͹������ź�
                        recv_rf_flag = 0; // �����־λ
                        key = 0;
                        old_key = 0;
                        return;
                    }
                    else
                    {
                        // ������ΰ��µļ�ֵ����ȣ�˵������˫��
                        // ���ﲻ�����־λ������������һ��ɨ��
                        send_status_keyval(KEY_PRESS_SHORT, old_key);
                        key = 0;
                        old_key = 0;
                        return;
                    }
                }
                else if (recv_rf_flag && tmr4_cnt > 110)
                {
                    // ������ΰ��°�����ʱ����������110ms��˵������˫��
                    // ���ﲻ�����־λ������������һ��ɨ��
                    send_status_keyval(KEY_PRESS_SHORT, old_key); // ���ʹ��ж̰���Ϣ�ļ�ֵ
                    key = 0;
                    old_key = 0;
                    return;
                }
                else if (0 == recv_rf_flag && tmr4_cnt > 110)
                {
                    // �������110ms��û�а�����һ��������˵���Ƕ̰�
                    tmr4_disable(); // �رն�ʱ��
                    tmr4_cnt = 0;
                    send_status_keyval(KEY_PRESS_SHORT, old_key); // ���ʹ��ж̰���Ϣ�ļ�ֵ
                    key = 0;
                    old_key = 0;
                    return;
                }
            }
        }
    }
    else
    {
        // �����ͳ����Ĵ���
        if (1 == press_flag && timer_total_cnt >= 75 && timer_total_cnt < 90)
        {
            // ������뵽���˵�����°���������750ms��
            // ����һ�δ��г�����־���ź�
            press_flag = 2;

#ifdef DEVELOPMENT_BOARD // �����������ֻ��Ҫ����ң������Ӧ�ļ�ֵ

            key = rf_data & 0x3F;

#endif // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD // Ŀ���·������Ҫ��ң�����ļ�ֵת�����Զ���Ĺ���ֵ

            // ��¼һ�μ�ֵ��ʵ�������Զ���Ĺ���ֵ��
            key = key_to_funval(rf_data >> 8, rf_data & 0x3F);

#endif // end ifdef CIRCUIT_BOARD

            send_status_keyval(KEY_PRESS_LONG, key); // ���ʹ��г�����Ϣ�ļ�ֵ

            press_flag = 3;
        }

        if (3 == press_flag && recv_rf_flag && tmr3_cnt <= 12)
        {
            // ����յ����źţ����������źŵ�ʱ�䲻����120ms
            // �������Ƕ̰���Ҳ���յ����ɸ��źţ���Щ�źŵļ����13ms���ң�
            // һ���źų���ʱ����40ms���ң�����ſ����ж�������
            recv_rf_flag = 0;

            if (1 == rf_addr_isMatch())
            {
#ifdef DEVELOPMENT_BOARD              // �����������ֻ��Ҫ����ң������Ӧ�ļ�ֵ
                key = rf_data & 0x3F; // ��¼��ֵ
#endif                                // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD // Ŀ���·������Ҫ��ң�����ļ�ֵת�����Զ���Ĺ���ֵ
                // ��¼һ�μ�ֵ��ʵ�������Զ���Ĺ���ֵ��
                key = key_to_funval(rf_data >> 8, rf_data & 0x3F);
#endif // end ifdef CIRCUIT_BOARD

                timer_total_cnt += tmr3_cnt; // �ۼƼ���ʱ��
                tmr3_cnt = 0;                // ��ռ���ֵ

                // �����ʱ��������ÿ���źŵ�����Ϊ׼����Ϊ�ж�recv_rf_flag��ʱ��
                // ����Ҫ���źŽ�����������־λ�Ż���һ��������ܽ�������
                if (timer_total_cnt >= 90)
                {
                    // ����ۼƼ���ʱ�䣬����һ�δ��г�����־�Ͱ�����ֵ���ź�
                    // ������ÿ��һ����ʱ��ͷ���һ�δ��г�����־�Ͱ�����ֵ���ź�
                    send_status_keyval(KEY_PRESS_CONTINUE, key); // ���ʹ��г���������Ϣ��16λ��ֵ
                    timer_total_cnt = 75;
                }
            }
        }
        else if (tmr3_cnt > 12 && press_flag)
        {
            // �����120ms��Χ�⣬û���յ��źţ�˵����ʱ�Ѿ��ɿ�����
            // ������Է���һ�γ������ɿ����ź�
            send_status_keyval(KEY_PRESS_LOOSE, key); // ���ͳ������ɿ��������ź�

            // �����־λ�ͼ���ֵ
            press_flag = 0;
            timer_total_cnt = 0;

            tmr3_disable();
            tmr3_cnt = 0;

            key = 0;
            return;
        }
    }
}
