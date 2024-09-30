// RF-315MHz��RF-433MHz����Դ����
/*
	˼·��ͨ��IO�ж�+��ʱ������������ʱ��ʼ��¼�ߵ�ƽ����ʱ�䣬���½���ʱ��ʼ��¼�͵�ƽ����ʱ��

	1. �иߵ�ƽ����ʱ��˵��ǰ���Ѿ���¼��һλ�źţ����ǵ�һλ�źŵ���
	2. �жϸߵ�ƽ�͵͵�ƽ����ʱ��ʱ����ΧҪ����Щ���������
	3. Ҫע��һ֡24λ�����ݵ���ʱ��ǰ�����нϳ��ĵ͵�ƽ�źţ�����ʱ�������źţ�ǰ��Ҳ��10ms���ϵĵ͵�ƽ�źš�
	������һ��������ֲ�ͬ��֡���������ݴ�λ

*/
#include "rf_recv.h"
#include "my_gpio.h"
#include "send_key.h"
#include "flash.h" // ��Ƭ���ϵ�flash��ز���

#include <stdlib.h> // NULL�Ķ���

// ��ʱ��TMR0�ļ�ʱ���ڣ�Ҳ���жϴ������ڣ�ÿ����ô���һ���жϣ�
// ��ʱ���ڲ��ܴ���65535����ΪTMR0��ż�ʱ���ڵļĴ���ֻ��16λ
// �����ö�ʱ��TMR0ÿ100us����һ���жϣ�ʵ������ʱ����120us����ʱ����80us���������������ģ�
#define TMR0_CNT_TIME ((152)) // 152 * 0.65625us == 99.75us��Լ����100us �����Ǽ���ó��ģ�ʵ���ϻ�����������Ҫ����ʵ��������ԡ��޸ģ�

static volatile int recv_bit_flag = 0; // �Ƿ���յ�������һλ�źŵı�־λ
static volatile u32 tmr0_cnt = 0;	   // ��ʱ���жϷ������У�ʹ�õ��ļ���ֵ

static volatile u32 high_level_time = 0; // �����������涨ʱ���ļ���ֵ����ʱʱ�䣩����ֹ��ʱ���ظ���ʱ�����tmr0_cnt�����ݸ���
static volatile u32 low_level_time = 0;	 // ��ŵ͵�ƽ������ʱ��

volatile u32 rf_data = 0; // ���ڴ�Ž��յ���24λ����

volatile int recv_rf_flag = 0; // �Ƿ���յ�������rf�źŵı�־λ

// ���ڴ�Ž��յ���24λ���ݣ�����n�Σ����ж���������û��������ַ�ظ��ء������ǳ��ִ�������
// ����У��򱣴�������������ַ
// ���û�У������
// ����������һ���ź����������ԼΪ50ms���������20�Σ�һ��1000ms��Ҳ���ǳ���1s���ҾͿ���ѧϰ�������Ӧ�ĵ�ַ
volatile u32 rf_data_buf[20] = {0};
static volatile unsigned char rf_data_buf_index = 0; // �������ݻ�����������
unsigned char rf_data_buf_overflow = 0;				 // ���ջ���������ı�־λ

