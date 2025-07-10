// 学习功能（保存器件地址相关）的源文件
#include "rf_learn.h" //
#include "flash.h"    // 包含单片机的flash相关操作
#include "rf_recv.h"  // rf解码相关操作
#include "send_key.h" // 发送键值，测试时用
#include "tmr2.h"     // 使用定时器TMR2来实现5s延时

#include "timer0.h"
// #include "stimer0.h"

#include <stdlib.h> // 使用了NULL

addr_info_t addr_info = {0}; // 存放从flash中读出的器件地址

#ifdef LEARN_BY_QUANTITY // 保存n个器件地址
/**
 * @brief RF学习功能，5s内检测到有按键长按，则将这个遥控器的器件地址写入到flash中（一般放在上电的5s中使用）
 *          如果 my_config.h文件中的USE_RF_UNPAIR为1，那么当收到有与flash中相同的器件地址传来的信号时，
 *          会从flash中删除该地址数据，实现取消配对
 */
void rf_learn(void)
{
    u32 appear_themost_ele = 0; // RF接收数组中出现最多次数的元素

    u32 i = 0; // 循环计数值
    u32 j = 0; // 循环计数值

    bit isSpare = 0; // 标志位，flash中是否有空闲位置来存储空的地址

    u32 temp = 0;             // 临时变量，存放临时值
    u32 the_oldest_index = 0; // 最旧的器件地址对应的数组下标

    // 打开定时器
    tmr2_enable();

    while (tmr2_flag != 1)
    {
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗
        rf_recv_databuf();           // 每次循环，试着读取一次数据

        // 这段时间内，一直清空 触摸按键模块初始化的倒计时
        touch_cnt_down_clear();
    }

    // 5s后，关闭定时器，清空它硬件的计数值
    tmr2_disable();
    tmr2_flag = 0; // 清除标志位
    tmr2_cnt = 0;  // 清除定时器的计数值

    // 从flash中读取器件地址
    flash_read(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));

    // 如果这段时间内检测到有按键长按，使得标志位置一，进行相关的处理
    if (rf_data_buf_overflow)
    {
        rf_data_buf_overflow = 0;
        // 找出数组中出现次数最多的元素
        appear_themost(rf_data_buf, sizeof(rf_data_buf) / sizeof(rf_data_buf[0]), &appear_themost_ele, NULL);

        // 如果不是电源按键(学习按键)，函数退出
        if (((appear_themost_ele & 0xFF) != 0x01) && ((appear_themost_ele & 0xFF) != 0x03))
        {
            return;
        }

        // 如果是电源按键
        // 将器件地址写入到单片机的flash中
        // |--如果没有则直接写入，如果有则覆盖最旧的器件地址，再一次性写入

        // 通过循环判断flash中是否已经有对应的地址
        for (i = 0; i < ADDR_MAX_NUM; i++)
        {
            if (addr_info.addr_buf[i] == (appear_themost_ele >> 8))
            {
#if USE_RF_UNPAIR // 如果开启 取消配对

                // 若器件地址相同，清除对应的数据
                addr_info.addr_buf[i] = 0;
                addr_info.weighted_val_buf[i] = 0; // 清除权值
                addr_info.remote_type[i] = 0;      // 清除遥控器类型

                // 将器件地址写回flash
                flash_erase_sector(FLASH_DEVICE_START_ADDR);
                flash_write(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));

                return; // 写入完成，函数直接返回

#else // 如果没有开启 取消配对

                addr_info.weighted_val_buf[i] = 0;
                addr_info.addr_buf[i] = (appear_themost_ele >> 8); // 保存地址这一步可以不用加，因为此时地址是相同的

                if (0x01 == (appear_themost_ele & 0xFF))
                {
                    // 如果是大摇控器上面的电源按键/学习按键
                    addr_info.remote_type[i] = REMOTE_TYPE_BIG_RM;
                }
                else if (0x03 == (appear_themost_ele & 0xFF))
                {
                    // 如果是小遥控器上面的电源按键/学习按键
                    addr_info.remote_type[i] = REMOTE_TYPE_SMALL_RM;
                }

                // 增加其他地址的权值（不增加空的地址的权值）
                for (j = 0; j < ADDR_MAX_NUM; j++)
                {
                    if ((j != i) && (addr_info.addr_buf[j] != 0))
                    {
                        addr_info.weighted_val_buf[j]++;
                    }
                }

                // 最后，将器件地址写回flash
                flash_erase_sector(FLASH_DEVICE_START_ADDR);
                flash_write(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));
                return; // 结束学习

#endif // end if USE_RF_UNPAIR
            }
        }

        // 如果要保存的地址在flash中并没有重复
        // |---找一处空的地方保存，如果没有，则覆盖最旧的器件地址
        for (i = 0; i < ADDR_MAX_NUM; i++)
        {
            if ((addr_info.weighted_val_buf[i] == 0) && (addr_info.addr_buf[i] == 0))
            {
                // 如果有空的地方，直接保存
                addr_info.weighted_val_buf[i] = 0;
                addr_info.addr_buf[i] = (appear_themost_ele >> 8);

                if (0x01 == (appear_themost_ele & 0xFF))
                {
                    // 如果是大摇控器上面的电源按键/学习按键
                    addr_info.remote_type[i] = REMOTE_TYPE_BIG_RM;
                }
                else if (0x03 == (appear_themost_ele & 0xFF))
                {
                    // 如果是小遥控器上面的电源按键/学习按键
                    addr_info.remote_type[i] = REMOTE_TYPE_SMALL_RM;
                }

                // 增加其他地址的权值（不增加空的地址的权值）
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

        // 如果没有空余的地方，则覆盖最旧的器件地址
        if (0 == isSpare)
        {
            // 通过循环找到最旧的地址所在的数组下标
            for (i = 0; i < ADDR_MAX_NUM; i++)
            {
                if (addr_info.weighted_val_buf[i] > temp)
                {
                    temp = addr_info.weighted_val_buf[i];
                    the_oldest_index = i;
                }
            }

            addr_info.addr_buf[the_oldest_index] = (appear_themost_ele >> 8); // 保存器件地址
            addr_info.weighted_val_buf[the_oldest_index] = 0;                 // 权值设置为0

            if (0x01 == (appear_themost_ele & 0xFF))
            {
                // 如果是大摇控器上面的电源按键/学习按键
                addr_info.remote_type[the_oldest_index] = REMOTE_TYPE_BIG_RM;
            }
            else if (0x03 == (appear_themost_ele & 0xFF))
            {
                // 如果是小遥控器上面的电源按键/学习按键
                addr_info.remote_type[the_oldest_index] = REMOTE_TYPE_SMALL_RM;
            }

            // 增加其他的器件地址的权值（这里要考虑不增加空的器件地址的权值）
            for (j = 0; j < ADDR_MAX_NUM; j++)
            {
                if ((j != the_oldest_index) && (addr_info.addr_buf[j] != 0))
                {
                    addr_info.weighted_val_buf[j]++;
                }
            }
        }

        // 最后，将器件地址写回flash
        flash_erase_sector(FLASH_DEVICE_START_ADDR);
        flash_write(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));
    }
}

