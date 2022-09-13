/**============================================================================
* @file      hw_uart.h
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of hw_uart
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 			
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_UART_H__
#define __HW_UART_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32l1xx_hal.h"
   
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
// extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_tx;   
/* Exported functions ------------------------------------------------------- */ 

/** 
* @brief  Init the UART1.
* @param  None
* @return None
*/
void HW_UART1_Init(void);

   /** 
* @brief  DeInit the UART1.
* @param  None
* @return None
*/
void HW_UART1_DeInit(void);

/** 
* @brief  Init the UART3.
* @param  None
* @return None
*/
void HW_UART3_Init(void);

   /** 
* @brief  DeInit the UART3.
* @param  None
* @return None
*/
void HW_UART3_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __HW_UART_H__*/

/*****END OF FILE****/
