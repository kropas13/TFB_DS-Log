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
  * @file    lora.c
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

/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "timeServer.h"
#include "low_power.h"
#include "lora.h"
#include "Comissioning.h"

#if defined( USE_BAND_868 )

#include "LoRaMacTest.h"

#define USE_SEMTECH_DEFAULT_CHANNEL_LINEUP          0

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 ) 

#define LC4                { 867100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 867300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 867500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 867700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 867900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC9                { 868800000, { ( ( DR_7 << 4 ) | DR_7 ) }, 2 }
#define LC10               { 868300000, { ( ( DR_6 << 4 ) | DR_6 ) }, 1 }

#endif

#endif

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFF_SIZE                           255

/*!
 * User application data
 */
static uint8_t AppDataBuff[LORAWAN_APP_DATA_BUFF_SIZE];

/*!
 * User application data structure
 */
static lora_AppData_t AppData={ AppDataBuff,  0 ,0 };

/*!
 * Indicates if the node is sending confirmed or unconfirmed messages
 */
static FunctionalState IsTxConfirmed ;

/*!
 * Defines the LoRa parameters at Init
 */
static  LoRaParam_t* LoRaParam;

/*!
 * Defines the LoRa Mac configuration structure as defined by comtac in LoRaMac.h
 */
static LoRaMacCfg_t  LoraMacCfg;

/*!
 * Defines the application data transmission duty cycle as defined by comtac
 */
static uint32_t TxDutyCycleTime = 0;

/*!
 * Timer to handle the application data transmission duty cycle
 */
static TimerEvent_t TxNextPacketTimer;

/*!
 * Timer to handle the join trials as implemented by comtac
 */
static TimerEvent_t JoinTrialTimer;

/*!
 * Initial state of lora_fsm
 */
static DeviceState_t DeviceState = DEVICE_STATE_INIT ;

/*!
 * Event detection flags as implemented by comtac
 */
static uint8_t u8LocalTxType = TX_NONE;

/*!
 * Timer to handle the state of LED State as added by comtac
 */
static TimerEvent_t LedStateTimer;

/*!
 * Timer to handle the state of LED LinkAct as added by comtac
 */
static TimerEvent_t LedLinkActTimer;

static LoRaMacPrimitives_t LoRaMacPrimitives;
static LoRaMacCallback_t LoRaMacCallbacks;
static MibRequestConfirm_t mibReq;

static LoRaMainCallback_t *LoRaMainCallbacks;


/*!
 * Indicates if a new packet can be sent
 */
static bool NextTx = true;

/*!
 * Actual LoRaMacStatus
 */
static LoRaMacStatus_t MacStatus = LORAMAC_STATUS_OK;


/* Modem Flags initialization defined by comtac [for now global] */
static uint8_t ModemFlags = NONE; 

/*!
 * LoRaWAN compliance tests support data
 */
struct ComplianceTest_s
{
    bool Running;
    uint8_t State;
    FunctionalState IsTxConfirmed;
    uint8_t AppPort;
    uint8_t AppDataSize;
    uint8_t *AppDataBuffer;
    uint16_t DownLinkCounter;
    bool LinkCheck;
    uint8_t DemodMargin;
    uint8_t NbGateways;
}ComplianceTest;

/*!
 * \brief   Prepares the payload of the frame, bOnEvent variable added by comtac for payload indication purposes
 */
static void PrepareTxFrame( uint8_t u8OnEvent )
{
    if( ComplianceTest.Running == true )
    {
        if( ComplianceTest.LinkCheck == true )
        {
            ComplianceTest.LinkCheck = false;
            AppData.BuffSize = 3;
            AppData.Buff[0] = 5;
            AppData.Buff[1] = ComplianceTest.DemodMargin;
            AppData.Buff[2] = ComplianceTest.NbGateways;
            ComplianceTest.State = 1;
        }
        else
        {
            switch( ComplianceTest.State )
            {
            case 4:
                ComplianceTest.State = 1;
                break;
            case 1:
                AppData.BuffSize = 2;
                AppData.Buff[0] = ComplianceTest.DownLinkCounter >> 8;
                AppData.Buff[1] = ComplianceTest.DownLinkCounter;
                break;
            }
        }
    }
    else
    {   
        //bOnEvent extension by comtac used here for payload indication  			
        LoRaMainCallbacks->LoraTxData(&AppData, &IsTxConfirmed, u8OnEvent);
    }
}

