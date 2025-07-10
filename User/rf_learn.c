// ѧϰ���ܣ�����������ַ��أ���Դ�ļ�
#include "rf_learn.h" //
#include "flash.h"    // ������Ƭ����flash��ز���
#include "rf_recv.h"  // rf������ز���
#include "send_key.h" // ���ͼ�ֵ������ʱ��
#include "tmr2.h"     // ʹ�ö�ʱ��TMR2��ʵ��5s��ʱ

#include "timer0.h"
// #include "stimer0.h"

#include <stdlib.h> // ʹ����NULL

addr_info_t addr_info = {0}; // ��Ŵ�flash�ж�����������ַ

#ifdef LEARN_BY_QUANTITY // ����n��������ַ
/**
 * @brief RFѧϰ���ܣ�5s�ڼ�⵽�а��������������ң������������ַд�뵽flash�У�һ������ϵ��5s��ʹ�ã�
 *          ��� my_config.h�ļ��е�USE_RF_UNPAIRΪ1����ô���յ�����flash����ͬ��������ַ�������ź�ʱ��
 *          ���flash��ɾ���õ�ַ���ݣ�ʵ��ȡ�����
 */
void rf_learn(void)
{
    u32 appear_themost_ele = 0; // RF���������г�����������Ԫ��

    u32 i = 0; // ѭ������ֵ
    u32 j = 0; // ѭ������ֵ

    bit isSpare = 0; // ��־λ��flash���Ƿ��п���λ�����洢�յĵ�ַ

    u32 temp = 0;             // ��ʱ�����������ʱֵ
    u32 the_oldest_index = 0; // ��ɵ�������ַ��Ӧ�������±�

    // �򿪶�ʱ��
    tmr2_enable();

    while (tmr2_flag != 1)
    {
        WDT_KEY = WDT_KEY_VAL(0xAA); // ι��
        rf_recv_databuf();           // ÿ��ѭ�������Ŷ�ȡһ������

        // ���ʱ���ڣ�һֱ��� ��������ģ���ʼ���ĵ���ʱ
        touch_cnt_down_clear();
    }

    // 5s�󣬹رն�ʱ���������Ӳ���ļ���ֵ
    tmr2_disable();
    tmr2_flag = 0; // �����־λ
    tmr2_cnt = 0;  // �����ʱ���ļ���ֵ

    // ��flash�ж�ȡ������ַ
    flash_read(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));

    // ������ʱ���ڼ�⵽�а���������ʹ�ñ�־λ��һ��������صĴ���
    if (rf_data_buf_overflow)
    {
        rf_data_buf_overflow = 0;
        // �ҳ������г��ִ�������Ԫ��
        appear_themost(rf_data_buf, sizeof(rf_data_buf) / sizeof(rf_data_buf[0]), &appear_themost_ele, NULL);

        // ������ǵ�Դ����(ѧϰ����)�������˳�
        if (((appear_themost_ele & 0xFF) != 0x01) && ((appear_themost_ele & 0xFF) != 0x03))
        {
            return;
        }

        // ����ǵ�Դ����
        // ��������ַд�뵽��Ƭ����flash��
        // |--���û����ֱ��д�룬������򸲸���ɵ�������ַ����һ����д��

        // ͨ��ѭ���ж�flash���Ƿ��Ѿ��ж�Ӧ�ĵ�ַ
        for (i = 0; i < ADDR_MAX_NUM; i++)
        {
            if (addr_info.addr_buf[i] == (appear_themost_ele >> 8))
            {
#if USE_RF_UNPAIR // ������� ȡ�����

                // ��������ַ��ͬ�������Ӧ������
                addr_info.addr_buf[i] = 0;
                addr_info.weighted_val_buf[i] = 0; // ���Ȩֵ
                addr_info.remote_type[i] = 0;      // ���ң��������

                // ��������ַд��flash
                flash_erase_sector(FLASH_DEVICE_START_ADDR);
                flash_write(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));

                return; // д����ɣ�����ֱ�ӷ���

#else // ���û�п��� ȡ�����

                addr_info.weighted_val_buf[i] = 0;
                addr_info.addr_buf[i] = (appear_themost_ele >> 8); // �����ַ��һ�����Բ��üӣ���Ϊ��ʱ��ַ����ͬ��

                if (0x01 == (appear_themost_ele & 0xFF))
                {
                    // ����Ǵ�ҡ��������ĵ�Դ����/ѧϰ����
                    addr_info.remote_type[i] = REMOTE_TYPE_BIG_RM;
                }
                else if (0x03 == (appear_themost_ele & 0xFF))
                {
                    // �����Сң��������ĵ�Դ����/ѧϰ����
                    addr_info.remote_type[i] = REMOTE_TYPE_SMALL_RM;
                }

                // ����������ַ��Ȩֵ�������ӿյĵ�ַ��Ȩֵ��
                for (j = 0; j < ADDR_MAX_NUM; j++)
                {
                    if ((j != i) && (addr_info.addr_buf[j] != 0))
                    {
                        addr_info.weighted_val_buf[j]++;
                    }
                }

                // ��󣬽�������ַд��flash
                flash_erase_sector(FLASH_DEVICE_START_ADDR);
                flash_write(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));
                return; // ����ѧϰ

#endif // end if USE_RF_UNPAIR
            }
        }

        // ���Ҫ����ĵ�ַ��flash�в�û���ظ�
        // |---��һ���յĵط����棬���û�У��򸲸���ɵ�������ַ
        for (i = 0; i < ADDR_MAX_NUM; i++)
        {
            if ((addr_info.weighted_val_buf[i] == 0) && (addr_info.addr_buf[i] == 0))
            {
                // ����пյĵط���ֱ�ӱ���
                addr_info.weighted_val_buf[i] = 0;
                addr_info.addr_buf[i] = (appear_themost_ele >> 8);

                if (0x01 == (appear_themost_ele & 0xFF))
                {
                    // ����Ǵ�ҡ��������ĵ�Դ����/ѧϰ����
                    addr_info.remote_type[i] = REMOTE_TYPE_BIG_RM;
                }
                else if (0x03 == (appear_themost_ele & 0xFF))
                {
                    // �����Сң��������ĵ�Դ����/ѧϰ����
                    addr_info.remote_type[i] = REMOTE_TYPE_SMALL_RM;
                }

                // ����������ַ��Ȩֵ�������ӿյĵ�ַ��Ȩֵ��
                for (j = 0; j < ADDR_MAX_NUM; j++)
                {
                    if ((j != i) && (addr_info.addr_buf[j] != 0))
                    {
                        addr_info.weighted_val_buf[j]++;
                    }
                }

                isSpare = 1;
                break;
            }
        }

        // ���û�п���ĵط����򸲸���ɵ�������ַ
        if (0 == isSpare)
        {
            // ͨ��ѭ���ҵ���ɵĵ�ַ���ڵ������±�
            for (i = 0; i < ADDR_MAX_NUM; i++)
            {
                if (addr_info.weighted_val_buf[i] > temp)
                {
                    temp = addr_info.weighted_val_buf[i];
                    the_oldest_index = i;
                }
            }

            addr_info.addr_buf[the_oldest_index] = (appear_themost_ele >> 8); // ����������ַ
            addr_info.weighted_val_buf[the_oldest_index] = 0;                 // Ȩֵ����Ϊ0

            if (0x01 == (appear_themost_ele & 0xFF))
            {
                // ����Ǵ�ҡ��������ĵ�Դ����/ѧϰ����
                addr_info.remote_type[the_oldest_index] = REMOTE_TYPE_BIG_RM;
            }
            else if (0x03 == (appear_themost_ele & 0xFF))
            {
                // �����Сң��������ĵ�Դ����/ѧϰ����
                addr_info.remote_type[the_oldest_index] = REMOTE_TYPE_SMALL_RM;
            }

            // ����������������ַ��Ȩֵ������Ҫ���ǲ����ӿյ�������ַ��Ȩֵ��
            for (j = 0; j < ADDR_MAX_NUM; j++)
            {
                if ((j != the_oldest_index) && (addr_info.addr_buf[j] != 0))
                {
                    addr_info.weighted_val_buf[j]++;
                }
            }
        }

        // ��󣬽�������ַд��flash
        flash_erase_sector(FLASH_DEVICE_START_ADDR);
        flash_write(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));
    }
}