// RFIN���ų�ʼ����RF�������ų�ʼ����
// ����Ҳʹ�õ��˶�ʱ��TMR0�����ó�ÿ100us���Ҳ���һ���жϣ����20us���������ú���������󣬶�ʱ��TMR0�ǹرյ�
void rfin_init(void)
{

#ifdef DEVELOPMENT_BOARD

	// ����P0_2Ϊ����ģʽ
	P0_MD0 &= ~GPIO_P02_MODE_SEL(0x01);
	// ����P0_2Ϊ��������ΪRF����ģ�������ڿ���ʱĬ��Ϊ�͵�ƽ
	P0_PD |= GPIO_P02_PULL_PD(0x01);

	__SetIRQnIP(P0_IRQn, P0_IQn_CFG);	// �����ж����ȼ�
	__EnableIRQ(P0_IRQn);				// ʹ���ж�
	IE_EA = 1;							// ʹ���ܿ���
	P0_IMK |= GPIO_P02_IRQ_MASK(0x01);	// ��P02���ж�
	P0_TRG0 &= ~GPIO_P02_TRG_SEL(0x03); // ����P02Ϊ˫���ش���

#endif // end ifdef DEVELOPMENT_BOARD

#ifdef CIRCUIT_BOARD

	// ����P1_3Ϊ����ģʽ
	P1_MD0 &= ~GPIO_P13_MODE_SEL(0x01);
	// ����P1_3Ϊ��������ΪRF����ģ�������ڿ���ʱĬ��Ϊ�͵�ƽ
	P1_PD |= GPIO_P13_PULL_PD(0x01);

	__SetIRQnIP(P1_IRQn, P1_IQn_CFG);	// �����ж����ȼ�
	__EnableIRQ(P1_IRQn);				// ʹ���ж�
	IE_EA = 1;							// ʹ���ܿ���
	P1_IMK |= GPIO_P13_IRQ_MASK(0x01);	// ��P13���ж�
	P1_TRG0 &= ~GPIO_P13_TRG_SEL(0x03); // ����P13Ϊ˫���ش���

#endif // end ifdef CIRCUIT_BOARD

	// ============================================================ //
	// ���ö�ʱ����������¼RF���յ��ĸߵ�ƽ����ʱ��
	__SetIRQnIP(TMR0_IRQn, TMR0_IQn_CFG); // �����ж����ȼ���TMR0��

	TMR0_CONL &= ~TMR_PRESCALE_SEL(0x03); // ���TMR0��Ԥ��Ƶ���üĴ���

	// ����TMR0��Ԥ��Ƶ��Ϊ32��Ƶ����21MHz / 32 = 0.67MHz��Լ0.67us����һ�Σ�
	// ��ʵ�ʲ��Ժͼ���ó����ϵͳʱ����21MHz�ģ����ǻ�����Щ��
	TMR0_CONL |= TMR_PRESCALE_SEL(0x05);
	TMR0_CONL &= ~TMR_MODE_SEL(0x03); // ���TMR0��ģʽ���üĴ���
	TMR0_CONL |= TMR_MODE_SEL(0x01);  // ����TMR0��ģʽΪ������ģʽ������ϵͳʱ�ӵ�������м���

	TMR0_CONH &= ~TMR_PRD_PND(0x01); // ���TMR0�ļ�����־λ����ʾδ��ɼ���
	TMR0_CONH |= TMR_PRD_IRQ_EN(1);	 // ʹ��TMR0�ļ����ж�

	// ����TMR0�ļ�������
	TMR0_PRL = (unsigned char)(TMR0_CNT_TIME % 255);
	TMR0_PRH = (unsigned char)(TMR0_CNT_TIME / 255);

	// ���TMR0�ļ���ֵ
	TMR0_CNTL = 0;
	TMR0_CNTH = 0;

	TMR0_CONL &= ~(TMR_SOURCE_SEL(0x07)); // ���TMR0��ʱ��Դ���üĴ���
	TMR0_CONL |= TMR_SOURCE_SEL(0x05);	  // ����TMR0��ʱ��Դ�������κ�ʱ��
										  // __EnableIRQ(TMR0_IRQn);			   // ʹ���ж�

	__DisableIRQ(TMR0_IRQn); // �����ж�
}

// TMR0�жϷ�����
void TIMR0_IRQHandler(void) interrupt TMR0_IRQn
{
	// �����ж�����IP������ɾ��
	__IRQnIPnPush(TMR0_IRQn);

	// ---------------- �û��������� -------------------

	// �����ж�
	if (TMR0_CONH & TMR_PRD_PND(0x1))
	{
		TMR0_CONH |= TMR_PRD_PND(0x1); // ���pending

		tmr0_cnt++; // ÿ80us��120us���һ��

		// P12 = ~P12;
	}

	// �˳��ж�����IP������ɾ��
	__IRQnIPnPop(TMR0_IRQn);
}

// ������ʱ��TMR0����ʼ��ʱ
void tmr0_enable(void)
{
	// ���¸�TMR0����ʱ��
	TMR0_CONL &= ~(TMR_SOURCE_SEL(0x07)); // �����ʱ����ʱ��Դ���üĴ���
	TMR0_CONL |= TMR_SOURCE_SEL(0x06);	  // ���ö�ʱ����ʱ��Դ��ʹ��ϵͳʱ�ӣ�Լ21MHz��

	__EnableIRQ(TMR0_IRQn); // ʹ���ж�
}