/*!
 * \brief   Prepares the payload of the frame
 *
 * \retval  [0: frame could be send, 1: error]
 */
static bool SendFrame( void )
{
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;
    
    if( LoRaMacQueryTxPossible( AppData.BuffSize, &txInfo ) != LORAMAC_STATUS_OK )
    {
        // Send empty frame in order to flush MAC commands
        mcpsReq.Type = MCPS_UNCONFIRMED;
        mcpsReq.Req.Unconfirmed.fBuffer = NULL;
        mcpsReq.Req.Unconfirmed.fBufferSize = 0;
				//comtac correction with usage of LoraParam structure
        mcpsReq.Req.Unconfirmed.Datarate = LoRaParam->pLoRaWANcfg->ui8DefaultDatarate;
    }
    else
    {
        if( IsTxConfirmed == DISABLE )
        {
            mcpsReq.Type = MCPS_UNCONFIRMED;
            mcpsReq.Req.Unconfirmed.fPort = AppData.Port;
            mcpsReq.Req.Unconfirmed.fBuffer = AppData.Buff;
            mcpsReq.Req.Unconfirmed.fBufferSize = AppData.BuffSize;
				    //comtac correction with usage of LoraParam structure					
            mcpsReq.Req.Unconfirmed.Datarate = LoRaParam->pLoRaWANcfg->ui8DefaultDatarate;
        }
        else
        {
            mcpsReq.Type = MCPS_CONFIRMED;
            mcpsReq.Req.Confirmed.fPort = AppData.Port;
            mcpsReq.Req.Confirmed.fBuffer = AppData.Buff;
            mcpsReq.Req.Confirmed.fBufferSize = AppData.BuffSize;
            mcpsReq.Req.Confirmed.NbTrials = 8;
				    //comtac correction with usage of LoraParam structure					
            mcpsReq.Req.Confirmed.Datarate = LoRaParam->pLoRaWANcfg->ui8DefaultDatarate;
        }
    }
    if( (LoRaMacMcpsRequest( &mcpsReq )) == LORAMAC_STATUS_OK )
    {
        return false;
    }
    return true;
}

void OnSendEvent( void )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    mibReq.Type = MIB_NETWORK_JOINED;
    status = LoRaMacMibGetRequestConfirm( &mibReq );

    if( status == LORAMAC_STATUS_OK )
    {
        if( mibReq.Param.IsNetworkJoined == true )
        {
            DeviceState = DEVICE_STATE_SEND;
            NextTx = true;
        }
        else
        {
            DeviceState = DEVICE_STATE_JOIN;
        }
    }
}
/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static void OnTxNextPacketTimerEvent( void )
{
    TimerStop( &TxNextPacketTimer );	
		if (DeviceState == DEVICE_STATE_SLEEP){
		  u8LocalTxType |= TX_ON_TIMER;  
		}	
    OnSendEvent();
}

/*!
 * \brief Function executed on JoinTrial Timeout event, added by comtac
 */
static void OnJoinTrialTimerEvent( void )
{
    TimerStop( &JoinTrialTimer);	
    ModemFlags = JOINERROR;
		// Schedule next packet transmission        
		if (LoRaParam->pLoRaWANcfg->ui16SendInterval_m){   
			// Usage of TxDutyCycleTime defined by comtac here, this is the functions return						
			TxDutyCycleTime = (LoRaParam->pLoRaWANcfg->ui16SendInterval_m*60000) + randr( -LoRaParam->DutyCycleRandomTime, LoRaParam->DutyCycleRandomTime ) - LoRaParam->DutyCycleSensorCaptureTime;        
		} else {
			// Usage of TxDutyCycleTime defined by comtac here, this is the functions return						
			TxDutyCycleTime = 0;
		}		
    DeviceState  = DEVICE_STATE_CYCLE;				
}

/*!
 * \brief Function executed on Led State Timeout event as defined by comtac
 */
static void OnLedStateTimerEvent( void )
{
    TimerStop( &LedStateTimer );
	  // until now 2016-12-06-Ra left as in comtac MS3(SenseTwo), for changes see loraModem.c at line 295
    if (LoRaMainCallbacks->BoardSetLedState)
      LoRaMainCallbacks->BoardSetLedState(false); // Switch LED State OFF
}

/*!
 * \brief Function executed on Led LinkAct Timeout event as defined by comtac
 */