// �жϵ�ǰ���յ����ݵĵ�ַ�Ƿ���flash�б���ĵ�ַƥ��
// ��Ҫ��ѧϰ����flash�ж�������
// ���أ� 0--��ƥ�䣬1--ƥ��
u8 rf_addr_isMatch(void)
{
    u8 i = 0;
    for (i = 0; i < ADDR_MAX_NUM; i++)
    {
        if (addr_info.addr_buf[i] == (rf_data >> 8))
        {
            return 1;
        }
    }

    return 0;
}

// ���Ժ������鿴flash�а���ͬ��ַ�����������������ַ
void show_addr_info_save_by_nums(void)
{
    u32 i;

    for (i = 0; i < ADDR_MAX_NUM; i++)
    {
        send_keyval(addr_info.weighted_val_buf[i]);
        send_keyval(addr_info.addr_buf[i]);
    }
}

#endif // end of #ifdef LEARN_BY_QUANTITY

#ifdef LEARN_BY_TYPE
/**
 * @brief RFѧϰ���ܣ�5s�ڼ�⵽�а��������������ң������������ַд�뵽flash�У�һ������ϵ��5s��ʹ�ã�
 *          ��� my_config.h�ļ��е�USE_RF_UNPAIRΪ1����ô���յ�����flash����ͬ��������ַ�������ź�ʱ��
 *          ���flash��ɾ���õ�ַ���ݣ�ʵ��ȡ�����
 */
