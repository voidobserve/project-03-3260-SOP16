#include "stimer0.h"

// 触摸按键模块初始化的倒计时
static volatile u32 touch_init_cnt = 0;

volatile bit flag_is_reinitialize_touch = 0; // 是否需要重新初始化触摸按键

// 清零触摸按键模块初始化的倒计时
void touch_cnt_down_clear(void)
{
    touch_init_cnt = 0;
    flag_is_reinitialize_touch = 0;
}

void stimer0_config(void)
{
    // STIMER0配置中断频率为1kHz
    __EnableIRQ(STMR0_IRQn);                                    // 使能STMR0模块中断
    STMR0_PSC = STMR_PRESCALE_VAL(0x00);                        // 不分频
    STMR0_PRH = STMR_PRD_VAL_H((STMR0_PEROID_VAL >> 8) & 0xFF); // 周期高八位寄存器
    STMR0_PRL = STMR_PRD_VAL_L((STMR0_PEROID_VAL >> 0) & 0xFF); // 周期低八位寄存器
    STMR0_IE = STMR_ZERO_IRQ_EN(0x1);                           // 计数值等于0中断
    STMR_CNTMD |= STMR_0_CNT_MODE(0x1);                         // 选择连续计数模式
    STMR_LOADEN |= STMR_0_LOAD_EN(0x1);                         // 自动装载使能
}

void STMR0_IRQHandler(void) interrupt STMR0_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(STMR0_IRQn);

    // ---------------- 用户函数处理 -------------------

    // 计数值等于0中断
    if (STMR0_IF & STMR_ZERO_FLAG(0x1))
    {
        STMR0_IF |= STMR_ZERO_FLAG(0x1); // 清中断标志位

        if (touch_init_cnt < 4294967296) // 防止计数溢出 （2^32）
        {
            touch_init_cnt++;
            if (touch_init_cnt >= 5000)
            {
                touch_init_cnt = 0;
                flag_is_reinitialize_touch = 1;
            }
        }
    }

    // 退出中断设置IP，不可删除
    __IRQnIPnPop(STMR0_IRQn);
}