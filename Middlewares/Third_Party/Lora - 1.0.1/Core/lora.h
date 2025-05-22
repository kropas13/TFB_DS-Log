 /*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRaMac classA device implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Gregory Cristian and Wael Guibene
*/
/******************************************************************************
  * @file    lora.h
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    15-November-2016
  * @brief   lora API to drive the lora state Machine
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __LORA_MAIN_H__
#define __LORA_MAIN_H__

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>   
#include "stm32l1xx.h"
#include "LoRaMac.h"	 
#include "LoRaMacCommon.h"
#include "LoRaMac-board.h"
#include "loraModem_flags.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/*!
 * Application Data structure
 */
typedef struct
{
  /*point to the LoRa App data buffer*/
  uint8_t* Buff;
  /*LoRa App data buffer size*/
  uint8_t BuffSize;
  /*Port on which the LoRa App is data is sent/ received*/
  uint8_t Port;
  /*Confirm the received data*/
  bool    bConfirm;
  /*Multicast Message*/
  bool    bMulticast;
  /* Rssi of last received packet */
  int16_t Rssi;
  /* Snr of last received packet */
  uint8_t Snr;  
    
  
} lora_AppData_t;

/*!
 * LoRa State Machine states 
 */
typedef enum eDeviceState
{
    DEVICE_STATE_INIT,
    DEVICE_STATE_JOIN,
		DEVICE_STATE_WAIT,
    DEVICE_STATE_JOINED,
    DEVICE_STATE_SEND,
    DEVICE_STATE_CYCLE,  
    DEVICE_STATE_SLEEP
} DeviceState_t;

/*!
 * LoRa Event sources as defined by comtac 
 */
typedef enum
{
    TX_ON_TIMER     = 0x01,
    TX_ON_TIMER_EXT = 0x02,
    TX_ON_EVENT     = 0x10,
	  TX_NONE         = 0x00
		
} TxEventType_t;

/*!
 * LoRa Cfg structure as defined by comtac 
 */
typedef struct{
  uint16_t ui16SendInterval_m; 
  // LoRaWAN
  uint8_t  u1OTA:1;         // OverTheAir Activation
  uint8_t  u1PrivateNetwork:1;
  uint8_t  u1ADR:1;         
  uint8_t  u6Reserve:5;
  uint8_t OTA_DevEui[8];  // OTA (LORAWAN_DEVICE_EUI)
  uint8_t OTA_AppEui[8];  // OTA (LORAWAN_APPLICATION_EUI)
  uint8_t OTA_AppKey[16]; // OTA (LORAWAN_APPLICATION_KEY)
  // or ABP (DeviceActivationByPersonalization)
  uint32_t ABP_NetworkID;
  uint32_t ABP_DeviceAddr;
  uint8_t  ABP_NetworkSessionKey[16];
  uint8_t  ABP_ApplicationSessionKey[16]; 
  // Broadcast 
  uint32_t BC_Addr;        
  uint8_t  BC_NetworkSessionKey[16];
  uint8_t  BC_ApplicationSessionKey[16]; 
  // LoRaMAC Datarate
  uint8_t  ui8MinDatarate;
  uint8_t  ui8MaxDatarate;
  uint8_t  ui8DefaultDatarate;      // Uplink datarate, if AdrEnable is off 
  uint8_t  ui8Rx2DefaultDatarate;
} tLoRaWAN_Cfg;

/*!
 * LoRa parameters structure as defined by comtac
 */
typedef struct
{
/*!
 * @brief LoRaWAN device class
 */
    DeviceClass_t Class;
/*!
 * @brief Activation state of adaptativeDatarate
 */
    bool AdrEnable;
/*!
 * @brief Enable or disable a public network
 *
 */
    bool EnablePublicNetwork;
/*!
 * @brief Enable or disable internal LoRaMac duty cycle control
 *
 */
    bool bDutyCycleOn;  
/*!
 * @brief Random time to be added/substracted to send interval
 *
 */
	  uint32_t DutyCycleRandomTime;  
/*!
 * @brief Sensor capture time to be substracted from send interval
 *
 */   
  	uint32_t DutyCycleSensorCaptureTime;  
/*!
 * @brief LoRaWAN configurable settings
 */    
    tLoRaWAN_Cfg *pLoRaWANcfg;
		
} LoRaParam_t;

