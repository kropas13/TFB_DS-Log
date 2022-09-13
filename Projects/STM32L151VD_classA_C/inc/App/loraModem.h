 /*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Generic lora driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Gregory Cristian and Wael Guibene
*/
/**============================================================================
* @file      loraModem.h
* @date      2016-12-05
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of Lora Modem application for Multi Sensor 3 (Sense Two)
*            
* VERSION:   
* 
* V0.01       2016-12-05-Ra      Create File 		
* V0.02       2016-12-27-Ra      Merged with loraModem concept of sensile	
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORAMODEM_H__
#define __LORAMODEM_H__
#ifdef __cplusplus
 extern "C" {
#endif

//#include "stm32l1xx.h"
#include "lora.h"
	 
/* Exported constants --------------------------------------------------------*/ 
/*!
 * Defines the application data transmission duty cycle [ms].
 */
#define LORAWAN_TX_DUTYCYCLE                     0 // per default turned off, application decides when to send
/*!
 * Indicates if the end-device is to be connected to a private or public network
 */
#define LORAWAN_PUBLIC_NETWORK                   true 
/*!
 * LoRaWAN Adaptive Data Rate
 * @note Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_ON                           1
/*!
 * LoRaWAN ETSI duty cycle control enable/disable
 *
 * \remark Please note that ETSI mandates duty cycled transmissions. Use only for test purposes
 */
#define LORAWAN_DUTYCYCLE_ON                     true
/*!
 * LoRaWAN application port
 * @note do not use 224. It is reserved for certification
 */
#define LORAWAN_DEF_PORT                         3
/*!
 * loraModem TX/RX done check time [s]
 */
#define LORAMODEM_TXRX_CHECK_TIME                1
/*!
 * loraModem TX/RX done check time for Joining [s]
 */
#define LORAMODEM_JOIN_CHECK_TIME                30

/* Exported types ------------------------------------------------------------*/   
typedef enum
	{
	  //TxRx Start OK
	  TX_RX_START_DONE,
		
		//TxRx Start NOK
		TX_RX_START_ERROR,
		
		//TxRx DONE
		TX_RX_DONE,
		
		//Rx ERROR 
		RX_ERROR,
		
		//Tx ERROR
		TX_ERROR,
		
		//Tx DELAY/RUNNING
		TX_DELAYED_OR_RUNNING,
		
		//JOIN ERROR
		JOIN_ERROR		
	} MessageStatus_t;	

/* External variables --------------------------------------------------------*/
extern LoRaParam_t sLoRaParamInit; 

/* Exported functions ------------------------------------------------------- */ 
/**
  * @brief  Inits Lora modem application
  */
bool IsDeviceClassC(void);

/**
  * @brief  Set default TX duty time and ABP for LoRa
  */
void LoraSetConfigDefault(void);

/**
  * @brief  Tx/RxPort setter 
  */
void LoraSetPorts(uint8_t TxPort, uint8_t RxPort);

/**
  * @brief  Returns actual RxPort
  */
uint8_t LoraGetRxPort(void);


/**
  * @brief  Returns actual TxPort
  */
uint8_t LoraGetTxPort(void);

/**
  * @brief  turns on Radio Module and initializes modem
  */
void LoraON( void );

/**
  * @brief  turns off radio module
  */
void LoraOFF( bool LoraDeInit );

/**
  * @brief  gets the max, payload sice for next Tx
  */
uint8_t LoraGetMaxPayloadSize( bool bConfirmedTx );

/**
  * @brief  Inits Lora modem application
  */
void LoraInit(void);

/**
  * @brief  sets a flag to activate a TX_ON_EVENT
  */
void LoraTxMessageSet(void);

/**
  * @brief  Function used to start a LoraTxRx event. The same can also be done by combining LoraTxMessageSet and LoraDo
  */
MessageStatus_t LoraTxRxStart(void);

/**
  * @brief  returns the state of the bModemLocked flag
  */
bool LoraTxRxStatus(void);

void LoRaGetAndCalcRssiSnr( uint8_t *pui8LastRssi, int8_t *pi8LastSnr );

/**
  * @brief  calls the lora fsm function until its done
  */
void LoraDo( void );

#ifdef __cplusplus
}
#endif

#endif /*__LORAMODEM_H__ */

/*****END OF FILE****/
