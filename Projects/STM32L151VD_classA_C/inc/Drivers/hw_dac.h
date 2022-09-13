/**============================================================================
* @file      hw_dac.h
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
#ifndef __HW_DAC_H__
#define __HW_DAC_H__
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
// #include "main.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern DAC_HandleTypeDef hdac;
extern DMA_HandleTypeDef hdma_dac_ch2;
   
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
/** 
* @brief  Init the DAC.
* @param  None
* @return None
*/   
void HW_DAC_FillSinusTable(void);
void HW_DAC_Init(void (* XferCpltCallback)(void) );
void HW_DAC_DeInit(void);
void HW_DAC_Start_1kHz_1Vpp(void);
void HW_DAC_Stop(void);

#ifdef __cplusplus
}
#endif
#endif /*__HW_DAC_H__ */

/*****END OF FILE****/