void rf_learn(void)
{
    u32 appear_themost_ele = 0; // RF���������г�����������Ԫ��

    int32 i = 0; // ѭ������ֵ
    int32 j = 0; // ѭ������ֵ

    u8 isExist = 0; // ��־λ����ַ�Ƿ��Ѿ���flash����
    u8 isSpare = 0; // ��־λ����־��ŵ�ַ���������Ƿ��пյĵط�

    u32 temp = 0;             // �����ʱֵ
    u32 the_oldest_index = 0; // ��ɵ�������ַ��Ӧ�������±�

    // �򿪶�ʱ��
    tmr2_enable();

    while (tmr2_flag != 1)
    {
        WDT_KEY = WDT_KEY_VAL(0xAA); // ι��
        rf_recv_databuf();           // ÿ��ѭ�������Ŷ�ȡһ������
    }

    // 5s�󣬹رն�ʱ���������Ӳ���ļ���ֵ
    tmr2_disable();
    tmr2_flag = 0; // �����־λ
    tmr2_cnt = 0;  // �����ʱ���ļ���ֵ

    // ��flash�ж�ȡ������ַ
    flash_read(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));

    // ������ʱ���ڣ�5s�ڣ���⵽�а���������ʹ�ñ�־λ��һ��������صĴ���
    if (rf_data_buf_overflow)
    {
        rf_data_buf_overflow = 0;
        // �ҳ������г��ִ�������Ԫ��
        appear_themost(rf_data_buf, sizeof(rf_data_buf) / sizeof(rf_data_buf[0]), &appear_themost_ele, NULL);

        // ������ǵ�Դ�����������˳��������䣩

        // ����ǵ�Դ����
        // ��������ַд�뵽��Ƭ����flash��
        // |--���û����ֱ��д�룬������򸲸���ɵ�������ַ����һ����д��
        for (i = 0; i < ADDR_MAX_TYPE_NUM; i++)
        {
            if (addr_info.type_buf[i] == (appear_themost_ele & 0xFF))
            {
                // ���flash���Ѿ��ж�Ӧ���͵�������ַ
                isExist = 1;
                break;
            }
        }

        if (isExist)
        {
            // ���flash���Ѿ��ж�Ӧ���͵�������ַ
            // �ж�������ַ�Ƿ���ͬ
            if (addr_info.addr_buf[i] == (appear_themost_ele >> 8))
            {
#if USE_RF_UNPAIR // ���ʹ����ȡ�����

                // ��������ַ��ͬ�������Ӧ������
                addr_info.addr_buf[i] = 0;
                addr_info.type_buf[i] = 0;
                addr_info.weighted_val_buf[i] = 0;

                // ��������ַд��flash
                flash_erase_sector(FLASH_DEVICE_START_ADDR);
                flash_write(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));

                return; // д����ɺ󣬺�������ֱ�ӷ�����

#else // �����ʹ��ȡ�����

                // ��������ַ��ͬ���ø��ǣ�ֱ���˳�
                return;

#endif // end if USE_RF_UNPAIR
            }

            addr_info.addr_buf[i] = (appear_themost_ele >> 8); // ����������ַ
            addr_info.weighted_val_buf[i] = 0;                 // Ȩֵ����Ϊ0����ʾ��������д���

            // �����������͵�������ַ��Ȩֵ������Ҫ���ǲ����ӿյ�������ַ��Ȩֵ��
            for (j = 0; j < ADDR_MAX_TYPE_NUM; j++)
            {
                if ((j != i) && (addr_info.addr_buf[j] != 0))
                {
                    addr_info.weighted_val_buf[j]++;
                }
            }
        }
        else
        {
            // ���flash��û�ж�Ӧ���͵�������ַ������һ���յĵط�������
            // ���û�пյĵط���������ɵ����ͺ�������ַ

            for (i = 0; i < ADDR_MAX_TYPE_NUM; i++)
            {
                if ((addr_info.type_buf[i] == 0) && (addr_info.addr_buf[i] == 0) && (addr_info.weighted_val_buf[i] == 0))
                {
                    // ����пյĵط�
                    addr_info.type_buf[i] = (appear_themost_ele & 0xFF); // ��ֵ��Ϊ�������ͣ���������
                    addr_info.addr_buf[i] = (appear_themost_ele >> 8);   // ����������ַ
                    addr_info.weighted_val_buf[i] = 0;                   // Ȩֵ����Ϊ0����ʾ��������д���

                    // �����������͵�������ַ��Ȩֵ������Ҫ���ǲ����ӿյ�������ַ��Ȩֵ��
                    for (j = 0; j < ADDR_MAX_TYPE_NUM; j++)
                    {
                        if ((j != i) && (addr_info.addr_buf[j] != 0))
                        {
                            addr_info.weighted_val_buf[j]++;
                        }
                    }
                    isSpare = 1; // �п���ĵط��������Ѿ���ɱ���
                    break;
                }
            }

            // ���û�п���ĵط����򸲸���ɵ����ͺ�������ַ
            if (0 == isSpare)
            {
                // ͨ��ѭ���ҵ���ɵ��������ڵ������±�
                for (i = 0; i < ADDR_MAX_TYPE_NUM; i++)
                {
                    if (addr_info.weighted_val_buf[i] > temp)
                    {
                        temp = addr_info.weighted_val_buf[i];
                        the_oldest_index = i;
                    }
                }

                addr_info.type_buf[the_oldest_index] = (appear_themost_ele & 0xFF); // ��ֵ��Ϊ��������
                addr_info.addr_buf[the_oldest_index] = (appear_themost_ele >> 8);   // ����������ַ
                addr_info.weighted_val_buf[the_oldest_index] = 0;

                // �����������͵�������ַ��Ȩֵ������Ҫ���ǲ����ӿյ�������ַ��Ȩֵ��
                for (j = 0; j < ADDR_MAX_TYPE_NUM; j++)
                {
                    if ((j != the_oldest_index) && (addr_info.addr_buf[j] != 0))
                    {
                        addr_info.weighted_val_buf[j]++;
                    }
                }
            }
        }

        // ��������ַд��flash
        flash_erase_sector(FLASH_DEVICE_START_ADDR);
        flash_write(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));
    }
}

// �жϵ�ǰ���յ����ݵĵ�ַ�Ƿ���flash�б���ĵ�ַƥ��
// ��Ҫ��ѧϰ����flash�ж�������
// ���أ� 0--��ƥ�䣬1--ƥ��
u8 rf_addr_isMatch(void)
{
    u32 i = 0;
    for (i = 0; i < ADDR_MAX_TYPE_NUM; i++)
    {
        if (addr_info.addr_buf[i] == (rf_data >> 8))
        {
            return 1;
        }
    }

    return 0;
}

// ���Ժ������鿴flash�а����������������������ַ
void show_addr_info_save_by_type(void)
{
    u32 i;

    for (i = 0; i < ADDR_MAX_TYPE_NUM; i++)
    {
        send_keyval(addr_info.weighted_val_buf[i]);
        send_keyval(addr_info.type_buf[i]);
        send_keyval(addr_info.addr_buf[i]);
    }
}

#endif // end ifdef LEARN_BY_TYPE
