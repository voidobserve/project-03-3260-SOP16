// GPIO配置的源文件
#include "my_gpio.h"

// 输出引脚初始化（测试用）
// P1_1
void output_pin_init(void)
{
    P1_MD0 &= ~(GPIO_P11_MODE_SEL(0x1)); // 对应的寄存器先清零
    P1_MD0 |= GPIO_P11_MODE_SEL(0x1);    // 配置为输出模式

    FOUT_S11 = GPIO_FOUT_AF_FUNC; // 选择AF功能输出

    // 位操作
    P11 = 1;
}

/**
 * @brief P12配置为输出模式
*/
void p12_output_config(void)
{
    P1_MD0 &= ~(GPIO_P12_MODE_SEL(0x1)); // 对应的寄存器先清零
    P1_MD0 |= GPIO_P12_MODE_SEL(0x1);    // 配置为输出模式

    FOUT_S12 = GPIO_FOUT_AF_FUNC; // 选择AF功能输出

    P1_PU |= GPIO_P12_PULL_UP(0x01); // 内部的上拉电阻
    P1_PD |= GPIO_P12_PULL_PD(0x01); // 内部的下拉电阻
    // 配置完上下拉电阻后，这个引脚就相当于浮空状态，同时能够避免一些电路的干扰

    // 位操作
    P12 = 1;
}


/**
 * @brief P01配置为输出模式
 *        P01在开发板是连接到了LED6的正极
*/
void p01_output_config(void)
{
    P0_MD0 &= ~(GPIO_P01_MODE_SEL(0x1)); // 对应的寄存器先清零
    P0_MD0 |= GPIO_P01_MODE_SEL(0x1);    // 配置为输出模式

    FOUT_S01 = GPIO_FOUT_AF_FUNC; // 选择AF功能输出

    P0_PU |= GPIO_P01_PULL_UP(0x01); // 内部的上拉电阻
    P0_PD |= GPIO_P01_PULL_PD(0x01); // 内部的下拉电阻
    // 配置完上下拉电阻后，这个引脚就相当于浮空状态，同时能够避免一些电路的干扰

    // 位操作
    P01 = 0;
}


/**
 * @brief P26配置为输出模式
*/
void p26_output_config(void)
{
    P2_MD1 &= ~(GPIO_P26_MODE_SEL(0x1)); // 对应的寄存器先清零
    P2_MD1 |= GPIO_P26_MODE_SEL(0x1);    // 配置为输出模式

    FOUT_S26 = GPIO_FOUT_AF_FUNC; // 选择AF功能输出

    P2_PU |= GPIO_P26_PULL_UP(0x01); // 内部的上拉电阻
    P2_PD |= GPIO_P26_PULL_PD(0x01); // 内部的下拉电阻
    // 配置完上下拉电阻后，这个引脚就相当于浮空状态，同时能够避免一些电路的干扰

    // 位操作
    P26 = 0;
}