// �رն�ʱ��0����ռ���ֵ
void tmr0_disable(void)
{
	// ������ʱ���ṩʱ�ӣ�����ֹͣ����
	TMR0_CONL &= ~(TMR_SOURCE_SEL(0x07)); // �����ʱ����ʱ��Դ���üĴ���
	TMR0_CONL |= TMR_SOURCE_SEL(0x05);	  // ���ö�ʱ����ʱ��Դ�������κ�ʱ��

	// �����ʱ���ļ���ֵ
	TMR0_CNTL = 0;
	TMR0_CNTH = 0;

	__DisableIRQ(TMR0_IRQn); // �ر��жϣ���ʹ���жϣ�
}


#ifdef DEVELOPMENT_BOARD
// P0�жϷ�����
void P0_IRQHandler(void) interrupt P0_IRQn
{
	// Px_PND�Ĵ���д�κ�ֵ�������־λ
	u8 p0_pnd = P0_PND;

	static volatile int i = 0; // ����ֵ����¼��ǰ���յ�����λ��

	// �����ж�����IP������ɾ��
	__IRQnIPnPush(P0_IRQn);
	__DisableIRQ(P0_IRQn); // ����IO���ж�

	// ---------------- �û��������� -------------------

	if (p0_pnd & GPIO_P02_IRQ_PNG(0x1)) // �����־λ
	{
		// �������д�жϷ������Ĵ���

		// �������ŵ�ǰ�ĵ�ƽ����/�رն�ʱ��TMR0������¼һ���ߵ�ƽ�źŵĳ���ʱ��
		if (RFIN == 1)
		{
			// �����ʱ�����Ǹߵ�ƽ����ȡ��¼�ĵ͵�ƽ��ʱ��
			low_level_time = tmr0_cnt;
			tmr0_cnt = 0; // �����ƽʱ�����ֵ

			// ��������Ǹߵ�ƽ��֮ǰ���ܽ�����һλ���źţ��ߵ�ƽ+�͵�ƽ��
			// һλ�������źż�¼��ɣ�����Ӧ�ı�־λ��һ
			recv_bit_flag = 1;
		}
		else
		{
			// �����ʱ�����ǵ͵�ƽ����ȡ��¼�ĵ�ƽ����ʱ��

			high_level_time = tmr0_cnt; // ����һ�θߵ�ƽ����ʱ��
			tmr0_cnt = 0;				// �����ƽʱ�����ֵ
		}

		// ���жϷ������н��н���
		if (recv_bit_flag)
		{
			// ����յ���һ�������ĸߵ�ƽ��960us��"1"��320us��"0"��
			recv_bit_flag = 0; // �����־λ

			if (low_level_time > 50)
			{
				// ����͵�ƽ����ʱ�����50 * 100us��5ms����׼����һ���ٶ�ȡ��Ч�ź�
				rf_data = 0; // ������յ�����֡
				i = 0;		 // ���������¼���յ�����λ��
			}

			// �жϸߵ�ƽ����ʱ��͵͵�ƽ����ʱ�䣬�Ƿ���Ϸ�Χ
			else if (high_level_time > 1 && high_level_time <= 6 && low_level_time >= 5 && low_level_time <= 20)
			{
				// �����360us���ҵĸߵ�ƽ��880us���ҵĵ͵�ƽ��˵����"0"
				rf_data &= ~1;
				i++;
				if (i != 24)
				{
					rf_data <<= 1; // ���ڴ�Ž���24λ���ݵı�������һλ
				}
			}
			else if (high_level_time >= 5 && high_level_time <= 20 && low_level_time >= 1 && low_level_time < 10)
			{
				// �����960us���ҵĸߵ�ƽ��280us���ҵĵ͵�ƽ��˵����"1"
				rf_data |= 1;
				i++;
				if (i != 24)
				{
					rf_data <<= 1; // ���ڴ�Ž���24λ���ݵı�������һλ
				}
			}
			else
			{
				// �Ȳ���"0"Ҳ����"1"��˵�����εĽ�����Ч
				rf_data = 0;
				i = 0;

				// P12 = ~P12;
			}

			if (i >= 24)
			{
				// ���������24λ���ݣ�0~23�����һ��i++ֱ�ӱ��24����˵��һ�ν������
				recv_rf_flag = 1;
				i = 0;
			}
		}
	}

	P0_PND = p0_pnd; // ��P2�жϱ�־λ��д�κ�ֵ�������־λ

	// -------------------------------------------------
	__EnableIRQ(P0_IRQn); // ʹ��IO���ж�
	// �˳��ж�����IP������ɾ��
	__IRQnIPnPop(P0_IRQn);
}
#endif // end ifdef DEVELOPMENT_BOARD


