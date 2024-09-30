// 定时器TMR2的驱动源文件
#include "tmr2.h"

volatile unsigned char tmr2_flag = 0; // tmr2中断服务函数中会置位的标志位
volatile u32 tmr2_cnt = 0; // 定时器TMR2的计数值（每次在中断服务函数中会加一）

/**
 * @brief 配置定时器TMR2
 */
void tmr2_config(void)
{
    // 配置定时器，用来记录RF接收到的高电平持续时间
    __SetIRQnIP(TMR2_IRQn, TMR2_IQn_CFG); // 设置中断优先级（TMR2）

    TMR2_CONL &= ~TMR_PRESCALE_SEL(0x03); // 清除TMR2的预分频配置寄存器
    // 配置TMR2的预分频，为32分频，即21MHz / 32 = 0.67MHz，约0.67us计数一次
    // （实际测试和计算得出这个系统时钟是21MHz的，但是还是有些误差，不是准确的21MHz）
    TMR2_CONL |= TMR_PRESCALE_SEL(0x05);
    TMR2_CONL &= ~TMR_MODE_SEL(0x03); // 清除TMR2的模式配置寄存器
    TMR2_CONL |= TMR_MODE_SEL(0x01);  // 配置TMR2的模式为计数器模式，最后对系统时钟的脉冲进行计数

    TMR2_CONH &= ~TMR_PRD_PND(0x01); // 清除TMR2的计数标志位，表示未完成计数
    TMR2_CONH |= TMR_PRD_IRQ_EN(1);  // 使能TMR2的计数中断

    // 配置TMR2的计数周期
    TMR2_PRL = (unsigned char)(TMR2_CNT_TIME % 255);
    TMR2_PRH = (unsigned char)(TMR2_CNT_TIME / 255);

    // 清除TMR2的计数值
    TMR2_CNTL = 0;
    TMR2_CNTH = 0;

    TMR2_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除TMR2的时钟源配置寄存器
    // TMR2_CONL |= TMR_SOURCE_SEL(0x07); // 配置TMR2的时钟源，使用系统时钟
    TMR2_CONL |= TMR_SOURCE_SEL(0x05); // 配置TMR2的时钟源，不用任何时钟
                                       // __EnableIRQ(TMR2_IRQn);			   // 使能中断

    __DisableIRQ(TMR2_IRQn); // 禁用中断
}

/**
 * @brief 开启定时器TMR2，开始计时
 */
void tmr2_enable(void)
{
    // 重新给TMR2配置时钟
    TMR2_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR2_CONL |= TMR_SOURCE_SEL(0x06);    // 配置定时器的时钟源，使用系统时钟（约21MHz）

    __EnableIRQ(TMR2_IRQn); // 使能中断
}

/**
 * @brief 关闭定时器2，清空计数值
 */
void tmr2_disable(void)
{
    // 不给定时器提供时钟，让它停止计数
    TMR2_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR2_CONL |= TMR_SOURCE_SEL(0x05);    // 配置定时器的时钟源，不用任何时钟

    // 清除定时器的计数值
    TMR2_CNTL = 0;
    TMR2_CNTH = 0;

    __DisableIRQ(TMR2_IRQn); // 关闭中断（不使能中断）
}

// TMR2中断服务函数
void TIMR2_IRQHandler(void) interrupt TMR2_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR2_IRQn);

    // ---------------- 用户函数处理 -------------------

    // 周期中断
    if (TMR2_CONH & TMR_PRD_PND(0x1))
    {
        TMR2_CONH |= TMR_PRD_PND(0x1); // 清除pending

        tmr2_cnt++; // 每5ms加一次

        // P12 = ~P12; // 测试用，看看中断触发周期是否正确

        // 如果到了5s
        if (tmr2_cnt == 1000)
        {
            tmr2_cnt = 0;
            tmr2_flag = 1; // 给对应的标志位置一
        }
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR2_IRQn);
}