static void OnLedLinkActTimerEvent( void )
{
    TimerStop( &LedLinkActTimer );
	  // until now 2016-12-06-Ra left as in comtac MS3(SenseTwo), for changes see loraModem.c at line 307	
    if (LoRaMainCallbacks->BoardSetLedLink)
      LoRaMainCallbacks->BoardSetLedLink(false);// Switch LED LinkAct OFF
}

/*!
 * \brief   MCPS-Confirm event function
 *
 * \param   [IN] McpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
	  //code added by comtac for StateTimerEvent functionality
    bool bSuccess = true;
  
	  // LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT is active when no ACK was received.
    // We need to let this go inside this if to be able to fill the flags correctly!
    if( mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK || mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT )
    {			
      ModemFlags |= TXDONE | RX2TIMEOUT;
			switch( mcpsConfirm->McpsRequest )
      {
        case MCPS_UNCONFIRMED:
        {      
          // Check Datarate
          // Check TxPower
        break;
        }
				
        case MCPS_CONFIRMED:
				{	
          // Check Datarate
          // Check TxPower
          // Check AckReceived
          // Check NbTrials
	        //code added by comtac for StateTimerEvent functionality							
          if (!mcpsConfirm->AckReceived){
            bSuccess = false;
					} else {
				    ModemFlags |= ACKED;
          }									
        break;
				}	
					
        case MCPS_PROPRIETARY:
				{	
        break;
				}
				
        default:
        break;
      }
		  //------------------------------------------------------------------------------	 
		  /** comtac INFO
		   *
		   *  code added for StateTimerEvent functionality (LED control)
		   *                	 
		   *  Author: 2016-12-06-Ra  
		   **/        
      if (bSuccess){
        // Switch LED LedState ON always
        if (LoRaMainCallbacks->BoardSetLedState){
          LoRaMainCallbacks->BoardSetLedState(true);       
          TimerStart( &LedStateTimer );
				}	
      } else {
					 // At the moment, we let LoRaMac reduce the DR by itself
					 // Every two tries LoRaMac reduces one DR. It does this 8 times. (done at line 257)
		  }
      //------------------------------------------------------------------------------	        
    } else if ( mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT){
      ModemFlags |= TXTIMEOUT;
    } else if (mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_RX2_ERROR){
      ModemFlags |= RXERROR;
    } 			
    NextTx = true;
}

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 */
static void McpsIndication( McpsIndication_t *mcpsIndication )
{ 
	if ( mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK )
	{
    ModemFlags |= RXERROR;
    ModemFlags &= (~RX2TIMEOUT);		
	} 
	else 
	{
    AppData.bConfirm = false;
    AppData.bMulticast = false;
    
		ModemFlags |= RXDONE;
    ModemFlags &= (~RX2TIMEOUT);				
    switch( mcpsIndication->McpsIndication )
    {
      case MCPS_UNCONFIRMED:
			{	
      break;
      }
			
      case MCPS_CONFIRMED:
			{	
        AppData.bConfirm = true; 
      break;
      }
			
      case MCPS_PROPRIETARY:
      {
			break;
      }
			
      case MCPS_MULTICAST:
			{	
        AppData.bMulticast = true;
      break;
      }
				
      default:
      break;
    }

    // Check Multicast
    // Check Port
    // Check Datarate
    // Check FramePending
    // Check Buffer
    // Check BufferSize
    // Check Rssi
    // Check Snr
    // Check RxSlot
    
    AppData.Rssi = mcpsIndication->Rssi;
    AppData.Snr = mcpsIndication->Snr;        

    if( ComplianceTest.Running == true )
    {
      ComplianceTest.DownLinkCounter++;
    }

    if( mcpsIndication->RxData == true )
    {
      switch( mcpsIndication->Port )
      {
        case 224:
          if( ComplianceTest.Running == false )
          {
            // Check compliance test enable command (i)
            if( ( mcpsIndication->BufferSize == 4 ) &&
                ( mcpsIndication->Buffer[0] == 0x01 ) &&
                ( mcpsIndication->Buffer[1] == 0x01 ) &&
                ( mcpsIndication->Buffer[2] == 0x01 ) &&
                ( mcpsIndication->Buffer[3] == 0x01 ) )
            {
              IsTxConfirmed = DISABLE;
              AppData.Port = 224;
              AppData.BuffSize = 2;
              ComplianceTest.DownLinkCounter = 0;
              ComplianceTest.LinkCheck = false;
              ComplianceTest.DemodMargin = 0;
              ComplianceTest.NbGateways = 0;
              ComplianceTest.Running = true;
              ComplianceTest.State = 1;
                    
              MibRequestConfirm_t mibReq;
              mibReq.Type = MIB_ADR;
              mibReq.Param.AdrEnable = true;
              LoRaMacMibSetRequestConfirm( &mibReq );

#if defined( USE_BAND_868 )
                  LoRaMacTestSetDutyCycleOn( false );
#endif
            }
          }
          else
          {
            ComplianceTest.State = mcpsIndication->Buffer[0];
            switch( ComplianceTest.State )
            {
              case 0: // Check compliance test disable command (ii)
                ComplianceTest.DownLinkCounter = 0;
                ComplianceTest.Running = false;
                    
                MibRequestConfirm_t mibReq;
                mibReq.Type = MIB_ADR;
                mibReq.Param.AdrEnable = LoRaParam->AdrEnable;
                LoRaMacMibSetRequestConfirm( &mibReq );
#if defined( USE_BAND_868 )
				        //comtac correction with usage of LoraParam structure								
                LoRaMacTestSetDutyCycleOn( LoRaParam->bDutyCycleOn );
#endif
              break;
              
							case 1: // (iii, iv)
                AppData.BuffSize = 2;
              break;
              
							case 2: // Enable confirmed messages (v)
                IsTxConfirmed = ENABLE;
                ComplianceTest.State = 1;
              break;
              
							case 3:  // Disable confirmed messages (vi)
                IsTxConfirmed = DISABLE;
                ComplianceTest.State = 1;
              break;
                
							case 4: // (vii)
                AppData.BuffSize = mcpsIndication->BufferSize;

                AppData.Buff[0] = 4;
                for( uint8_t i = 1; i < AppData.BuffSize; i++ )
                {
                  AppData.Buff[i] = mcpsIndication->Buffer[i] + 1;
                }
              break;
              
							case 5: // (viii)
              {
                MlmeReq_t mlmeReq;
                mlmeReq.Type = MLME_LINK_CHECK;
                LoRaMacMlmeRequest( &mlmeReq );
              }
              break;
							
              case 6: // (ix)
              {
                MlmeReq_t mlmeReq;

                mlmeReq.Type = MLME_JOIN;
				        
								//comtac correction with usage of LoraParam structure			
                mlmeReq.Req.Join.DevEui = LoRaParam->pLoRaWANcfg->OTA_DevEui;
                mlmeReq.Req.Join.AppEui = LoRaParam->pLoRaWANcfg->OTA_AppEui;
                mlmeReq.Req.Join.AppKey = LoRaParam->pLoRaWANcfg->OTA_AppKey;

                LoRaMacMlmeRequest( &mlmeReq );
                DeviceState = DEVICE_STATE_SLEEP;
              }
              break;
               
							default:                  
              break;
            }
          }
        break;
					
        default:
            
          AppData.Port = mcpsIndication->Port;
          AppData.BuffSize = mcpsIndication->BufferSize;
          memcpy1( AppData.Buff, mcpsIndication->Buffer, AppData.BuffSize );
            
          LoRaMainCallbacks->LoraRxData( &AppData );
        break;
      }
    }
		
		//------------------------------------------------------------------------------	 
    /** comtac INFO
		 *
     *  code added for LinkActTimerEvent functionality (LED control)
		 *  
 		 *  switches LED LinkAct ON for each received downlink  
     * 		
     *  Author: 2016-12-06-Ra  
     **/ 
    if (LoRaMainCallbacks->BoardSetLedLink){
      LoRaMainCallbacks->BoardSetLedLink(true);
			TimerStart( &LedLinkActTimer );
		}
     
		//------------------------------------------------------------------------------
  }		
}

