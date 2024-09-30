// RF-315MHz和RF-433MHz解码源程序
/*
	思路：通过IO中断+定时器，当上升沿时开始记录高电平持续时间，当下降沿时开始记录低电平持续时间

	1. 有高电平到来时，说明前面已经记录完一位信号，或是第一位信号到来
	2. 判断高电平和低电平持续时间时，范围要宽松些，以免出错
	3. 要注意一帧24位的数据到来时，前面是有较长的低电平信号，哪怕时连续的信号，前面也有10ms以上的低电平信号。
	根据这一点可以区分不同的帧，以免数据错位

*/
#include "rf_recv.h"
#include "my_gpio.h"
#include "send_key.h"
#include "flash.h" // 单片机上的flash相关操作

#include <stdlib.h> // NULL的定义

// 定时器TMR0的计时周期，也是中断触发周期（每隔多久触发一次中断）
// 计时周期不能大于65535，因为TMR0存放计时周期的寄存器只有16位
// 现在让定时器TMR0每100us触发一次中断（实际上有时候是120us，有时候是80us，好像是轮流来的）
#define TMR0_CNT_TIME ((152)) // 152 * 0.65625us == 99.75us，约等于100us （这是计算得出的，实际上会有误差，可能需要根据实情况来调试、修改）

static volatile int recv_bit_flag = 0; // 是否接收到完整的一位信号的标志位
static volatile u32 tmr0_cnt = 0;	   // 定时器中断服务函数中，使用到的计数值

static volatile u32 high_level_time = 0; // 用来单独保存定时器的计数值（定时时间），防止定时器重复定时，造成tmr0_cnt的数据覆盖
static volatile u32 low_level_time = 0;	 // 存放低电平持续的时间

volatile u32 rf_data = 0; // 用于存放接收到的24位数据

volatile int recv_rf_flag = 0; // 是否接收到完整的rf信号的标志位

// 用于存放接收到的24位数据，接收n次，再判断这里面有没有器件地址重复地、并且是出现次数最多的
// 如果有，则保存下它的器件地址
// 如果没有，则忽略
// 接收完整的一次信号四舍五入后约为50ms，这里接收20次，一共1000ms，也就是长按1s左右就可以学习、保存对应的地址
volatile u32 rf_data_buf[20] = {0};
static volatile unsigned char rf_data_buf_index = 0; // 接收数据缓冲区的索引
unsigned char rf_data_buf_overflow = 0;				 // 接收缓冲区溢出的标志位

// RFIN引脚初始化（RF接收引脚初始化）
// 这里也使用到了定时器TMR0，配置成每100us左右产生一次中断（误差20us），不过该函数调用完后，定时器TMR0是关闭的
void rfin_init(void)
{

#ifdef DEVELOPMENT_BOARD

	// 配置P0_2为输入模式
	P0_MD0 &= ~GPIO_P02_MODE_SEL(0x01);
	// 配置P0_2为下拉，因为RF接收模块的输出在空闲时默认为低电平
	P0_PD |= GPIO_P02_PULL_PD(0x01);

	__SetIRQnIP(P0_IRQn, P0_IQn_CFG);	// 设置中断优先级
	__EnableIRQ(P0_IRQn);				// 使能中断
	IE_EA = 1;							// 使能总开关
	P0_IMK |= GPIO_P02_IRQ_MASK(0x01);	// 打开P02的中断
	P0_TRG0 &= ~GPIO_P02_TRG_SEL(0x03); // 配置P02为双边沿触发

#endif // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD

	// 配置P1_3为输入模式
	P1_MD0 &= ~GPIO_P13_MODE_SEL(0x01);
	// 配置P1_3为下拉，因为RF接收模块的输出在空闲时默认为低电平
	P1_PD |= GPIO_P13_PULL_PD(0x01);

	__SetIRQnIP(P1_IRQn, P1_IQn_CFG);	// 设置中断优先级
	__EnableIRQ(P1_IRQn);				// 使能中断
	IE_EA = 1;							// 使能总开关
	P1_IMK |= GPIO_P13_IRQ_MASK(0x01);	// 打开P13的中断
	P1_TRG0 &= ~GPIO_P13_TRG_SEL(0x03); // 配置P13为双边沿触发

#endif // end ifdef CIRCUIT_BOARD

	// ============================================================ //
	// 配置定时器，用来记录RF接收到的高电平持续时间
	__SetIRQnIP(TMR0_IRQn, TMR0_IQn_CFG); // 设置中断优先级（TMR0）

	TMR0_CONL &= ~TMR_PRESCALE_SEL(0x03); // 清除TMR0的预分频配置寄存器

	// 配置TMR0的预分频，为32分频，即21MHz / 32 = 0.67MHz，约0.67us计数一次，
	// （实际测试和计算得出这个系统时钟是21MHz的，但是还是有些误差）
	TMR0_CONL |= TMR_PRESCALE_SEL(0x05);
	TMR0_CONL &= ~TMR_MODE_SEL(0x03); // 清除TMR0的模式配置寄存器
	TMR0_CONL |= TMR_MODE_SEL(0x01);  // 配置TMR0的模式为计数器模式，最后对系统时钟的脉冲进行计数

	TMR0_CONH &= ~TMR_PRD_PND(0x01); // 清除TMR0的计数标志位，表示未完成计数
	TMR0_CONH |= TMR_PRD_IRQ_EN(1);	 // 使能TMR0的计数中断

	// 配置TMR0的计数周期
	TMR0_PRL = (unsigned char)(TMR0_CNT_TIME % 255);
	TMR0_PRH = (unsigned char)(TMR0_CNT_TIME / 255);

	// 清除TMR0的计数值
	TMR0_CNTL = 0;
	TMR0_CNTH = 0;

	TMR0_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除TMR0的时钟源配置寄存器
	TMR0_CONL |= TMR_SOURCE_SEL(0x05);	  // 配置TMR0的时钟源，不用任何时钟
										  // __EnableIRQ(TMR0_IRQn);			   // 使能中断

	__DisableIRQ(TMR0_IRQn); // 禁用中断
}

