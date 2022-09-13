/**============================================================================
* @file      hw_msp.h
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of hw_msp.c
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 			
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_MSP_H__
#define __HW_MSP_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
/*!
 * \brief Configures the sytem Clock at start-up
 *
 * \param none
 * \retval none
 */
void SystemClock_Config( void );
   
#ifdef USE_HSE_PINS_AI_IN_STOP
void SystemClockHSE_Config(void);  
void SystemClockHSI_Config(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __HW_MSP_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