// 判断当前接收的数据的地址是否与flash中保存的地址匹配
// 需要先学习，从flash中读出数据
// 返回： 0--不匹配，1--匹配
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

// 测试函数，查看flash中按不同地址来保存的所有器件地址
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
 * @brief RF学习功能，5s内检测到有按键长按，则将这个遥控器的器件地址写入到flash中（一般放在上电的5s中使用）
 *          如果 my_config.h文件中的USE_RF_UNPAIR为1，那么当收到有与flash中相同的器件地址传来的信号时，
 *          会从flash中删除该地址数据，实现取消配对
 */
void rf_learn(void)
{
    u32 appear_themost_ele = 0; // RF接收数组中出现最多次数的元素

    int32 i = 0; // 循环计数值
    int32 j = 0; // 循环计数值

    u8 isExist = 0; // 标志位，地址是否已经在flash保存
    u8 isSpare = 0; // 标志位，标志存放地址的容器中是否有空的地方

    u32 temp = 0;             // 存放临时值
    u32 the_oldest_index = 0; // 最旧的器件地址对应的数组下标

    // 打开定时器
    tmr2_enable();

    while (tmr2_flag != 1)
    {
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗
        rf_recv_databuf();           // 每次循环，试着读取一次数据
    }

    // 5s后，关闭定时器，清空它硬件的计数值
    tmr2_disable();
    tmr2_flag = 0; // 清除标志位
    tmr2_cnt = 0;  // 清除定时器的计数值

    // 从flash中读取器件地址
    flash_read(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));

    // 如果这段时间内（5s内）检测到有按键长按，使得标志位置一，进行相关的处理
    if (rf_data_buf_overflow)
    {
        rf_data_buf_overflow = 0;
        // 找出数组中出现次数最多的元素
        appear_themost(rf_data_buf, sizeof(rf_data_buf) / sizeof(rf_data_buf[0]), &appear_themost_ele, NULL);

        // 如果不是电源按键，函数退出（待补充）

        // 如果是电源按键
        // 将器件地址写入到单片机的flash中
        // |--如果没有则直接写入，如果有则覆盖最旧的器件地址，再一次性写入
        for (i = 0; i < ADDR_MAX_TYPE_NUM; i++)
        {
            if (addr_info.type_buf[i] == (appear_themost_ele & 0xFF))
            {
                // 如果flash中已经有对应类型的器件地址
                isExist = 1;
                break;
            }
        }

        if (isExist)
        {
            // 如果flash中已经有对应类型的器件地址
            // 判断器件地址是否相同
            if (addr_info.addr_buf[i] == (appear_themost_ele >> 8))
            {
#if USE_RF_UNPAIR // 如果使用了取消配对

                // 若器件地址相同，清除对应的数据
                addr_info.addr_buf[i] = 0;
                addr_info.type_buf[i] = 0;
                addr_info.weighted_val_buf[i] = 0;

                // 将器件地址写回flash
                flash_erase_sector(FLASH_DEVICE_START_ADDR);
                flash_write(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));

                return; // 写入完成后，函数可以直接返回了

#else // 如果不使用取消配对

                // 若器件地址相同则不用覆盖，直接退出
                return;

#endif // end if USE_RF_UNPAIR
            }

            addr_info.addr_buf[i] = (appear_themost_ele >> 8); // 覆盖器件地址
            addr_info.weighted_val_buf[i] = 0;                 // 权值设置为0，表示它是最新写入的

            // 增加其他类型的器件地址的权值（这里要考虑不增加空的器件地址的权值）
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
            // 如果flash中没有对应类型的器件地址，则找一处空的地方，保存
            // 如果没有空的地方，覆盖最旧的类型和器件地址

            for (i = 0; i < ADDR_MAX_TYPE_NUM; i++)
            {
                if ((addr_info.type_buf[i] == 0) && (addr_info.addr_buf[i] == 0) && (addr_info.weighted_val_buf[i] == 0))
                {
                    // 如果有空的地方
                    addr_info.type_buf[i] = (appear_themost_ele & 0xFF); // 键值作为器件类型，保存起来
                    addr_info.addr_buf[i] = (appear_themost_ele >> 8);   // 保存器件地址
                    addr_info.weighted_val_buf[i] = 0;                   // 权值设置为0，表示它是最新写入的

                    // 增加其他类型的器件地址的权值（这里要考虑不增加空的器件地址的权值）
                    for (j = 0; j < ADDR_MAX_TYPE_NUM; j++)
                    {
                        if ((j != i) && (addr_info.addr_buf[j] != 0))
                        {
                            addr_info.weighted_val_buf[j]++;
                        }
                    }
                    isSpare = 1; // 有空余的地方，并且已经完成保存
                    break;
                }
            }

            // 如果没有空余的地方，则覆盖最旧的类型和器件地址
            if (0 == isSpare)
            {
                // 通过循环找到最旧的类型所在的数组下标
                for (i = 0; i < ADDR_MAX_TYPE_NUM; i++)
                {
                    if (addr_info.weighted_val_buf[i] > temp)
                    {
                        temp = addr_info.weighted_val_buf[i];
                        the_oldest_index = i;
                    }
                }

                addr_info.type_buf[the_oldest_index] = (appear_themost_ele & 0xFF); // 键值作为器件类型
                addr_info.addr_buf[the_oldest_index] = (appear_themost_ele >> 8);   // 保存器件地址
                addr_info.weighted_val_buf[the_oldest_index] = 0;

                // 增加其他类型的器件地址的权值（这里要考虑不增加空的器件地址的权值）
                for (j = 0; j < ADDR_MAX_TYPE_NUM; j++)
                {
                    if ((j != the_oldest_index) && (addr_info.addr_buf[j] != 0))
                    {
                        addr_info.weighted_val_buf[j]++;
                    }
                }
            }
        }

        // 将器件地址写回flash
        flash_erase_sector(FLASH_DEVICE_START_ADDR);
        flash_write(FLASH_DEVICE_START_ADDR, (unsigned char *)(&addr_info), sizeof(addr_info));
    }
}

// 判断当前接收的数据的地址是否与flash中保存的地址匹配
// 需要先学习，从flash中读出数据
// 返回： 0--不匹配，1--匹配
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

// 测试函数，查看flash中按器件类型来保存的器件地址
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