/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] MlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
    {
        switch( mlmeConfirm->MlmeRequest )
        {
            case MLME_JOIN:
            {
                // Status is OK, node has joined the network
                DeviceState = DEVICE_STATE_JOINED;
                break;
            }
            case MLME_LINK_CHECK:
            {
                // Check DemodMargin
                // Check NbGateways
                if( ComplianceTest.Running == true )
                {
                    ComplianceTest.LinkCheck = true;
                    ComplianceTest.DemodMargin = mlmeConfirm->DemodMargin;
                    ComplianceTest.NbGateways = mlmeConfirm->NbGateways;
                }
                break;
            }
            default:
                break;
        }
    } else if ( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL )	{
		  PRINTF("JOIN FAILED\r\n");
    }			
		NextTx = true;
}

/**
 *  lora default config funtion as defined by comtac
 */
void lora_DefaultConfig (tLoRaWAN_Cfg *pLoRaWAN_Cfg )
{  
  #if (STATIC_DEVICE_EUI != 1)
    HW_GetUniqueId( (uint8_t*)&pLoRaWAN_Cfg->OTA_DevEui );  // Initialize LoRaMac device unique ID  
  #else  
    memcpy(pLoRaWAN_Cfg->OTA_DevEui, LORAWAN_DEVICE_EUI, sizeof(pLoRaWAN_Cfg->OTA_DevEui)); // IEEE EUI-64 defined in commisioning.h               
  #endif  
  #if (STATIC_DEVICE_ADDRESS != 1)
    pLoRaWAN_Cfg->ABP_DeviceAddr = HW_GetRandomSeed() & 0x01FFFFFF; // LORAWAN_DEVICE_ADDRESS;     
  #else
    pLoRaWAN_Cfg->ABP_DeviceAddr = LORAWAN_DEVICE_ADDRESS;
  #endif
    memcpy(pLoRaWAN_Cfg->OTA_AppEui, LORAWAN_APPLICATION_EUI, sizeof(pLoRaWAN_Cfg->OTA_AppEui));
    memcpy(pLoRaWAN_Cfg->OTA_AppKey, LORAWAN_APPLICATION_KEY, sizeof(pLoRaWAN_Cfg->OTA_AppKey));
    
		pLoRaWAN_Cfg->ABP_NetworkID = LORAWAN_NETWORK_ID;    
    memcpy(pLoRaWAN_Cfg->ABP_ApplicationSessionKey, LORAWAN_APPSKEY, sizeof(pLoRaWAN_Cfg->ABP_ApplicationSessionKey));
    memcpy(pLoRaWAN_Cfg->ABP_NetworkSessionKey, LORAWAN_NWKSKEY, sizeof(pLoRaWAN_Cfg->ABP_NetworkSessionKey)); 
    
		// Broadcast
    pLoRaWAN_Cfg->BC_Addr = LORAWAN_BROADCAST_ADDRESS; 
    memcpy(pLoRaWAN_Cfg->BC_ApplicationSessionKey, LORAWAN_BC_APPSKEY, sizeof(pLoRaWAN_Cfg->BC_ApplicationSessionKey));
    memcpy(pLoRaWAN_Cfg->BC_NetworkSessionKey, LORAWAN_BC_NWKSKEY, sizeof(pLoRaWAN_Cfg->BC_NetworkSessionKey)); 
    
		// LoRaMAC Datarate
    pLoRaWAN_Cfg->ui8MinDatarate        = LORAMAC_TX_MIN_DATARATE;
    pLoRaWAN_Cfg->ui8MaxDatarate        = LORAMAC_TX_MAX_DATARATE;
    pLoRaWAN_Cfg->ui8DefaultDatarate    = LORAMAC_DEFAULT_DATARATE;     
    pLoRaWAN_Cfg->ui8Rx2DefaultDatarate = LORAMAC_DEFAULT_RX2_DATARATE;  
}