// TMR0中断服务函数
void TIMR0_IRQHandler(void) interrupt TMR0_IRQn
{
	// 进入中断设置IP，不可删除
	__IRQnIPnPush(TMR0_IRQn);

	// ---------------- 用户函数处理 -------------------

	// 周期中断
	if (TMR0_CONH & TMR_PRD_PND(0x1))
	{
		TMR0_CONH |= TMR_PRD_PND(0x1); // 清除pending

		tmr0_cnt++; // 每80us或120us会加一次

		// P12 = ~P12;
	}

	// 退出中断设置IP，不可删除
	__IRQnIPnPop(TMR0_IRQn);
}

// 开启定时器TMR0，开始计时
void tmr0_enable(void)
{
	// 重新给TMR0配置时钟
	TMR0_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
	TMR0_CONL |= TMR_SOURCE_SEL(0x06);	  // 配置定时器的时钟源，使用系统时钟（约21MHz）

	__EnableIRQ(TMR0_IRQn); // 使能中断
}

// 关闭定时器0，清空计数值
void tmr0_disable(void)
{
	// 不给定时器提供时钟，让它停止计数
	TMR0_CONL &= ~(TMR_SOURCE_SEL(0x07)); // 清除定时器的时钟源配置寄存器
	TMR0_CONL |= TMR_SOURCE_SEL(0x05);	  // 配置定时器的时钟源，不用任何时钟

	// 清除定时器的计数值
	TMR0_CNTL = 0;
	TMR0_CNTH = 0;

	__DisableIRQ(TMR0_IRQn); // 关闭中断（不使能中断）
}


