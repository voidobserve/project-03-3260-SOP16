// 用于发送键值的源程序，使用到了定时器TMR1
#include "send_key.h"
#include "tmr2.h" // 定时器TMR2可用于发送格式头的延时和每次发送间隔的延时

volatile char send_keyval_flag = 0; // 是否要发送键值的标志位，0--不发送，1--发送

static volatile unsigned short key_val = 0; // 要发送的16位数据，高位先行MSB

// 发送键值信号引脚的初始化函数
// 引脚的默认状态为低电平
void send_keyval_pin_init(void)
{
#ifdef DEVELOPMENT_BOARD

    P1_MD0 &= ~(GPIO_P11_MODE_SEL(0x1)); // 对应的寄存器先清零
    P1_MD0 |= GPIO_P11_MODE_SEL(0x1);    // P1_1配置为输出模式

    P1_PU |= GPIO_P11_PULL_UP(0x01); // 内部的上拉电阻

    FOUT_S11 = GPIO_FOUT_AF_FUNC; // 选择AF功能输出（看手册上是数字复用功能输出，是输出逻辑高低电平用的）

    // 位操作，让p1_1输出高电平（默认让引脚处于空闲状态，空闲状态为高电平）
    P11 = 1;

#endif // end of #ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD

    P3_MD0 &= ~(GPIO_P31_MODE_SEL(0x1)); // 对应的寄存器先清零
    P3_MD0 |= GPIO_P31_MODE_SEL(0x1);    // P2_1配置为输出模式

    P3_PU |= GPIO_P31_PULL_UP(0x01); // 内部的上拉电阻

    FOUT_S31 = GPIO_FOUT_AF_FUNC; // 选择AF功能输出（看手册上是数字复用功能输出，是输出逻辑高低电平用的）

    // 位操作，让p2_1输出高电平（默认让引脚处于空闲状态，空闲状态为高电平）
    P31 = 1;

#endif // end of #ifdef CIRCUIT_BOARD
}

// 发送键值信号的引脚使用的定时器的初始化
// 这里使用TMR1，默认禁用中断，需要时再开启
void send_keyval_timer_init(void)
{
    // 配置定时器，用来控制输出键值的引脚输出
    __SetIRQnIP(TMR1_IRQn, TMR1_IQn_CFG); // 设置中断优先级（TMR1）

    TMR1_CONL &= ~TMR_PRESCALE_SEL(0x03); // 清除TMR1的预分频配置寄存器
    TMR1_CONL |= TMR_PRESCALE_SEL(0x05);  // 配置TMR1的预分频，为16分频，即21MHz / 32 = 0.65625MHz，约0.65625us计数一次
    TMR1_CONL &= ~TMR_MODE_SEL(0x03);     // 清除TMR1的模式配置寄存器
    TMR1_CONL |= TMR_MODE_SEL(0x01);      // 配置TMR1的模式为计数器模式，最后对HIRC的脉冲进行计数

    TMR1_CONH &= ~TMR_PRD_PND(0x01); // 清除TMR1的计数标志位，表示未完成计数
    TMR1_CONH |= TMR_PRD_IRQ_EN(1);  // 使能TMR1的计数中断

    // 配置TMR1的计数周期
    TMR1_PRL = TMR1_CNT_TIME % 255;
    TMR1_PRH = TMR1_CNT_TIME / 255;

    // 清除TMR1的计数值
    TMR1_CNTL = 0;
    TMR1_CNTH = 0;

    TMR1_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除TMR1的时钟源配置寄存器
    // TMR1_CONL |= TMR_SOURCE_SEL(0x07); // 配置TMR1的时钟源，使用系统时钟
    TMR1_CONL |= TMR_SOURCE_SEL(0x05); // 配置TMR1的时钟源，不用任何时钟
                                       // __EnableIRQ(TMR1_IRQn);			   // 使能中断

    __DisableIRQ(TMR1_IRQn); // 禁用中断
    // __EnableIRQ(TMR1_IRQn);
    // IE_EA = 1; // 使能总中断
}

// 开启定时器TMR1，开始计时
void tmr1_enable(void)
{
    // 重新给TMR1配置时钟
    TMR1_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR1_CONL |= TMR_SOURCE_SEL(0x06);    // 配置定时器的时钟源，使用系统时钟

    __EnableIRQ(TMR1_IRQn); // 使能中断
}

// 关闭定时器1，清空计数值
void tmr1_disable(void)
{
    // 不给定时器提供时钟，让它停止计数
    TMR1_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR1_CONL |= TMR_SOURCE_SEL(0x05);    // 配置定时器的时钟源，不用任何时钟

    // 清除定时器的计数值
    TMR1_CNTL = 0;
    TMR1_CNTH = 0;

    __DisableIRQ(TMR1_IRQn); // 关闭中断（不使能中断）
}

