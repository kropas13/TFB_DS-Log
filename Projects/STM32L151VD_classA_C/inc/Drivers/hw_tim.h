/**============================================================================
* @file      hw_tim.h
* @date      2018-06-21
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of hw_dac.c
*            
* VERSION:   
* 
* V0.01      2018-06-21-Kd      Create File 			
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_TIM_H__
#define __HW_TIM_H__
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
   
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
   
   
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
/** 
* @brief  Init the TIM4.
* @param  None
* @return None
*/     
void HW_TIM4_Init(void);
void HW_TIM4_DeInit(void);
   
/** 
* @brief  Init the TIM6.
* @param  None
* @return None
*/  
void HW_TIM6_Init(uint32_t u32Freq_Hz);
void HW_TIM6_DeInit(void);
   
#ifdef __cplusplus
}
#endif
#endif /*__ __HW_TIM_H__ */

/*****END OF FILE****/
