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
* @file      loraModem.c
* @date      2016-12-05
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     comtac's Lora Modem implementation 
*            
* VERSION:   
* 
* V0.01            2016-12-05-Ra      Create File 	
* V0.02            2016-12-27-Ra      Merged with loraModem concept of sensile		
*
*============================================================================*/
/* Includes -----------------------------------------------------------------------*/
#include <stdbool.h>   
#include "hw.h"
#include "radio.h"
#include "sx1272.h"
#include "loraModem.h"
#include "LoRaMacTest.h"
#include "low_power.h"
#include "timeServer.h"
#include "bsp.h"
#include "vcom.h"
#include "version.h"
#include "config.h" 	
#include "app.h" 	

/* Private typedef ----------------------------------------------------------------*/
/* Private define -----------------------------------------------------------------*/
/* Private macro ------------------------------------------------------------------*/
/* Private function prototypes ----------------------------------------------------*/
/* call back when LoRa Rx1 windows is open*/

#if defined (DEF_TIMINGTEST)
static void TestRx1WindowsState(bool bState);

/* call back when LoRa Rx2 windows is open*/
static void TestRx2WindowsState(bool bState);
#endif 
/* call back when LoRa message transmit is ongoing*/
static void SetLedState (bool bState);

/* call back when LoRa received a message*/
static void SetLedLink (bool bState);

/* call back when LoRa will transmit a frame*/
static void LoraTxData( lora_AppData_t *AppData, FunctionalState* IsTxConfirmed, uint8_t u8OnEvent);

/* call back when LoRa has received a frame*/
static void LoraRxData( lora_AppData_t *AppData);

/* call back for event check in LoRa Modem */
static uint8_t LoraHasTxEvents(void);

/* Private variables --------------------------------------------------------------*/     
/* load callbacks*/
static LoRaMainCallback_t sLoRaMainCallbacks = {HW_GetBatteryLevel,
                                                HW_GetUniqueId,
                                                HW_GetRandomSeed,
                                                NULL, //no TestRx1WindowsState function defined
                                                NULL, //no TestRx2WindowsState function defined  
                                                NULL, //no SetLedState function defined
                                                NULL, //no SetLedLink function defined
                                                LoraTxData,
                                                LoraRxData,
                                                LoraHasTxEvents};  																							 
/* Initialises the global Lora Parameters */
LoRaParam_t sLoRaParamInit = {
  .Class                      = CLASS_A,                 
  .AdrEnable                  = LORAWAN_ADR_ON,
  .EnablePublicNetwork        = LORAWAN_PUBLIC_NETWORK,
  .bDutyCycleOn               = LORAWAN_DUTYCYCLE_ON, 
  .DutyCycleRandomTime        = APP_TX_DUTYCYCLE_RND,     
  .DutyCycleSensorCaptureTime = APP_DATA_CAPTURE_TIME,
  .pLoRaWANcfg                = &sCfg.sLoRaWANCfg};
                                                 
/* loraModem variables */
static uint32_t u32TxDutyTimer = 0;
static uint8_t  u8LocalTxType = TX_NONE;
static bool     bTxMessageSet = false;     
static bool     bLoRaModemIsInitialized = false;
																							 
/* Communication variables */
static uint8_t  u8TxPort     = LORAWAN_DEF_PORT;
static uint8_t  u8RxPort     = LORAWAN_DEF_PORT; //APP RX PORT SAME AS TX

