// 芯片内部的flash相关操作的源程序
#include "flash.h"

#define FLASH_START_ADDR (0x00) // 起始地址128byte对齐

/**
 * @brief  flash erase sector
 * @param  addr : sector address in flash
 * @retval None
 */
void flash_erase_sector(u8 addr)
{
    FLASH_ADDR = 0x3F;
    FLASH_ADDR = addr;
    FLASH_PASSWORD = FLASH_PASSWORD(0xB9); // 写入操作密码
    FLASH_CON = FLASH_SER_TRG(0x1);        // 触发扇区擦除

    while (!(FLASH_STA & FLASH_SER_FLG(0x1)))
        ; // 等待扇区擦除空闲
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
            ; // 等待烧录空闲
        FLASH_DATA = *(p_data++);
        FLASH_PASSWORD = FLASH_PASSWORD(0xB9); // 写入操作密码
        FLASH_CON = FLASH_PROG_TRG(0x1);       // 触发烧录

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


// 测试函数，测试能够实现flash的读写
// 使用前需要先初始化P12
void flash_test(void)
{
    unsigned int device_addr = 0x12345678;
    unsigned int buf = 0;
    
    flash_erase_sector(0x00);
    // 写入数据
    flash_write(0x00, (unsigned char *)&device_addr, sizeof(device_addr));
    // 读取数据
    flash_read(0x00, (unsigned char *)&buf, sizeof(buf));

    if ((const unsigned int)0x12345678 == buf)
    {
        P12 = 0;
    }
}