#ifdef DEVELOPMENT_BOARD
// P0中断服务函数
void P0_IRQHandler(void) interrupt P0_IRQn
{
	// Px_PND寄存器写任何值都会清标志位
	u8 p0_pnd = P0_PND;

	static volatile int i = 0; // 计数值，记录当前接收的数据位数

	// 进入中断设置IP，不可删除
	__IRQnIPnPush(P0_IRQn);
	__DisableIRQ(P0_IRQn); // 禁用IO口中断

	// ---------------- 用户函数处理 -------------------

	if (p0_pnd & GPIO_P02_IRQ_PNG(0x1)) // 清除标志位
	{
		// 在这里编写中断服务函数的代码

		// 根据引脚当前的电平来打开/关闭定时器TMR0，最后记录一个高电平信号的持续时间
		if (RFIN == 1)
		{
			// 如果此时引脚是高电平，读取记录的低电平的时间
			low_level_time = tmr0_cnt;
			tmr0_cnt = 0; // 清除电平时间计数值

			// 如果现在是高电平，之前可能接收了一位的信号（高电平+低电平）
			// 一位完整的信号记录完成，给对应的标志位置一
			recv_bit_flag = 1;
		}
		else
		{
			// 如果此时引脚是低电平，读取记录的电平持续时间

			high_level_time = tmr0_cnt; // 读出一次高电平持续时间
			tmr0_cnt = 0;				// 清除电平时间计数值
		}

		// 在中断服务函数中进行解码
		if (recv_bit_flag)
		{
			// 如果收到了一次完整的高电平（960us的"1"或320us的"0"）
			recv_bit_flag = 0; // 清除标志位

			if (low_level_time > 50)
			{
				// 如果低电平持续时间大于50 * 100us（5ms），准备下一次再读取有效信号
				rf_data = 0; // 清除接收的数据帧
				i = 0;		 // 清除用来记录接收的数据位数
			}

			// 判断高电平持续时间和低电平持续时间，是否符合范围
			else if (high_level_time > 1 && high_level_time <= 6 && low_level_time >= 5 && low_level_time <= 20)
			{
				// 如果是360us左右的高电平和880us左右的低电平，说明是"0"
				rf_data &= ~1;
				i++;
				if (i != 24)
				{
					rf_data <<= 1; // 用于存放接收24位数据的变量左移一位
				}
			}
			else if (high_level_time >= 5 && high_level_time <= 20 && low_level_time >= 1 && low_level_time < 10)
			{
				// 如果是960us左右的高电平和280us左右的低电平，说明是"1"
				rf_data |= 1;
				i++;
				if (i != 24)
				{
					rf_data <<= 1; // 用于存放接收24位数据的变量左移一位
				}
			}
			else
			{
				// 既不是"0"也不是"1"，说明本次的接收无效
				rf_data = 0;
				i = 0;

				// P12 = ~P12;
			}

			if (i >= 24)
			{
				// 如果接收了24位数据（0~23，最后一次i++直接变成24），说明一次接收完成
				recv_rf_flag = 1;
				i = 0;
			}
		}
	}

	P0_PND = p0_pnd; // 清P2中断标志位，写任何值都会清标志位

	// -------------------------------------------------
	__EnableIRQ(P0_IRQn); // 使能IO口中断
	// 退出中断设置IP，不可删除
	__IRQnIPnPop(P0_IRQn);
}
#endif // end ifdef DEVELOPMENT_BOARD


#ifdef CIRCUIT_BOARD

// P1中断服务函数
void P1_IRQHandler(void) interrupt P1_IRQn
{
	// Px_PND寄存器写任何值都会清标志位
	u8 p1_pnd = P1_PND;

	static volatile int i = 0; // 计数值，记录当前接收的数据位数

	// 进入中断设置IP，不可删除
	__IRQnIPnPush(P1_IRQn);
	__DisableIRQ(P1_IRQn); // 禁用IO口中断

	// ---------------- 用户函数处理 -------------------

	if (p1_pnd & GPIO_P13_IRQ_PNG(0x1)) // 清除标志位
	{
		// 在这里编写中断服务函数的代码

		// 根据引脚当前的电平来打开/关闭定时器TMR0，最后记录一个高电平信号的持续时间
		if (RFIN == 1)
		{
			// 如果此时引脚是高电平，读取记录的低电平的时间
			low_level_time = tmr0_cnt;
			tmr0_cnt = 0; // 清除电平时间计数值

			// 如果现在是高电平，之前可能接收了一位的信号（高电平+低电平）
			// 一位完整的信号记录完成，给对应的标志位置一
			recv_bit_flag = 1;
		}
		else
		{
			// 如果此时引脚是低电平，读取记录的电平持续时间

			high_level_time = tmr0_cnt; // 读出一次高电平持续时间
			tmr0_cnt = 0;				// 清除电平时间计数值
		}

		// 在中断服务函数中进行解码
		if (recv_bit_flag)
		{
			// 如果收到了一次完整的高电平（960us的"1"或320us的"0"）
			recv_bit_flag = 0; // 清除标志位

			if (low_level_time > 50)
			{
				// 如果低电平持续时间大于50 * 100us（5ms），准备下一次再读取有效信号
				rf_data = 0; // 清除接收的数据帧
				i = 0;		 // 清除用来记录接收的数据位数
			}

			// 判断高电平持续时间和低电平持续时间，是否符合范围
			else if (high_level_time > 1 && high_level_time <= 6 && low_level_time >= 5 && low_level_time <= 20)
			{
				// 如果是360us左右的高电平和880us左右的低电平，说明是"0"
				rf_data &= ~1;
				i++;
				if (i != 24)
				{
					rf_data <<= 1; // 用于存放接收24位数据的变量左移一位
				}
			}
			else if (high_level_time >= 5 && high_level_time <= 20 && low_level_time >= 1 && low_level_time < 10)
			{
				// 如果是960us左右的高电平和280us左右的低电平，说明是"1"
				rf_data |= 1;
				i++;
				if (i != 24)
				{
					rf_data <<= 1; // 用于存放接收24位数据的变量左移一位
				}
			}
			else
			{
				// 既不是"0"也不是"1"，说明本次的接收无效
				rf_data = 0;
				i = 0;

				// P12 = ~P12;
			}

			if (i >= 24)
			{
				// 如果接收了24位数据（0~23，最后一次i++直接变成24），说明一次接收完成
				recv_rf_flag = 1;
				i = 0;
			}
		}
	}

	P1_PND = p1_pnd; // 清P2中断标志位，写任何值都会清标志位

	// -------------------------------------------------
	__EnableIRQ(P1_IRQn); // 使能IO口中断
	// 退出中断设置IP，不可删除
	__IRQnIPnPop(P1_IRQn);
}