/* TX/RX Done timer */
static TimerEvent_t TxRxDoneTimer;
static bool bModemLocked = false; 																							 

																		 
/* Timer callback functions -------------------------------------------------------*/
static void OnTxRxDoneTimerEvent( void )
{ 	
  TimerStop( &TxRxDoneTimer );
	uint8_t u8ModemFlags = lora_getModemFlags();

  if (u8ModemFlags & (RX2TIMEOUT|RXDONE|RXERROR|TXTIMEOUT)){
	  SetShowLED(true);		
    bModemLocked = false;
#if defined (DEF_RF_ON_OFF)		
    /* turn off Module after TxRx, ReInit OFF (only for class A) */		
		if (!IsDeviceClassC()){
	    LoraOFF(false);
		}	
#endif	
		PRINTF("[info] MODEMFLAGS: 0x%02X\r\n",u8ModemFlags);
		PRINTF("[info] LoRa TX/RX UNLOCK\r\n");
  }else {
		if (u8ModemFlags & JOINERROR){
			SetShowLED(true);
			//if a join error happened, we change the check timer to 30 seconds (to save battery)
		  TimerSetValue( &TxRxDoneTimer, LORAMODEM_JOIN_CHECK_TIME * 1000);
			PRINTF("[info] LoRa NOT JOINED, BLOCK\r\n");
		}
		TimerStart( &TxRxDoneTimer );
		bModemLocked = true;
  }
}	

/* IRQ callback functions ---------------------------------------------------------*/																		 
/* Global functions ---------------------------------------------------------------*/
/**
  * @brief  Returns device class
  * @retval true if Class C and false if Class A
  */
bool IsDeviceClassC(void)
{
  return (sLoRaParamInit.Class == CLASS_C) ? true : false;
}

/**
  * @brief  Set default TX duty time and ABP for LoRa
  */
void LoraSetConfigDefault(void)
{  
  // lora default keys and datarates	
	lora_DefaultConfig(sLoRaParamInit.pLoRaWANcfg);   
  // default send interval
  sLoRaParamInit.pLoRaWANcfg->ui16SendInterval_m = LORAWAN_TX_DUTYCYCLE/60000;
	// default ABP
  sLoRaParamInit.pLoRaWANcfg->u1OTA = 0;		
}

/**
  * @brief  Tx/RxPort setter 
  * @param  new value for Tx/RxPorts
  * @retval none		
  */
void LoraSetPorts(uint8_t TxPort, uint8_t RxPort)
{
  assert_param(TxPort != 224);
	assert_param(TxPort != 0);
	assert_param(RxPort != 224);	
	assert_param(RxPort != 0); 
	
	if ((TxPort != 224) && (TxPort != 0)){ 
    u8TxPort = TxPort;
	}
	if ((RxPort != 224) && (RxPort != 0)){ 	
    u8RxPort = RxPort;
  }		
}

/**
  * @brief  Returns actual RxPort
  * @param  none
  * @retval RxPort	
  */
uint8_t LoraGetRxPort(void)
{
  return u8RxPort;
}

/**
  * @brief  Returns actual TxPort
  * @param  none
  * @retval TxPort	
  */
uint8_t LoraGetTxPort(void)
{
  return u8TxPort;
}

/**
  * @brief  turns on Radio Module and initializes modem
  * @param  none
  * @retval none
  */
void LoraON( void )
{
	if (!GetLoraPower()){
    SetLoraPower(true);
	}	
  LoraInit();	
}

/**
  * @brief  turns off radio module
  * @param  LoraDeInit --> reinitialize LoraMac parameters YES / NO
  * @retval none	
  */
void LoraOFF( bool LoraDeInit )
{
	if (GetLoraPower()){
	  SetLoraPower(false);
	}
  if (LoraDeInit){
    bLoRaModemIsInitialized = false;		
  }  
}

// 2018-07-04-Kd DANI
uint8_t LoraGetMaxPayloadSize( bool bConfirmedTx )
{
  return  LoRaMacQueryMaxPayloadSize( bConfirmedTx );
}


/**
  * @brief  Inits Lora modem application
  */
