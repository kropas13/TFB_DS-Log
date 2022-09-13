/**============================================================================
* @file      com.h
* @date      2018-06-11
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of com.c
*            
* VERSION:   
* 
* V0.10      2018-06-11-Kd      Create File 			
*
*============================================================================*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COM_H
#define __COM_H

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/	
#include <stdint.h>
#include <stdbool.h>
   
/* Exported types ------------------------------------------------------------*/
#define COM_TX_DATA_SIZE                128
   
/* Exported constants --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
extern bool bCom_SendMeasData;
extern bool bCom_GetMeasData;
   
/* Exported functions ------------------------------------------------------- */ 	

/**
  * @brief  Initializes application 
  */
uint16_t Com_HandleTelegramm( uint8_t* pui8RxBuffAddr, uint16_t ui16RxBuffLen, uint8_t* pui8TxBuffAddr, bool* pbRequestTxProlong);
	 
	 
#ifdef __cplusplus
}
#endif

#endif /*__COM_H */

/*****END OF FILE****/
