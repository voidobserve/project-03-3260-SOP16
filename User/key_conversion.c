// 将遥控器按键的键值转换成唯一的功能值的源文件
#include "key_conversion.h"
#include "rf_learn.h"

// 根据器件地址和键值，转换成键值对应的功能值
u8 key_to_funval(u16 addr, u8 key)
{
    u8 funval = 0x00;
    u8 remote_type = 0x55; // 存放遥控器类型的变量

    u8 i = 0;
    for (i = 0; i < ADDR_MAX_NUM; i++) // 根据地址，找到其对应的遥控器类型
    {
        if (addr_info.addr_buf[i] == addr)
        {
            remote_type = addr_info.remote_type[i];
            break;
        }
    }

    if (REMOTE_TYPE_SMALL_RM == remote_type)
    {
        // 如果是小遥控器
        // 根据它的键值来做转换
        switch (key)
        {
        case KEY_SMALL_RM_1: // 按键--上
            funval = KEY_FUNCTION_UP;
            break;
        case KEY_SMALL_RM_2: // 按键--左
            funval = KEY_FUNCTION_LEFT;
            break;
        case KEY_SMALL_RM_3: // 按键--电源
            funval = KEY_FUNCTION_POWER;
            break;
        case KEY_SMALL_RM_4: // 按键--右
            funval = KEY_FUNCTION_RIGHT;
            break;
        case KEY_SMALL_RM_5: // 按键--下
            funval = KEY_FUNCTION_DOWN;
            break;
        case KEY_SMALL_RM_6: // 按键--灯光、亮度
            funval = KEY_FUNCTION_LIGHT;
            break;
        case KEY_SMALL_RM_7: // 按键--音乐，按钮上是个音乐图标
            funval = KEY_FUNCTION_MUSIC_ICON;
            break;
        case KEY_SMALL_RM_8: // 按键--自动
            funval = KEY_FUNCTION_AUTO;
            break;
        case KEY_SMALL_RM_9: // 按键--列表
            funval = KEY_FUNCTION_LIST;
            break;
        case KEY_SMALL_RM_10: // 按键--RAINBOW，按钮上就是这个字样
            funval = KEY_FUNCTION_RAINBOW;
            break;
        }
    }
    else if (REMOTE_TYPE_BIG_RM == remote_type)
    {
        switch (key)
        {
        case KEY_BIG_RM_1: // 按键--电源
            funval = KEY_FUNCTION_POWER;
            break;
        case KEY_BIG_RM_2: // 按键--静音
            funval = KEY_FUNCTION_MUTE;
            break;
        case KEY_BIG_RM_3: // 按键--上
            funval = KEY_FUNCTION_UP;
            break;
        case KEY_BIG_RM_4: // 按键--左
            funval = KEY_FUNCTION_LEFT;
            break;
        case KEY_BIG_RM_5: // 按键--播放暂停
            funval = KEY_FUNCTION_PLAY_PAUSE;
            break;
        case KEY_BIG_RM_6: // 按键--右
            funval = KEY_FUNCTION_RIGHT;
            break;
        case KEY_BIG_RM_7: // 按键--下
            funval = KEY_FUNCTION_DOWN;
            break;
        case KEY_BIG_RM_8: // 按键--音乐，按钮上是个MUSIC字样
            funval = KEY_FUNCTION_MUSIC_FONT;
            break;
        case KEY_BIG_RM_9: // 按键--模式
            funval = KEY_FUNCTION_MODE;
            break;
        case KEY_BIG_RM_10: // 按键--影视，按钮上是MOVIE字样
            funval = KEY_FUNCTION_MOVIE;
            break;
        case KEY_BIG_RM_11: // 按键--TREB+，高音+
            funval = KEY_FUNCTION_TREB_ADD;
            break;
        case KEY_BIG_RM_12: // 按键--新闻，NEWS
            funval = KEY_FUNCTION_NEWS;
            break;
        case KEY_BIG_RM_13: // 按键--低音+，BASS+
            funval = KEY_FUNCTION_BASS_ADD;
            break;
        case KEY_BIG_RM_14: // 按键--高音-，TREB-
            funval = KEY_FUNCTION_TREB_SUB;
            break;
        case KEY_BIG_RM_15: // 按键--显示，DISPLAY
            funval = KEY_FUNCTION_DISPLAY;
            break;
        case KEY_BIG_RM_16: // 按键--低音-，BASS--
            funval = KEY_FUNCTION_BASS_SUB;
            break;
        }
    }

    return funval;
}