void LoraInit(void)
{
  if (!bLoRaModemIsInitialized){ 			
    /* Library Version */
    PRINTF("[info] LoRaMac Version: %X\r\n", VERSION_LORA);
    /* Configure the Lora Stack*/		
    lora_Init( &sLoRaMainCallbacks, &sLoRaParamInit); 
		/* Initialize TxRxDoneTimer */
		TimerInit( &TxRxDoneTimer, OnTxRxDoneTimerEvent ); 
		TimerSetValue(&TxRxDoneTimer, LORAMODEM_TXRX_CHECK_TIME * 1000); //must be in ms
  
    bLoRaModemIsInitialized = true;
  } else {
    SX1272ReInit();
		
		srand1(SX1272Random());
		
		if( sCfg.sLoRaWANCfg.u1PrivateNetwork == false )
    {
			// Change LoRa modem SyncWord to public
			SX1272SetSyncWord( LORA_MAC_PUBLIC_SYNCWORD );
    }
    else
    {
			// Change LoRa modem SyncWord to private
			SX1272SetSyncWord( LORA_MAC_PRIVATE_SYNCWORD );
    }
		SX1272SetSleep();
	}		
}

/**
  * @brief  sets a flag to activate a TX event, clears TxType flag 
  */
void LoraTxMessageSet(void){
    bTxMessageSet = true;
		/* here save a local copy of the TxType and clear the external flag */
	  u8LocalTxType = GetTxType();
	  SetTxType(TX_NONE);	
}

/**
  * @brief  Function used to start a LoraTxRx event (Blocking). The same can also be done by combining LoraTxMessageSet and LoraModem_Do
  * @param  none
  * @retval MessageStatus_t as defined in loraModem.h		
  */
MessageStatus_t LoraTxRxStart(void)
{	
	/* return variable */
  MessageStatus_t StatusRet = TX_RX_START_DONE;
	
	/* local NextTx and MacStatus variables (defined only for debug purpouses) */
	bool bNextTx = true;
	LoRaMacStatus_t tMacStatus = LORAMAC_STATUS_OK; 
	
	/* if modem not initialized yet, initialize here */
  if (!bLoRaModemIsInitialized){	
    LoraInit();
	}
	
	bNextTx = lora_getNextTx();
	tMacStatus = lora_getMacStatus();
	
	if ((bNextTx) && (tMacStatus == LORAMAC_STATUS_OK)){
	  /* here we set a TX Type */
	  if (GetTxType()){
      LoraTxMessageSet();
	  }	
		
    PRINTF("[info] LoRa TX/RX LOCK\r\n");		  		
    LoraDo();
    TimerStart(&TxRxDoneTimer);
	  bModemLocked = true;	
//	  while (!(ModemFlags & (RX2TIMEOUT|RXAPPDONE|RXDONE|RXERROR))){ 
//	  BACKUP_PRIMASK();
//    DISABLE_IRQ( );
//#ifndef LOW_POWER_DISABLE			
//		LowPower_Handler( );
//#endif			
//    RESTORE_PRIMASK();	
//  }
//  HAL_Delay(10);
//  PRINTF("[info] LoRa TX/RX UNLOCK\r\n"); 
  } else {
    StatusRet = TX_RX_START_ERROR;
    PRINTF("[info] LoRa TX/RX UNLOCK, START ERROR\r\n");		
  }		
	
	return StatusRet;	
}

void LoRaGetAndCalcRssiSnr( uint8_t *pui8LastRssi, int8_t *pi8LastSnr )
{
  /* Rssi of last received packet */
  int16_t i16Rssi = lora_getLastRssi();
  uint8_t ui8Snr  = lora_getLastSnr();
   
  if (pui8LastRssi)
  {
    if (i16Rssi < 0)    
    {
      if (i16Rssi >= -255)             
        *pui8LastRssi = (i16Rssi * -1); // 0..255 Rssi *-1 -> 0..-255[dB]
      else
        *pui8LastRssi = 255;            
    }
    else
      *pui8LastRssi = 0; 
  }
  if (pi8LastSnr)
  {
    /* Snr of last received packet in +-db    */
    if( ui8Snr & 0x80 ) // The SNR sign bit is 1
    {
      // Invert and divide by 4
      *pi8LastSnr  = ( ( ~ui8Snr + 1 ) & 0xFF ) >> 2;
      *pi8LastSnr  = -*pi8LastSnr ;
    }
    else                  
      *pi8LastSnr  = ( ui8Snr & 0xFF ) >> 2; // Divide by 4      
  }
}

