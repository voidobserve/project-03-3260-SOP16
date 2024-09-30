/**
  ******************************************************************************
  * @file     User/tk_user.h
  * @author   TAIXIN-IC Application Team
  * @version  V1.0.0
  * @date     05-20-2022
  * @brief    Main program body.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2022 TAIXIN-IC</center></h2>
  *
  *
  *
  ******************************************************************************
  */ 
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TK_USER_H
#define __TK_USER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "my_config.h"
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
 
/** @defgroup XXX_LL_Register_Constants XXX LL Register Constants
  * @ingroup  XXX_LL_Driver
  * @brief    XXX LL register constant table definition
  *
  *
  
@verbatim   
  ===============================================================================
                                Register Constants
  ===============================================================================  
  
    Register Constants mainly encapsulates each bit in each group in the XXX 
    register. In the process of configuration, the macro definition can be directly 
    called to configure the XXX register, mainly for convenience. Understand the 
    configuration of the XXX.
    
@endverbatim
  *
  * @{
  */
#if TK_DEBUG_EN 
void debug_gpio_config(void);
#endif

void user_init(void);
void user_handle(void);
/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif // __TK_USER__

/**
  * @}
  */

/**
  * @}
  */

/*************************** (C) COPYRIGHT 2021 TAIXIN-IC ***** END OF FILE *****/


