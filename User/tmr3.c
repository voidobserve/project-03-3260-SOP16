// 定时器TMR3的驱动源文件
#include "tmr3.h"

// 定时器TMR3的计时周期，也是中断触发周期（每隔多久触发一次中断）
// 计时周期不能大于65535，因为TMR3存放计时周期的寄存器只有16位
// 0.65625 * 15238 约等于 10000，即定时器每 10000us（10ms） 触发一次中断
// 实际上还需要调整，它的时钟不是很准确，因为不是晶振提供的，而是RC振荡提供的
#define TMR3_CNT_TIME ((15100)) 

volatile u32 tmr3_cnt = 0; // 定时器TMR3的计数值（每次在中断服务函数中会加一）

// 定时器TMR3的初始化（初始化完成后，默认是关闭的）
void tmr3_config(void)
{
    // 配置定时器，用来记录RF接收到的高电平持续时间
    __SetIRQnIP(TMR3_IRQn, TMR3_IQn_CFG); // 设置中断优先级（TMR3）

    TMR3_CONL &= ~TMR_PRESCALE_SEL(0x03); // 清除TMR3的预分频配置寄存器
    // 配置TMR3的预分频，为32分频，即21MHz / 32 = 0.65625MHz，约0.65625us计数一次
    // （实际测试和计算得出这个系统时钟是21MHz的，但是还是有些误差）
    TMR3_CONL |= TMR_PRESCALE_SEL(0x05);
    TMR3_CONL &= ~TMR_MODE_SEL(0x03); // 清除TMR3的模式配置寄存器
    TMR3_CONL |= TMR_MODE_SEL(0x01);  // 配置TMR3的模式为计数器模式，最后对系统时钟的脉冲进行计数

    TMR3_CONH &= ~TMR_PRD_PND(0x01); // 清除TMR3的计数标志位，表示未完成计数
    TMR3_CONH |= TMR_PRD_IRQ_EN(1);  // 使能TMR3的计数中断

    __DisableIRQ(TMR3_IRQn); // 禁用中断

    // 配置TMR3的计数周期
    TMR3_PRL = (unsigned char)(TMR3_CNT_TIME % 255);
    TMR3_PRH = (unsigned char)(TMR3_CNT_TIME / 255);

    // 清除TMR3的计数值
    TMR3_CNTL = 0;
    TMR3_CNTH = 0;

    TMR3_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除TMR3的时钟源配置寄存器
    // TMR3_CONL |= TMR_SOURCE_SEL(0x07); // 配置TMR3的时钟源，使用系统时钟
    TMR3_CONL |= TMR_SOURCE_SEL(0x05); // 配置TMR3的时钟源，不用任何时钟
                                       // __EnableIRQ(TMR3_IRQn);			   // 使能中断
}

// 开启定时器TMR3，开始计时
void tmr3_enable(void)
{
    // 重新给TMR3配置时钟
    TMR3_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR3_CONL |= TMR_SOURCE_SEL(0x06);    // 配置定时器的时钟源，使用系统时钟（约21MHz）

    __EnableIRQ(TMR3_IRQn); // 使能中断
}

// 关闭定时器，清空计数值
void tmr3_disable(void)
{
    // 不给定时器提供时钟，让它停止计数
    TMR3_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
    TMR3_CONL |= TMR_SOURCE_SEL(0x05);    // 配置定时器的时钟源，不用任何时钟

    // 清除定时器的计数值
    TMR3_CNTL = 0;
    TMR3_CNTH = 0;

    __DisableIRQ(TMR3_IRQn); // 关闭中断（不使能中断）
}

// TMR3中断服务函数
void TIMR3_IRQHandler(void) interrupt TMR3_IRQn
{
    // 进入中断设置IP，不可删除
    __IRQnIPnPush(TMR3_IRQn);

    // ---------------- 用户函数处理 -------------------
    // 周期中断
    if (TMR3_CONH & TMR_PRD_PND(0x1))
    {
        TMR3_CONH |= TMR_PRD_PND(0x1); // 清除pending

        tmr3_cnt++; // 

        // P12 = ~P12; // 测试用，看看中断触发周期是否正确
    }
    
    // 退出中断设置IP，不可删除
    __IRQnIPnPop(TMR3_IRQn);
}