#ifdef CIRCUIT_BOARD

// P1�жϷ�����
void P1_IRQHandler(void) interrupt P1_IRQn
{
	// Px_PND�Ĵ���д�κ�ֵ�������־λ
	u8 p1_pnd = P1_PND;

	static volatile int i = 0; // ����ֵ����¼��ǰ���յ�����λ��

	// �����ж�����IP������ɾ��
	__IRQnIPnPush(P1_IRQn);
	__DisableIRQ(P1_IRQn); // ����IO���ж�

	// ---------------- �û��������� -------------------

	if (p1_pnd & GPIO_P13_IRQ_PNG(0x1)) // �����־λ
	{
		// �������д�жϷ������Ĵ���

		// �������ŵ�ǰ�ĵ�ƽ����/�رն�ʱ��TMR0������¼һ���ߵ�ƽ�źŵĳ���ʱ��
		if (RFIN == 1)
		{
			// �����ʱ�����Ǹߵ�ƽ����ȡ��¼�ĵ͵�ƽ��ʱ��
			low_level_time = tmr0_cnt;
			tmr0_cnt = 0; // �����ƽʱ�����ֵ

			// ��������Ǹߵ�ƽ��֮ǰ���ܽ�����һλ���źţ��ߵ�ƽ+�͵�ƽ��
			// һλ�������źż�¼��ɣ�����Ӧ�ı�־λ��һ
			recv_bit_flag = 1;
		}
		else
		{
			// �����ʱ�����ǵ͵�ƽ����ȡ��¼�ĵ�ƽ����ʱ��

			high_level_time = tmr0_cnt; // ����һ�θߵ�ƽ����ʱ��
			tmr0_cnt = 0;				// �����ƽʱ�����ֵ
		}

		// ���жϷ������н��н���
		if (recv_bit_flag)
		{
			// ����յ���һ�������ĸߵ�ƽ��960us��"1"��320us��"0"��
			recv_bit_flag = 0; // �����־λ

			if (low_level_time > 50)
			{
				// ����͵�ƽ����ʱ�����50 * 100us��5ms����׼����һ���ٶ�ȡ��Ч�ź�
				rf_data = 0; // ������յ�����֡
				i = 0;		 // ���������¼���յ�����λ��
			}

			// �жϸߵ�ƽ����ʱ��͵͵�ƽ����ʱ�䣬�Ƿ���Ϸ�Χ
			else if (high_level_time > 1 && high_level_time <= 6 && low_level_time >= 5 && low_level_time <= 20)
			{
				// �����360us���ҵĸߵ�ƽ��880us���ҵĵ͵�ƽ��˵����"0"
				rf_data &= ~1;
				i++;
				if (i != 24)
				{
					rf_data <<= 1; // ���ڴ�Ž���24λ���ݵı�������һλ
				}
			}
			else if (high_level_time >= 5 && high_level_time <= 20 && low_level_time >= 1 && low_level_time < 10)
			{
				// �����960us���ҵĸߵ�ƽ��280us���ҵĵ͵�ƽ��˵����"1"
				rf_data |= 1;
				i++;
				if (i != 24)
				{
					rf_data <<= 1; // ���ڴ�Ž���24λ���ݵı�������һλ
				}
			}
			else
			{
				// �Ȳ���"0"Ҳ����"1"��˵�����εĽ�����Ч
				rf_data = 0;
				i = 0;

				// P12 = ~P12;
			}

			if (i >= 24)
			{
				// ���������24λ���ݣ�0~23�����һ��i++ֱ�ӱ��24����˵��һ�ν������
				recv_rf_flag = 1;
				i = 0;
			}
		}
	}

	P1_PND = p1_pnd; // ��P2�жϱ�־λ��д�κ�ֵ�������־λ

	// -------------------------------------------------
	__EnableIRQ(P1_IRQn); // ʹ��IO���ж�
	// �˳��ж�����IP������ɾ��
	__IRQnIPnPop(P1_IRQn);
}

#endif // end of #ifdef CIRCUIT_BOARD