/**
  * @brief  returns bModemLocked flag
  * @param  none
  * @retval bModemLocked: 0 UNLOCKED, 1 LOCKED 	
  */
bool LoraTxRxStatus(void)
{	
  return bModemLocked;
}	

/**
  * @brief  calls the lora_fsm until the device state equals sleep
  * @param  none
  * @retval none
  */
void LoraDo( void )
{ 
  /* here we let the lora Tx state machine work until it is set to sleep and no more rejoins are programmed */
  do
  {	  
	  DISABLE_IRQ();
    u32TxDutyTimer = lora_fsm( );
    ENABLE_IRQ();		
  }while (lora_getDeviceState( ) != DEVICE_STATE_SLEEP) ;	  
}  

/* Private functions ---------------------------------------------------------------*/
/* LoRa Callback functions ---------------------------------------------------------*/
/**
  * @brief  Used to check LoRa RX1 window timing by toggling a specific pin
  * @param  bState: state of the output
  */
/* call back when LoRa Rx1 windows is open (2016-11-04-Kd 4.998s Joining and 0.9983s Receiving with a 200ms test timer running) */
#if defined (DEF_TIMINGTEST)
static void TestRx1WindowsState(bool bState)
{  
  HAL_GPIO_WritePin( TP404_GPIO_Port, TP404_Pin, bState ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
#endif  

/**
  * @brief  Used to check LoRa RX2 window timing by toggling a specific pin
  * @param  bState: state of the output
  */
/* call back when LoRa Rx2 windows is open (2016-11-04-Kd 1.998s Receiving with a 200ms test timer running) */
#if defined (DEF_TIMINGTEST) 
static void TestRx2WindowsState(bool bState)
{  
  HAL_GPIO_WritePin( TP405_GPIO_Port, TP405_Pin, bState ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
#endif 

/**
  * @brief  Use to set the State LED when a message is ongoing
  * @param  bState: state of the output
  */
/* call back when LoRa message transmit is ongoing*/
static void SetLedState (bool bState)
{ 
  SetLEDGreen(bState);
}

/**
  * @brief  Use to set the Link LED when a message was received
  * @param  bState: state of the output
  */
/* call back when LoRa received a message*/
static void SetLedLink (bool bState)
{
  SetLEDRed(bState);
} 

/**
  * @brief  LoraTx payload creation application --> in our case measurements and so on.
  * @param  AppData: LoRa app data, payload must be defined here
  *         IsTxConfirmed: TX confirm or unconfirm
  *         bOnEvent: message happens because of event
  */
static void LoraTxData( lora_AppData_t *AppData, FunctionalState* IsTxConfirmed, uint8_t u8OnEvent)
{
  appBuildTxPayload(AppData,IsTxConfirmed, u8OnEvent);
}

/**
  * @brief  LoraRx payload application
  *         
  * @param  AppData: LoRa app data, payload must be read from here
  */
static void LoraRxData( lora_AppData_t *AppData )
{
  appParseRxPayloadIsr(AppData);  
}	

/**
  * @brief  Passes the TX_ON_EVENT to the main lora_fsm in lora.c
  */
static uint8_t LoraHasTxEvents(void)
{
	uint8_t u8TxEvents = 0x00;
	if (bTxMessageSet){
		u8TxEvents = u8LocalTxType;
	  bTxMessageSet = false;
		u8LocalTxType = TX_NONE;
	}
	
	return u8TxEvents;	
}	

/*****END OF FILE****/
