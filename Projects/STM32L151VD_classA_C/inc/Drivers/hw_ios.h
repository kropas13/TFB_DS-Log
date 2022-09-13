/**============================================================================
* @file      hw_ios.c
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of hw_ios.c 
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 			
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_IOS_H__
#define __HW_IOS_H__

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/   
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/   
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
/**
 * @brief  initializes Sirop I/Os
 *
 * @note
 * @retval None
 */
void  HW_IOs_Init( void  );

/**
 * @brief  deinitializes Sirop I/Os
 *
 * @note
 * @retval None
 */
void  HW_IOs_DeInit( void  );

/**
  * @brief initilizes Radio IOs as Analog
  * @param none
  * @retval none
  */
void HW_RadioIoInit_Analog(void);

#ifdef __cplusplus
}
#endif

#endif /* __HW_IOS_H__ */

/*****END OF FILE****/
