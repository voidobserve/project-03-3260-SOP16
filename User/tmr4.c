#include "tmr4.h"

#define TMR4_CNT_TIME 1520 // 1520 * 0.65625us 约等于1000us，1ms

volatile u32 tmr4_cnt = 0; // 定时器TMR4的计数值（每次在中断服务函数中会加一）
// volatile bit tmr4_flag = 0; // tmr4中断服务函数中会置位的标志位

// 定时器TMR4的初始化（初始化完成后，默认是关闭的）
void tmr4_config(void)
{
    __EnableIRQ(TMR4_IRQn);                                   // 打开TIMER4模块中断
    TMR_ALLCON = TMR4_CNT_CLR(0x1);                           // Timer4 计数清零
    TMR4_CAP10 = TMR4_PRD_VAL_L((TMR4_CNT_TIME >> 0) & 0xFF); // 周期低八位寄存器
    TMR4_CAP11 = TMR4_PRD_VAL_H((TMR4_CNT_TIME >> 8) & 0xFF); // 周期高八位寄存器
    TMR4_CON0 = TMR4_PRESCALE_SEL(0x05) |                     // 32分频
                TMR4_SOURCE_SEL(0x7) |                        // 计数源选择系统时钟
                TMR4_MODE_SEL(0x0);                           // 计数模式
    TMR4_IE0 = TMR4_PRD_IRQ_EN(0x1);                          // 使能计数值等于周期值中断
    TMR4_EN = TMR4_EN(0x0);                                   // 不使能定时器

    IE_EA = 1; // 使能总中断
}

// 开启定时器TMR4，开始计时
void tmr4_enable(void)
{
    TMR4_EN = TMR4_EN(0x1); // 使能定时器
}

// 关闭定时器，清空计数值
void tmr4_disable(void)
{
    TMR4_EN = TMR4_EN(0x0);         // 不使能定时器
    TMR_ALLCON = TMR4_CNT_CLR(0x1); // Timer4 计数清零
}

// TMR4中断服务函数
void TIMR4_IRQHandler(void) interrupt TMR4_IRQn
{
#if 1 // 定时器的定时中断
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR4_IRQn);

    // ---------------- 用户函数处理 -------------------
    // 计数值等于周期值中断
    if (TMR4_FLAG0 & TMR4_PRD_FLAG(0x1))
    {
        TMR4_CLR0 |= TMR4_PRD_FLG_CLR(0x1);

        tmr4_cnt++;

        // P24 = ~P24; // 测试用，看看中断触发时间是否一致
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR4_IRQn);
#endif
}