/* Lora Main callbacks with some comtac ADDS (see comtac ADD)*/
typedef struct sLoRaMainCallback
{
/*!
 * @brief Get the current battery level
 *
 * @retval value  battery level ( 0: very low, 254: fully charged )
 */
    uint8_t ( *BoardGetBatteryLevel )( void );
  
/*!
 * @brief Gets the board 64 bits unique ID 
 *
 * @param [IN] id Pointer to an array that will contain the Unique ID
 */
    void    ( *BoardGetUniqueId ) ( uint8_t *id);
 /*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * @retval seed Generated pseudo random seed
 */
    uint32_t ( *BoardGetRandomSeed ) (void);
/*!
 * comtac ADD : For tests to meassure the Rx1 windows lenght (active when windows open)
 *
 * @retval None
 */  
    void ( *BoardSetRx1WindowsState ) ( bool bState);
/*!
 * comtac ADD : For tests to meassure the Rx2 windows lenght (active when windows open)
 *
 * @retval None
 */      
    void ( *BoardSetRx2WindowsState ) ( bool bState);  
/*!
 * comtac ADD : For LED control, min. 25ms active when message transmit is ongoing
 *
 * @retval None
 */    
    void ( *BoardSetLedState ) ( bool bState);
/*!
 * comtac ADD : For LED control, 25ms active when message received 
 *
 * @retval None
 */     
    void ( *BoardSetLedLink ) ( bool bState);
/*!
 * @brief Prepares Tx Data to be sent on Lora network 
 *
 * @param [IN] AppData is a buffer to fill
 *
 * @param [IN] port is a Application port on wicth Appdata will be sent
 *
 * @param [IN] length of the AppDataBuffer to send
 *
 * @param [IN] requests a confirmed Frame from the Network
 *
 * @param [IN] comtac ADD --> on event TX 
 */
    void ( *LoraTxData ) ( lora_AppData_t *AppData, FunctionalState* IsTxConfirmed, uint8_t u8OnEvent);
/*!
 * @brief Process Rx Data received from Lora network 
 *
 * @param [IN] AppData is a buffer to process
 *
 * @param [IN] port is a Application port on wicth Appdata will be sent
 *
 * @param [IN] length is the number of recieved bytes
 */
    void ( *LoraRxData ) ( lora_AppData_t *AppData);
/*!
* comtac ADD: are there events?
 *
 * @retval true -> yes; false -> no
 */       
    uint8_t ( *HasTxEvents) ( void );		
		
} LoRaMainCallback_t;

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
/**
 * @brief Lora default Configuration as defined by comtac
 * @param [IN] pLoRaWAN_Cfg pointer to comtac's Lora Cfg file
 * @retval none
 */
void lora_DefaultConfig (tLoRaWAN_Cfg *pLoRaWAN_Cfg );
/**
 * @brief Lora Initialisation
 * @param [IN] LoRaMainCallback_t
 * @param [IN] application parmaters
 * @retval none
 */
void lora_Init (LoRaMainCallback_t *callbacks, LoRaParam_t* LoRaParamInit );

/**
 * @brief run Lora classA state Machine 
 * @param [IN] none
 * @retval comtac ADD: return the value of the TxDutyCycleTime
 */
uint32_t lora_fsm( void );

/**
 * @brief API returns the state of the lora state machine
 * @note return @DeviceState_t state
 * @param [IN] none
 * @retval return @FlagStatus
  */
DeviceState_t lora_getDeviceState( void );

/**
 * @brief API returns the last MacStatus
 * @note return MacStatus, has to be used after having sent a request
 * @param [IN] none
 * @retval return @LoRaMacStatus_t
  */
LoRaMacStatus_t lora_getMacStatus (void);

/**
 * @brief API returns the NextTx flag 
 * @note return NextTx
 * @param [IN] none
 * @retval return @FlagStatus
  */
bool lora_getNextTx(void);

/**
 * @brief API returns the last ModemFlags
 * @note return ModemFlags
 * @param [IN] none
 * @retval return @FlagStatus
  */
uint8_t lora_getModemFlags (void);

int16_t lora_getLastRssi( void);
uint8_t lora_getLastSnr( void);

#ifdef __cplusplus
}
#endif

#endif /*__LORA_MAIN_H__*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