#endif // end of #ifdef CIRCUIT_BOARD

// 判断数组中出现次数最多的元素
// 如果出现次数一样多，只会返回次数一样的第一个元素
// 如果 参数 element 为NULL，则不会给参数element对应的空间赋值
// 如果 参数 index 为NULL，则不会给参数index对应的空间赋值
void appear_themost(u32 *arr, u32 arr_len, u32 *element, u32 *index)
{
	u32 themost = 0;		// 出现次数最多的元素的值
	u16 themost_index = -1; // 出现次数最多的元素所在数组中的下标
	u32 themost_cnt = 0;	// 出现次数最多的元素对应的个数

	u32 cur_element = 0;	 // 记录下当前的元素的值
	u32 cur_element_cnt = 0; // 记录当前元素的出现次数

	int i = 0;
	int j = 0;

	// 错误处理
	if (arr == NULL)
	{
		return;
	}

	for (i = 0; i < arr_len; i++)
	{
		cur_element = arr[i]; // 记录下当前的元素的值，接下来的循环中记录当前元素出现的个数
		cur_element_cnt = 0;
		for (j = 0; j < arr_len; j++)
		{
			if (cur_element == arr[j])
			{
				// 如果数组中出现过一次该元素，则让计数值加一
				cur_element_cnt++;
			}
		}

		// 到了这里，便得到了一个元素在这个数组中出现的次数
		// 跟已知出现次数最多的元素相比较，如果元素的数值不一样且当前元素的出现次数更多，
		// 则让当前元素作为已知的出现次数最多的元素
		if (cur_element != themost && cur_element_cnt > themost_cnt)
		{
			themost = cur_element;
			themost_cnt = cur_element_cnt;
			themost_index = j - 1; // 临时记录它对应的数组下标
		}
	}

	// 最后，通过形参返回出现次数最多的元素的值和它所对应的数组下标
	if (NULL != element)
	{
		*element = themost;
	}

	if (index != NULL)
	{
		*index = themost_index;
	}

	// send_keyval(themost >> 16); // 测试用（测试通过）
	// send_keyval(themost & 0xFFFF);
}

// 接收RF信号到缓冲区中，接收n次信号（n是数组支持的元素个数，可以根据需要修改）
void rf_recv_databuf(void)
{
	// u32 appear_themost = 0; // 测试用的变量
	u32 index = 0;
	// int i = 0; // 循环计数值 （测试用）

	if (recv_rf_flag)
	{
		// 如果成功接收了24位的数据，将其保存到缓冲区中
		recv_rf_flag = 0;
#ifdef DEVELOPMENT_BOARD
		__DisableIRQ(P0_IRQn); // 禁用IO口中断，这时不能再从RF接收引脚接收新的数据
#endif // end ifdef DEVELOPMENT_BOARD
#ifdef CIRCUIT_BOARD
		__DisableIRQ(P1_IRQn); // 禁用IO口中断，这时不能再从RF接收引脚接收新的数据
#endif // end ifdef CIRCUIT_BOARD
		rf_data_buf[rf_data_buf_index++] = rf_data; // 存放一次数据

// send_keyval(rf_data & 0xFF);
#ifdef DEVELOPMENT_BOARD
		__EnableIRQ(P0_IRQn); // 使能IO口中断
#endif // end ifdef DEVELOPMENT_BOARD
#ifdef CIRCUIT_BOARD
		__EnableIRQ(P1_IRQn); // 使能IO口中断
#endif // end ifdef CIRCUIT_BOARD
	}

	if (rf_data_buf_index == sizeof(rf_data_buf) / sizeof(rf_data_buf[0]))
	{
		// 如果接收了n次数据
		rf_data_buf_index = 0; // 数组（缓冲区）下标清零
		rf_data_buf_overflow = 1; // 数组（缓冲区）溢出标志置一

#if 0
		// 将收到的信号都显示出来
		for (i = 0; i < sizeof(rf_data_buf) / sizeof(rf_data_buf[0]); i++)
		{
			// send_keyval((unsigned short)((rf_data_buf[i] >> 16) & 0xFF));
			// send_keyval((unsigned short)(rf_data_buf[i] & 0xFF));

			send_keyval((unsigned short)(rf_data_buf[i])); // 这个可以在逻辑分析仪上看出对应的键值（后16位数据）
		}
#endif

#if 0
		// 找出出现次数最多的元素
		appear_themost(rf_data_buf, sizeof(rf_data_buf) / sizeof(rf_data_buf[0]), &appear_themost, &index);

		// send_keyval(appear_themost); // 这个可以在逻辑分析仪上看出对应的键值（后16位数据）

		// if ((((unsigned short)appear_themost) & (~0x0F)) == (u32)0xF0B480) // 测试通过的程序
		// {
		// 	if ((appear_themost & 0x0F) == KEY_RF315_A) // == 优先级要大于 &
		// 	{
		// 		P12 = ~P12;
		// 		send_keyval(KEY_RF315_A);
		// 	}
		// }
#endif
	}
}