/**
 *  lora Init
 */
void lora_Init (LoRaMainCallback_t *callbacks, LoRaParam_t* LoRaParamInit )
{
  /* init the DeviceState*/
  DeviceState= DEVICE_STATE_INIT;
  
  /* init the Tx Duty Cycle*/
  LoRaParam = LoRaParamInit;
  
  /* init the main call backs*/
  LoRaMainCallbacks = callbacks;  
 
	//comtac correction with usage of LoraParam structure			
  if (LoRaParam->pLoRaWANcfg->u1OTA)
  {
    PRINTF("[info] OTAA\r\n"); 
	  //comtac correction with usage of LoraParam structure				
    PRINTF("[info] DevEui= %02X", LoRaParam->pLoRaWANcfg->OTA_DevEui[0]) ;for(int i=1; i<8 ; i++) {PRINTF("-%02X", LoRaParam->pLoRaWANcfg->OTA_DevEui[i]); }; PRINTF("\r\n");
    PRINTF("[info] AppEui= %02X", LoRaParam->pLoRaWANcfg->OTA_AppEui[0]) ;for(int i=1; i<8 ; i++) {PRINTF("-%02X", LoRaParam->pLoRaWANcfg->OTA_AppEui[i]); }; PRINTF("\r\n");
    PRINTF("[info] AppKey= %02X", LoRaParam->pLoRaWANcfg->OTA_AppKey[0]) ;for(int i=1; i<16; i++) {PRINTF(" %02X", LoRaParam->pLoRaWANcfg->OTA_AppKey[i]); }; PRINTF("\r\n");
  }
  else
  {
    PRINTF("[info] ABP\r\n"); 
	  //comtac correction with usage of LoraParam structure				
    PRINTF("[info] DevEui= %02X", LoRaParam->pLoRaWANcfg->OTA_DevEui[0]) ;for(int i=1; i<8 ; i++) {PRINTF(" %02X", LoRaParam->pLoRaWANcfg->OTA_DevEui[i]); }; PRINTF("\r\n");
    PRINTF("[info] DevAdd= %08X\r\n", LoRaParam->pLoRaWANcfg->ABP_DeviceAddr) ;
    PRINTF("[info] NwkSKey= %02X", LoRaParam->pLoRaWANcfg->ABP_NetworkSessionKey[0]) ;for(int i=1; i<16 ; i++) {PRINTF(" %02X", LoRaParam->pLoRaWANcfg->ABP_NetworkSessionKey[i]); }; PRINTF("\r\n");
    PRINTF("[info] AppSKey= %02X", LoRaParam->pLoRaWANcfg->ABP_ApplicationSessionKey[0]) ;for(int i=1; i<16 ; i++) {PRINTF(" %02X", LoRaParam->pLoRaWANcfg->ABP_ApplicationSessionKey[i]); }; PRINTF("\r\n");
  }  
}

