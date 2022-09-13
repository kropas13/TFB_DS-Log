/**============================================================================
* @file      hw_adc.h
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of hw_adc.c
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 			
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_ADC_H__
#define __HW_ADC_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
   
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern ADC_HandleTypeDef hadc;
extern DMA_HandleTypeDef hdma_adc;
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
/** 
* @brief  Init the ADC.
* @param  None
* @return None
*/
void HW_ADC1_SingleConv_Init(  void );

/** 
* @brief  Deninitializes the ADC.
* @param  None
* @return None
*/
void HW_ADC1_DeInit( void );

/*!
* @brief  Read an analog voltage value
* @param  Channel [IN]  Channel to read
* @return value   [OUT] Analogue value of channel
 */
uint16_t HW_ADC1_Read( uint32_t Channel, uint16_t AverageCnt);

void HW_ADC_MultipleConv_Init(  void );
void HW_ADC_MultipleConv_DeInit(  void );

#ifdef __cplusplus
}
#endif

#endif /* __HW_ADC_H__ */

/*****END OF FILE****/