// �ж������г��ִ�������Ԫ��
// ������ִ���һ���ֻ࣬�᷵�ش���һ���ĵ�һ��Ԫ��
// ��� ���� element ΪNULL���򲻻������element��Ӧ�Ŀռ丳ֵ
// ��� ���� index ΪNULL���򲻻������index��Ӧ�Ŀռ丳ֵ
void appear_themost(u32 *arr, u32 arr_len, u32 *element, u32 *index)
{
	u32 themost = 0;		// ���ִ�������Ԫ�ص�ֵ
	u16 themost_index = -1; // ���ִ�������Ԫ�����������е��±�
	u32 themost_cnt = 0;	// ���ִ�������Ԫ�ض�Ӧ�ĸ���

	u32 cur_element = 0;	 // ��¼�µ�ǰ��Ԫ�ص�ֵ
	u32 cur_element_cnt = 0; // ��¼��ǰԪ�صĳ��ִ���

	int i = 0;
	int j = 0;

	// ������
	if (arr == NULL)
	{
		return;
	}

	for (i = 0; i < arr_len; i++)
	{
		cur_element = arr[i]; // ��¼�µ�ǰ��Ԫ�ص�ֵ����������ѭ���м�¼��ǰԪ�س��ֵĸ���
		cur_element_cnt = 0;
		for (j = 0; j < arr_len; j++)
		{
			if (cur_element == arr[j])
			{
				// ��������г��ֹ�һ�θ�Ԫ�أ����ü���ֵ��һ
				cur_element_cnt++;
			}
		}

		// ���������õ���һ��Ԫ������������г��ֵĴ���
		// ����֪���ִ�������Ԫ����Ƚϣ����Ԫ�ص���ֵ��һ���ҵ�ǰԪ�صĳ��ִ������࣬
		// ���õ�ǰԪ����Ϊ��֪�ĳ��ִ�������Ԫ��
		if (cur_element != themost && cur_element_cnt > themost_cnt)
		{
			themost = cur_element;
			themost_cnt = cur_element_cnt;
			themost_index = j - 1; // ��ʱ��¼����Ӧ�������±�
		}
	}

	// ���ͨ���βη��س��ִ�������Ԫ�ص�ֵ��������Ӧ�������±�
	if (NULL != element)
	{
		*element = themost;
	}

	if (index != NULL)
	{
		*index = themost_index;
	}

	// send_keyval(themost >> 16); // �����ã�����ͨ����
	// send_keyval(themost & 0xFFFF);
}

// ����RF�źŵ��������У�����n���źţ�n������֧�ֵ�Ԫ�ظ��������Ը�����Ҫ�޸ģ�
void rf_recv_databuf(void)
{
	// u32 appear_themost = 0; // �����õı���
	u32 index = 0;
	// int i = 0; // ѭ������ֵ �������ã�

	if (recv_rf_flag)
	{
		// ����ɹ�������24λ�����ݣ����䱣�浽��������
		recv_rf_flag = 0;
#ifdef DEVELOPMENT_BOARD
		__DisableIRQ(P0_IRQn); // ����IO���жϣ���ʱ�����ٴ�RF�������Ž����µ�����
#endif // end ifdef DEVELOPMENT_BOARD
#ifdef CIRCUIT_BOARD
		__DisableIRQ(P1_IRQn); // ����IO���жϣ���ʱ�����ٴ�RF�������Ž����µ�����
#endif // end ifdef CIRCUIT_BOARD
		rf_data_buf[rf_data_buf_index++] = rf_data; // ���һ������

// send_keyval(rf_data & 0xFF);
#ifdef DEVELOPMENT_BOARD
		__EnableIRQ(P0_IRQn); // ʹ��IO���ж�
#endif // end ifdef DEVELOPMENT_BOARD
#ifdef CIRCUIT_BOARD
		__EnableIRQ(P1_IRQn); // ʹ��IO���ж�
#endif // end ifdef CIRCUIT_BOARD
	}

	if (rf_data_buf_index == sizeof(rf_data_buf) / sizeof(rf_data_buf[0]))
	{
		// ���������n������
		rf_data_buf_index = 0; // ���飨���������±�����
		rf_data_buf_overflow = 1; // ���飨�������������־��һ

#if 0
		// ���յ����źŶ���ʾ����
		for (i = 0; i < sizeof(rf_data_buf) / sizeof(rf_data_buf[0]); i++)
		{
			// send_keyval((unsigned short)((rf_data_buf[i] >> 16) & 0xFF));
			// send_keyval((unsigned short)(rf_data_buf[i] & 0xFF));

			send_keyval((unsigned short)(rf_data_buf[i])); // ����������߼��������Ͽ�����Ӧ�ļ�ֵ����16λ���ݣ�
		}
#endif

#if 0
		// �ҳ����ִ�������Ԫ��
		appear_themost(rf_data_buf, sizeof(rf_data_buf) / sizeof(rf_data_buf[0]), &appear_themost, &index);

		// send_keyval(appear_themost); // ����������߼��������Ͽ�����Ӧ�ļ�ֵ����16λ���ݣ�

		// if ((((unsigned short)appear_themost) & (~0x0F)) == (u32)0xF0B480) // ����ͨ���ĳ���
		// {
		// 	if ((appear_themost & 0x0F) == KEY_RF315_A) // == ���ȼ�Ҫ���� &
		// 	{
		// 		P12 = ~P12;
		// 		send_keyval(KEY_RF315_A);
		// 	}
		// }
#endif
	}
}

