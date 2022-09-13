/**============================================================================
* @file      RS485.h
* @date      2018-06-14
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     * Header of driver for RS485 communication
*            
* VERSION:   
* 
* V0.01      2018-06-14-Kd      Create File			
*
*============================================================================*/
/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __RS485_H
#define __RS485_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>   
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

bool RS485_TxDataPolled(uint8_t *pData, uint16_t Size, uint32_t Timeout);

#ifdef __cplusplus
}
#endif

#endif //__RS485_H

/*****END OF FILE****/
