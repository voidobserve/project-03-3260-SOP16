// оƬ�ڲ���flash��ز�����Դ����
#include "flash.h"

#define FLASH_START_ADDR (0x00) // ��ʼ��ַ128byte����

/**
 * @brief  flash erase sector
 * @param  addr : sector address in flash
 * @retval None
 */
void flash_erase_sector(u8 addr)
{
    FLASH_ADDR = 0x3F;
    FLASH_ADDR = addr;
    FLASH_PASSWORD = FLASH_PASSWORD(0xB9); // д���������
    FLASH_CON = FLASH_SER_TRG(0x1);        // ������������

    while (!(FLASH_STA & FLASH_SER_FLG(0x1)))
        ; // �ȴ�������������
}

/**
 * @brief  flash program
 * @param  addr   : Write data address in flash
 * @param  p_data : Write data to flash
 * @param  len    : Data length
 * @retval None
 */
void flash_write(u8 addr, u8 *p_data, u8 len)
{
    FLASH_ADDR = 0x3F;
    FLASH_ADDR = addr;

    while (len >= 1)
    {
        while (!(FLASH_STA & FLASH_PROG_FLG(0x1)))
            ; // �ȴ���¼����
        FLASH_DATA = *(p_data++);
        FLASH_PASSWORD = FLASH_PASSWORD(0xB9); // д���������
        FLASH_CON = FLASH_PROG_TRG(0x1);       // ������¼

        len -= 1;
    }
}

/**
 * @brief  flash program
 * @param  addr   : Read data address in flash
 * @param  p_data : Read data to flash
 * @param  len    : Data length
 * @retval None
 */
void flash_read(u8 addr, u8 *p_data, u8 len)
{
    while (len != 0)
    {
        *(p_data++) = *((u8 code *)(0x3F00 + addr++));
        len--;
    }
}


// ���Ժ����������ܹ�ʵ��flash�Ķ�д
// ʹ��ǰ��Ҫ�ȳ�ʼ��P12
void flash_test(void)
{
    unsigned int device_addr = 0x12345678;
    unsigned int buf = 0;
    
    flash_erase_sector(0x00);
    // д������
    flash_write(0x00, (unsigned char *)&device_addr, sizeof(device_addr));
    // ��ȡ����
    flash_read(0x00, (unsigned char *)&buf, sizeof(buf));

    if ((const unsigned int)0x12345678 == buf)
    {
        P12 = 0;
    }
}