/**
 *  lora class A state machine
 */

//------------------------------------------------------------------------------	 
/** comtac INFO
 *
 *  return type of lora_fsm function added to implement comtac's loraModem interface
 *  
 * 		
 *  Author: 2016-12-06-Ra  
 **/ 
//------------------------------------------------------------------------------	 
uint32_t lora_fsm( void)
{
  switch( DeviceState )
  {
    case DEVICE_STATE_INIT:
    {
      LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
      LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
      LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
	    //comtac correction with usage of LoraParam structure and LoraMacCfg structure	      
      LoraMacCfg.DefaultTxDatarate = LoRaParam->pLoRaWANcfg->ui8DefaultDatarate;
      LoraMacCfg.MaxTxDatarate = LoRaParam->pLoRaWANcfg->ui8MaxDatarate;
      LoraMacCfg.MinTxDatarate = LoRaParam->pLoRaWANcfg->ui8MinDatarate;
      LoraMacCfg.Rx2DefaultDatarate = LoRaParam->pLoRaWANcfg->ui8Rx2DefaultDatarate;

      LoRaMacCallbacks.GetBatteryLevel = LoRaMainCallbacks->BoardGetBatteryLevel; 
	    //comtac correction with the adding of two more callbacks to LoRaMacCallbacks
      LoRaMacCallbacks.TestSetRx1WindowsState = LoRaMainCallbacks->BoardSetRx1WindowsState;
      LoRaMacCallbacks.TestSetRx2WindowsState = LoRaMainCallbacks->BoardSetRx2WindowsState;
	    //comtac correction with the implementation of the new LoRaMacInitialization parameter, see LoRaMac.h file line 1525 	
      LoRaMacInitialization( &LoRaMacPrimitives, &LoRaMacCallbacks , &LoraMacCfg);

      TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );
			TimerInit( &JoinTrialTimer   , OnJoinTrialTimerEvent);				
				
		  //------------------------------------------------------------------------------	 
      /** comtac INFO
		   *
       *  code added for LinkActTimerEvent functionality (LED control)
		   *  
 		   *  switches LED LinkAct ON for each received downlink  
       * 		
       *  Author: 2016-12-06-Ra  
       **/ 			
      TimerInit( &LedStateTimer, OnLedStateTimerEvent );
      TimerSetValue( &LedStateTimer, 25 );
      
      TimerInit( &LedLinkActTimer, OnLedLinkActTimerEvent );
      TimerSetValue( &LedLinkActTimer, 25 );        
		  //------------------------------------------------------------------------------	         
				
      mibReq.Type = MIB_ADR;
      mibReq.Param.AdrEnable = LoRaParam->AdrEnable;
      LoRaMacMibSetRequestConfirm( &mibReq );
    	
      //When receiving, the SX1272 cant evaluate the SyncWord(SyncAddress)	
			//Only when sending it is used correctly! (2016-12-06-Ra) 
      mibReq.Type = MIB_PUBLIC_NETWORK;
      mibReq.Param.EnablePublicNetwork = LoRaParam->EnablePublicNetwork;
      LoRaMacMibSetRequestConfirm( &mibReq );
                        
      mibReq.Type = MIB_DEVICE_CLASS;
      mibReq.Param.Class= LoRaParam->Class;
      LoRaMacMibSetRequestConfirm( &mibReq );
        
      //------------------------------------------------------------------------------	 
      /** comtac INFO
		   *
       *  code added for LED functionality (LED control)
 		   *                	 
       *  Author: 2016-12-06-Ra  
       **/ 				
      if (LoRaMainCallbacks->BoardSetLedState){
        LoRaMainCallbacks->BoardSetLedState(false); // LED off
      }					
      if (LoRaMainCallbacks->BoardSetLedLink){
        LoRaMainCallbacks->BoardSetLedLink(false);  // LED off  
      } 					
      //------------------------------------------------------------------------------	 				


#if defined( USE_BAND_868 )
	    //comtac correction with usage of LoraParam structure				
      LoRaMacTestSetDutyCycleOn( LoRaParam->bDutyCycleOn );

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 ) 
			LoRaMacChannelAdd( 3, ( ChannelParams_t )LC4 );
			LoRaMacChannelAdd( 4, ( ChannelParams_t )LC5 );
			LoRaMacChannelAdd( 5, ( ChannelParams_t )LC6 );
			LoRaMacChannelAdd( 6, ( ChannelParams_t )LC7 );
			LoRaMacChannelAdd( 7, ( ChannelParams_t )LC8 );
			LoRaMacChannelAdd( 8, ( ChannelParams_t )LC9 );
			LoRaMacChannelAdd( 9, ( ChannelParams_t )LC10 );
#endif

#endif
      DeviceState = DEVICE_STATE_JOIN;
      //bTxOnEvent extension by comtac initialized here			
      u8LocalTxType = TX_NONE;		
      break;
    }
    case DEVICE_STATE_JOIN:
    {     
      //------------------------------------------------------------------------------	 
      /** comtac INFO
		   *
       *   correction with usage of LoraParam structure for OTA
 		   *                	 
       *  Author: 2016-12-06-Ra  
       **/ 			
      if (LoRaParam->pLoRaWANcfg->u1OTA)
      {
        MlmeReq_t mlmeReq;

        mlmeReq.Type = MLME_JOIN;

        mlmeReq.Req.Join.DevEui = LoRaParam->pLoRaWANcfg->OTA_DevEui;
        mlmeReq.Req.Join.AppEui = LoRaParam->pLoRaWANcfg->OTA_AppEui;
        mlmeReq.Req.Join.AppKey = LoRaParam->pLoRaWANcfg->OTA_AppKey;
      //------------------------------------------------------------------------------	 
			  /* clear ModemFlags before starting joining process */
				ModemFlags = NONE;
				
        if( NextTx == true )
        {
            LoRaMacMlmeRequest( &mlmeReq );
        }

        // Make state machine to not stop until we are joined or until a join trial number was reached
				TimerSetValue( &JoinTrialTimer , 12000); // wait 12 s until both RX windows are done, if no join RXERROR
        TimerStart   (&JoinTrialTimer);					
				DeviceState = DEVICE_STATE_WAIT;			
      }
      else
      {
        //------------------------------------------------------------------------------	 
        /** comtac INFO
		     *
         *   correction with usage of LoraParam structure for ABP
 		     *                	 
         *  Author: 2016-12-06-Ra  
         **/ 		
        mibReq.Type = MIB_NET_ID;
        mibReq.Param.NetID = LoRaParam->pLoRaWANcfg->ABP_NetworkID; // not used
        LoRaMacMibSetRequestConfirm( &mibReq );
        
        mibReq.Type = MIB_DEV_ADDR;
        mibReq.Param.DevAddr = LoRaParam->pLoRaWANcfg->ABP_DeviceAddr;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_NWK_SKEY;
        mibReq.Param.NwkSKey = LoRaParam->pLoRaWANcfg->ABP_NetworkSessionKey;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_APP_SKEY;
        mibReq.Param.AppSKey = LoRaParam->pLoRaWANcfg->ABP_ApplicationSessionKey;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_NETWORK_JOINED;
        mibReq.Param.IsNetworkJoined = true;
        LoRaMacMibSetRequestConfirm( &mibReq );
        //------------------------------------------------------------------------------	
       
			  /* when ABP we assume the joining was done correctly by the user */ 
			  ModemFlags   = JOINDONE; 
        DeviceState  = DEVICE_STATE_SEND;  					
        u8LocalTxType |= LoRaMainCallbacks->HasTxEvents();   				
      }		
      break;
    }
		
		case DEVICE_STATE_WAIT:
		{			  
		  //do nothing, we wait for the joining event or for the tx on timer event
		  break;
		}
		
    case DEVICE_STATE_JOINED:
    {
			//set Join flag and clear JoinError flag
		  ModemFlags = JOINDONE;
      PRINTF("[info] Device JOINED\r\n");
			TimerStop( &JoinTrialTimer);		
      DeviceState  = DEVICE_STATE_SEND;
      u8LocalTxType |= LoRaMainCallbacks->HasTxEvents();  			
      break;
    }
    case DEVICE_STATE_SEND:
    {
      if( NextTx == true )
      {
			   	/* clear old TX/RX flags, joined stays */
			    ModemFlags &= (JOINDONE | JOINERROR);
          //u8LocalTxType extension by comtac passed here		  
          PrepareTxFrame( u8LocalTxType );
          NextTx = SendFrame( );		
          // here we reset the event flags					
					u8LocalTxType  =	TX_NONE;
        
      }
			
      if( ComplianceTest.Running == true )
      {
          // Schedule next packet transmission as soon as possible			
					TimerSetValue( &TxNextPacketTimer, 5000 );
          TimerStart( &TxNextPacketTimer );		
          DeviceState = DEVICE_STATE_WAIT;
      }
      else
      {
        // Schedule next packet transmission        
        if (LoRaParam->pLoRaWANcfg->ui16SendInterval_m){   
				  // Usage of TxDutyCycleTime defined by comtac here, this is the functions return						
          TxDutyCycleTime = (LoRaParam->pLoRaWANcfg->ui16SendInterval_m*60000) + randr( -LoRaParam->DutyCycleRandomTime, LoRaParam->DutyCycleRandomTime ) - LoRaParam->DutyCycleSensorCaptureTime;        
				} else {
				  // Usage of TxDutyCycleTime defined by comtac here, this is the functions return						
          TxDutyCycleTime = 0;
				}				
        DeviceState = DEVICE_STATE_CYCLE;
      }
      break;
    }	
    case DEVICE_STATE_CYCLE:
    //------------------------------------------------------------------------------	 
    /** comtac INFO
		 *
     *   This is comtac's own implementation of the state Cycle as defined in lora.h
 		 *                	 
     *  Author: 2016-12-06-Ra  
     **/ 		
    //------------------------------------------------------------------------------	 		
    {                             
        // Schedule next packet transmission, if defined, clear TX on Timer flag
				// Usage of TxDutyCycleTime defined by comtac here, this is the functions return				
        if (TxDutyCycleTime)
        {   				
          TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
          TimerStart( &TxNextPacketTimer );				
        }
   			DeviceState  =  DEVICE_STATE_SLEEP;	
        break;				
    }    
    case DEVICE_STATE_SLEEP:
    {
      //------------------------------------------------------------------------------	 
      /** comtac INFO
		   *
       *   This is comtac's own implementation of the Wake up through events
 		   *                	 
       *  Author: 2016-12-06-Ra  
       **/	
      u8LocalTxType |=  LoRaMainCallbacks->HasTxEvents();      
      //------------------------------------------------------------------------------					
      if (u8LocalTxType && !ComplianceTest.Running && NextTx)
        OnSendEvent();
      break;
    }
    default:
    {
      DeviceState = DEVICE_STATE_INIT;
      break;
    }
  }
	return TxDutyCycleTime;
}

DeviceState_t lora_getDeviceState( void )
{
  return DeviceState;
}

LoRaMacStatus_t lora_getMacStatus(void)
{
	
	McpsReq_t mcpsReq;
	mcpsReq.Type = MCPS_MULTICAST;
	MacStatus = LoRaMacMcpsRequest(&mcpsReq);
	
	if (MacStatus != LORAMAC_STATUS_BUSY){
		MacStatus = LORAMAC_STATUS_OK;
	}
	
  return MacStatus;
}

bool lora_getNextTx(void)
{
  return NextTx;
}	

uint8_t lora_getModemFlags (void)
{
  return ModemFlags;
}	

int16_t lora_getLastRssi( void)
{
  return AppData.Rssi;
}

uint8_t lora_getLastSnr( void)
{
  return AppData.Snr;
}


/*****END OF FILE****/

