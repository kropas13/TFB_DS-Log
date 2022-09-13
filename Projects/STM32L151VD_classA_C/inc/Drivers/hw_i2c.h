/**============================================================================
* @file      hw_i2c.h
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of hw_i2c.c
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 			
*
*============================================================================*/

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __HW_I2C_H__
#define __HW_I2C_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal_i2c.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
/** 
* @brief  Init the I2C1.
* @param  None
* @return None
*/
HAL_StatusTypeDef HW_I2C1_Init(void);

   /** 
* @brief  DeInit the I2C1.
* @param  None
* @return None
*/
HAL_StatusTypeDef HW_I2C1_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __HW_I2C_H__ */

/*****END OF FILE****/