// ����Ƿ���յ���RF�źţ���������Ӧ��������п��ƣ���˺����ڲ�ֻ�ǽ��գ�������Ӧ�Ĵ���
// ������
void rf_recv(void)
{
	int i = 0;
	unsigned char isMatch = 0; // ��־λ��������ַ�Ƿ���flash�еķ���

	// ��һ����Ժ��ֹ�����������
	// send_keyval(addr_info.old_index);

	// ���� ��ѧϰ���ǲ�������ܴ�flash�ж�ȡ24bit�ĵ�ַ
	// send_keyval((unsigned short)(addr_info.addr[0] >> 16));
	// send_keyval((unsigned short)(addr_info.addr[0] & 0xFF));

	// send_keyval((unsigned short)(addr_info.addr[1] >> 16));
	// send_keyval((unsigned short)(addr_info.addr[1] & 0xFFFF));

	if (recv_rf_flag)
	{
		// ����ɹ�������24λ������
		recv_rf_flag = 0;

		__DisableIRQ(P0_IRQn); // ����IO���жϣ�һ��Ҫ�ر��жϣ�����ᱻ�µ�RF�źŴ�ϣ����º������߼������ǿ����Ĳ�����Ԥ��Ĳ�һ�£�
		send_keyval(rf_data);

		// send_keyval(rf_data >> 16);
		// send_keyval(rf_data);

#if 0 // ����ͨ���Ĵ���
	  // p11_send_data_8bit_msb(rf_data & 0x0F);

		// ����ǰ�󶼼Ӹ�1ms���źţ�����۲�
		P11 = 0;
		delay_ms(1);
		P11 = 1;
		delay_ms(1);
		// p11_send_data_8bit_msb(rf_data); // ������
		p11_send_data_16bit_msb(rf_data); // ������
		P11 = 1;
		delay_ms(1);
		P11 = 0;
		delay_ms(1);

		rf_data = 0; // ������ɺ�������յ�������
					 // delay_ms(200); // �������ʱ��Ҫ�Ƿ�ֹ��ͣ�ش����жϣ����º�����յ������ݲ�׼ȷ
#endif

#if 0 // ����ͨ���Ĵ���

		// ���жϵ�ַ��Բ���
		if ((rf_data & (~0x0F)) == (u32)0xF0B480) // ����ıȽϱ���Ҫ��ǿ��ת��
		{
			switch (rf_data & 0x0F) // �жϽ��յ��ĵ���λ������λ�Ǽ�ֵ
			{
			case KEY_RF315_A: // �����ң��������A
				send_keyval(KEY_RF315_A);
				break;

			case KEY_RF315_B: // �����ң��������B
				send_keyval(KEY_RF315_B);
				break;

			case KEY_RF315_C: // �����ң��������C
				send_keyval(KEY_RF315_C);
				break;

			case KEY_RF315_D: // �����ң��������D
				send_keyval(KEY_RF315_D);
				break;

			case KEY_RF315_E: // �����ң��������E
				send_keyval(KEY_RF315_E);
				break;

			case KEY_RF315_F: // �����ң��������F
				send_keyval(KEY_RF315_F);
				break;
			}
		}

#endif

#if 0
		// ���жϵ�ַ��Բ���
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
			// ����ǵ�Ƭ��flash�б���ĵ�ַ������Ӧ����ź�
			isMatch = 0;
			send_keyval(rf_data >> 16);
			send_keyval(rf_data);
		}

#endif
		__EnableIRQ(P0_IRQn); // ʹ��IO���ж�
	}
}
