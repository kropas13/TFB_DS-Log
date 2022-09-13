/**============================================================================
* @file      loraModem_flags.h
* @date      2016-12-15
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     contains  the comtac implemented SX1272 flags for lora modem applications
*            
* VERSION:   
* 
* V0.10      2016-12-15-Ra      Create File 			
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORAMODEM_FLAGS_H
#define __LORAMODEM_FLAGS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum
{
	NONE       = 0x00,
	JOINDONE   = 0x01,
	TXDONE     = 0x02, //Tx done --> either only data or data and mac (usually we tx only when we have data to transmit, mac gets automatically added to these Tx)
	TXTIMEOUT  = 0x04, //Tx timeout 
	RXDONE     = 0x08, //Rx done --> either mac, data, or mac and data		
	RX2TIMEOUT = 0x10, //Rx window 2 timeout 
  RXERROR	   = 0x20, //Rx error
	ACKED      = 0x40,  //ACK Ok
	JOINERROR  = 0x80
} ModemFlags_t;

/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
#ifdef __cplusplus
}
#endif

#endif /* __LORAMODEM_FLAGS_H */


/*****END OF FILE****/
