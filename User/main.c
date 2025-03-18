/**
 ******************************************************************************
 * @file    main.c
 * @author  HUGE-IC Application Team
 * @version V1.0.0
 * @date    05-11-2022
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 HUGE-IC</center></h2>
 *
 * ��Ȩ˵����������
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "include.h"

/** @addtogroup Template_Project
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
void main(void)
{
  // ���Ź�Ĭ�ϴ�, ��λʱ��2s
  system_init();
  WDT_KEY = 0x55;
  IO_MAP &= ~0x01;
  WDT_KEY = 0xBB;

  // touchkey handle funtion
  tk_handle(); // �ڲ��Ǹ���ѭ��

  // while (1)
  //   ;
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2022 HUGE-IC ***** END OF FILE *****/
