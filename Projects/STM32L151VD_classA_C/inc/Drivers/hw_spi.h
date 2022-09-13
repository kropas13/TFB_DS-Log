/**============================================================================
* @file      hw_spi.h
* @date      2016-12-05
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of hw_spi.c

*            
* VERSION:   
* 
* V0.01      2016-12-05-Ra      Create File 
* V0.02		 2016-12-09-Ra      hspi1 defined as extern 	
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_SPI_H__
#define __HW_SPI_H__

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"   

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

/*!
 * @brief Initializes the SPI object and MCU peripheral
 *
 * @param [IN] none
 */
HAL_StatusTypeDef HW_SPI2_Init( void );

/*!
 * @brief De-initializes the SPI object and MCU peripheral
 *
 * @param [IN] none
 */
HAL_StatusTypeDef HW_SPI2_DeInit( void );

/*!
 * @brief Initializes the SPI object and MCU peripheral
 *
 * @param [IN] none
 */
HAL_StatusTypeDef HW_SPI3_Init( void );

/*!
 * @brief De-initializes the SPI object and MCU peripheral
 *
 * @param [IN] none
 */
HAL_StatusTypeDef HW_SPI3_DeInit( void );


/*!
 * @brief Sends outData and receives inData
 *
 * @param [IN] outData Byte to be sent
 * @retval inData      Received byte.
 */
uint8_t HW_SPI2_InOut( uint8_t txData );

/*!
 * @brief Sends outData and receives inData
 *
 * @param [IN] outData Byte to be sent
 * @retval inData      Received byte.
 */
uint8_t HW_SPI3_InOut( uint8_t txData );

#ifdef __cplusplus
}
#endif

#endif  /* __HW_SPI_H__ */

/*****END OF FILE****/