// 定时器TMR1中断服务函数
void TIMR1_IRQHandler(void) interrupt TMR1_IRQn
{
    // 每个要发送的二进制数据用定时器触发中断的四个周期来发送
    // 这里用3个周期的高电平+1个周期的低电平表示"1"
    // 用1个周期的高电平+3个周期的低电平表示"0"
    static volatile unsigned char cur_period = 0; // 当前周期

    static volatile unsigned char cur_bit = 0; // 当前要发送的第几位二进制数

    static volatile unsigned char last_period = 0; // 标志位，是否到了最后一个位的最后一个周期

    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR1_IRQn);

    // ---------------- 用户函数处理 -------------------
    // 周期中断
    if (TMR1_CONH & TMR_PRD_PND(0x1))
    {
        TMR1_CONH |= TMR_PRD_PND(0x1); // 清除pending

        // SEND_KEY_VAL_PIN = ~SEND_KEY_VAL_PIN; // 测试用，观察是不是能触发中断，以及每次中断触发的时间间隔

        if (send_keyval_flag == 1)
        {
            // 发送键值

            // 先判断当前要发送的是低几位二进制数，对应的逻辑值是多少
            if (key_val & (0x01 << (15 - cur_bit)))
            {
                // 如果当前要发送的是"1"

                // 判断当前要发送第几个周期了
                switch (cur_period)
                {
                case 0:
                    SEND_KEY_VAL_PIN = 1;
                    break;

                case 1:
                    SEND_KEY_VAL_PIN = 1;
                    break;

                case 2:
                    SEND_KEY_VAL_PIN = 1;
                    break;

                case 3:
                    SEND_KEY_VAL_PIN = 0;
                    break;
                }
            }
            else
            {
                // 如果当前要发送的是"0"

                // 判断当前要发送第几个周期了
                switch (cur_period)
                {
                case 0:
                    SEND_KEY_VAL_PIN = 1;
                    break;

                case 1:
                    SEND_KEY_VAL_PIN = 0;
                    break;

                case 2:
                    SEND_KEY_VAL_PIN = 0;
                    break;

                case 3:
                    SEND_KEY_VAL_PIN = 0;
                    break;
                }
            }

            // 发送完一个周期后，对应的计数值加一
            cur_period++;
            if (cur_period >= 4)
            {
                // 如果计数周期大于等于4了，说明发送完了1位二进制数
                cur_bit++;

                cur_period = 0; // 从下一位二进制数的第0个周期开始，准备下一次发送
            }

            if (cur_bit >= 16 && 0 == last_period)
            {
                // 如果发送到了第15位（0~15）二进制数的第四个周期（第4的周期还未完成）
                last_period = 1;
                cur_bit--;
                cur_period = 3;
            }
            else if (cur_bit >= 16 && 1 == last_period) // 如果发送完了16位二进制数
            {
                // 清空计数值，准备下一次发送
                cur_bit = 0;
                cur_period = 0;
                last_period = 0;

                send_keyval_flag = 0; // 清空对应的标志位，准备下一次键值的发送
                SEND_KEY_VAL_PIN = 1; // 输出键值的引脚设置为空闲状态，空前状态保持高电平
            }
        }
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR1_IRQn);
}

// 发送一次按键键值(总共16bit)
void send_keyval(unsigned short send_data)
{
    key_val = send_data; // 存放要发送的键值，定时器的中断服务函数会读取key_val中的值，然后发送出去

    // 先发送协议头（引脚空闲状态是高电平，用低电平作为协议头）
    SEND_KEY_VAL_PIN = 0;

    // delay_ms(5);
    tmr2_enable();       // 打开定时器TMR2，下面开始用定时器来实现延时5ms
    while (tmr2_cnt < 1) // TMR2开启5ms后才会将这个计数值加一
    {
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗
    }
    tmr2_disable(); // 关闭定时器TMR2，函数内部会把它硬件的计数值清零
    tmr2_flag = 0;  // 清除标志位
    tmr2_cnt = 0;   // tmr2_cnt = 0; // 清除定时器的计数值

    tmr1_enable(); // 打开定时器，发送键值数据
    send_keyval_flag = 1;
    while (send_keyval_flag != 0) // 等待发送完成
    {
        WDT_KEY = WDT_KEY_VAL(0xAA);
    };
    tmr1_disable(); // 关闭定时器

    delay_ms(10); // 每个键值至少间隔10ms（要求是5~10ms）

#if 0
    tmr2_enable();       // 打开定时器TMR2，这里通过复用定时器来实现延时10ms
    while (tmr2_cnt < 2) // TMR2开启5ms后才会将这个计数值加一
    {
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗
    }
    tmr2_disable(); // 关闭定时器TMR2，函数内部会把它硬件的计数值清零
    tmr2_flag = 0;  // 清除标志位
    tmr2_cnt = 0;   // tmr2_cnt = 0; // 清除定时器的计数值
#endif
}

// 发送带有按键状态信息的16位数据
// 前三位信息表示短按、长按、持续和长按后松开（相关定义在rf_scan.h中）
// 后五位信息表示键值
void send_status_keyval(u8 status, u8 key)
{
    u16 send_data = 0;

    // if (0 == key)
    // {
    //     return; // 不发送空的键值
    // }

    send_data = status << (5 + 8);  // 长短按信息（3bits）
    send_data |= (key & 0x1F) << 8; // 存放5位的键值
    send_data |= (~send_data >> 8); // 低8位存放反码
    send_keyval(send_data);
}