// 检测是否接收到了RF信号，并根据相应的命令进行控制（因此函数内不只是接收，还有相应的处理）
// 测试用
void rf_recv(void)
{
	int i = 0;
	unsigned char isMatch = 0; // 标志位，器件地址是否与flash中的符合

	// 这一项测试后发现功能是正常的
	// send_keyval(addr_info.old_index);

	// 测试 在学习后是不是真的能从flash中读取24bit的地址
	// send_keyval((unsigned short)(addr_info.addr[0] >> 16));
	// send_keyval((unsigned short)(addr_info.addr[0] & 0xFF));

	// send_keyval((unsigned short)(addr_info.addr[1] >> 16));
	// send_keyval((unsigned short)(addr_info.addr[1] & 0xFFFF));

	if (recv_rf_flag)
	{
		// 如果成功接收了24位的数据
		recv_rf_flag = 0;

		__DisableIRQ(P0_IRQn); // 禁用IO口中断（一定要关闭中断，否则会被新的RF信号打断，导致后面在逻辑分析仪看到的波形与预想的不一致）
		send_keyval(rf_data);

		// send_keyval(rf_data >> 16);
		// send_keyval(rf_data);

#if 0 // 测试通过的代码
	  // p11_send_data_8bit_msb(rf_data & 0x0F);

		// 发送前后都加个1ms的信号，方便观察
		P11 = 0;
		delay_ms(1);
		P11 = 1;
		delay_ms(1);
		// p11_send_data_8bit_msb(rf_data); // 测试用
		p11_send_data_16bit_msb(rf_data); // 测试用
		P11 = 1;
		delay_ms(1);
		P11 = 0;
		delay_ms(1);

		rf_data = 0; // 发送完成后，清除接收到的数据
					 // delay_ms(200); // 这里加延时主要是防止不停地触发中断，导致后面接收到的数据不准确
#endif

#if 0 // 测试通过的代码

		// 先判断地址码对不对
		if ((rf_data & (~0x0F)) == (u32)0xF0B480) // 这里的比较必须要加强制转换
		{
			switch (rf_data & 0x0F) // 判断接收到的低四位，低四位是键值
			{
			case KEY_RF315_A: // 如果是遥控器按键A
				send_keyval(KEY_RF315_A);
				break;

			case KEY_RF315_B: // 如果是遥控器按键B
				send_keyval(KEY_RF315_B);
				break;

			case KEY_RF315_C: // 如果是遥控器按键C
				send_keyval(KEY_RF315_C);
				break;

			case KEY_RF315_D: // 如果是遥控器按键D
				send_keyval(KEY_RF315_D);
				break;

			case KEY_RF315_E: // 如果是遥控器按键E
				send_keyval(KEY_RF315_E);
				break;

			case KEY_RF315_F: // 如果是遥控器按键F
				send_keyval(KEY_RF315_F);
				break;
			}
		}

#endif

#if 0
		// 先判断地址码对不对
		for (i = 0; i < ADDR_MAX_NUM; i++)
		{
			if (addr_info.addr[i] == (rf_data >> 4))
			{
				isMatch = 1;
				break;
			}
		}

		if (isMatch) 
		{
			// 如果是单片机flash中保存的地址，则响应这个信号
			isMatch = 0;
			send_keyval(rf_data >> 16);
			send_keyval(rf_data);
		}

#endif
		__EnableIRQ(P0_IRQn); // 使能IO口中断
	}
}
