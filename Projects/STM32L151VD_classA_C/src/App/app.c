/**============================================================================
* @file      app.c
* @date      2017-09-01
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     main file of SenseTwoMaster
*            
* VERSION:   
* 
* V0.01            2017-09-01-Ra      Create File 		
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "low_power.h"
#include "bsp.h"
#include "meas.h"
#include "timeServer.h"
#include "fatfs.h"
#include "eeprom.h"
#include "disk.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "vcom.h"
#include "loraModem.h"
#include "log.h"
#include "com.h"
#include "bt.h"
#include "config.h"
#include "Si705x.h"
#include "rtcApp.h"
#include "app.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum {
  eAppLoRaDownlinkCmd_SetTxInterval = 1,
  eAppLoRaDownlinkCmd_SetRTC,
} teAppLoRaDownlinkCmd;

typedef struct{
  volatile bool       bDataRx;
  uint8_t             aRxBuf[APP_RX_BUF_SIZE];
  lora_AppData_t      sLoRaAppDataRx;  
} tTFB_LoRaRx;

/* Private define ------------------------------------------------------------*/

// #define USE_IMST_ExpLoRaStudio  // Max. Payloadlenght = 127 (Tool hat Fehler)

//LED Blinking times defines
#define LED_OK_BLINK        100
#define LED_NOK_BLINK       25
#define LED_LONG_BLINK      1000

//IWDG refresh time
#define IWDG_REFRESH_TIME   (uint32_t)10000  // 10 secs


/* Private macro -------------------------------------------------------------*/
typedef enum {
  eUplink_GrpID_TEMPERATURE = 0,
  eUplink_GrpID_VOLTAGE,
  eUplink_GrpID_CURRENT,
  eUplink_GrpID_CORRVOLTAGE,
  eUplink_GrpID_CORRCURRENT,  
  eUplink_GrpID_IMPEDANCE,
  eUplink_GrpID_IMPEDANCEPHASE,
  eUplink_GrpID_RESISTANCE,
  eUplink_GrpID_VOLTAGEHIGHRES,
  eUplink_GrpID_Cnt
} t_eUplink_GrpID;

/* Private function prototypes -----------------------------------------------*/
static void LEDLoRaCtrlFSM( void );
static uint32_t CalculateTxInterval(void);
#if defined DEF_USB	
static void InitUSB(void);
static void DeinitUSB(void);

uint8_t  ui8USB_Class_CDC; // CDC oder MSC Class (Default MSC)
uint16_t ui16UsbCdcRxBuffLen;
uint8_t* pui8UsbCdcRxBuffAddr;
#endif

/* Private variables ---------------------------------------------------------*/
uint8_t ui8ResetValue __attribute__((section("NoInit"), zero_init));

bool        bColdJunctionTempErr, bExtAdcErr;

bool        bBtnReset;
bool        bLoggingOk;
bool        bAppMeasIntervall;
char        msg_str[200];
tTFB_LoRaRx sTFB_LoRaRx;
uint16_t    u16BatteryVDDmV;  
int16_t     i16Temperature_GC_100 = 2500;

volatile static bool bIWDG_Refresh = false;

/*!
 * Timer to handle the delay of USB
 */
#if defined DEF_USB	 
static TimerEvent_t USBTimer;
volatile static bool bUSBTimerOver = false;
#endif
/*!
 * Timer to handle applications send interval
 */
static TimerEvent_t TxIntervalTimer;
/*!
 * Timer to control LED blue and orange status display
 */
static TimerEvent_t LEDLoRaDisplayTimer;
/*!
 * Timer to handle the delay of
 */
// installs a 200ms timer for timer accuracy tests
#if defined (DEF_TIMINGTEST)  
static TimerEvent_t TestTimer;
volatile static bool bTest;
#endif

/* Timer used for IWDG refresh */
static TimerEvent_t RefreshIWDGTimer;

/* variable to detect if USB was already defined */
bool     bUSBinitialized = false;

/* statics for LED control */
static tLEDFSMStates eLEDLoraFSMState     = LED_SLEEP;
static tLEDFSMStates eLEDLoraFSMStateLast = LED_SLEEP;

/* Global functions ---------------------------------------------------------*/
/* Timer callback functions ---------------------------------------------------------*/
#if defined DEF_USB	
static void OnUSBTimerEvent( void )
{
   TimerStop( &USBTimer );
   bUSBTimerOver = true;
}
#endif

static void OnLEDLoRaDisplayTimerEvent( void )
{
   TimerStop( &LEDLoRaDisplayTimer );
   eLEDLoraFSMState = LED_PROCESS;			
}

static void RestartTxInterval( void )
{
  TimerStop( &TxIntervalTimer );
  /* reset and start AppTxIntervalTimer after TxRxStart */
  if (sCfg.sAppCfg.u32AppTxInterval_m){
    TimerSetValue( &TxIntervalTimer, CalculateTxInterval()); //value must be in ms 	
    TimerStart( &TxIntervalTimer);
  }    
}

static void OnTxIntervalTimerEvent( void )
{
  
  bAppMeasIntervall = true;	
  RestartTxInterval();
}

#if defined (DEF_TIMINGTEST)  
static void OnTestTimerEvent( void )
{
   TimerStop( &TestTimer );
   bTest = true;  
}
#endif

static void OnRefreshIWDGTimerEvent( void )
{
   /* Refresh IWDG */
   bIWDG_Refresh = true; // 2018-08-23-Kd ab V0.04 IWDG-Refresh von Timer IRQ in the appDo() verschoben 
}

/* IRQ callback functions ---------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  default values for app
  */
void appSetDefault( void )
{	
  uint8_t u8Ch;
  // default definition LoRa
  LoraSetConfigDefault();	
  // Application's default Max and Min TxDatarate set with APP_MAX_DATARATE/APP_MIN_DATARATE
  sCfg.sLoRaWANCfg.ui8MaxDatarate        = APP_MAX_DATARATE;	
  sCfg.sLoRaWANCfg.ui8MinDatarate        = APP_MIN_DATARATE;	 
  // Application's default datarate set with APP_DEFAULT_DATARATE
  sCfg.sLoRaWANCfg.ui8DefaultDatarate    = APP_DEFAULT_DATARATE; 
  // Application's default RX2 datarate set with APP_DEFAULT_DATARATE
  sCfg.sLoRaWANCfg.ui8Rx2DefaultDatarate = APP_DEFAULT_RX2_DATARATE; 	
  // Application definition for TX and RX Ports
  LoraSetPorts(LORAWAN_DEF_PORT, APP_RX_RECEIVED_PORT);

  // default Application
  sCfg.sAppCfg.u32AppTxInterval_m = APP_DEF_TX_INTERVAL;	
  sCfg.sAppCfg.u32AppMeasInterval_m = APP_DEF_TX_INTERVAL;
  for (u8Ch=0; u8Ch<EXT_SENSE_CHANNELS; u8Ch++)
    sCfg.sAppCfg.au8TemperatureSensorType[u8Ch] = eMEAS_EXTSENSETEMPTYPE_PT1000_3Wire;
  sCfg.sAppCfg.u8ChMskTemperature = 0xFF;
  sCfg.sAppCfg.u8ChMskVoltage = 0xFF;
  sCfg.sAppCfg.u8ChMskCurrent = 0xFF;
  sCfg.sAppCfg.u8ChMskCorrVoltage = 0xFF;
  sCfg.sAppCfg.u8ChMskCorrCurrent = 0xFF;
  sCfg.sAppCfg.u8ChMskImpedance = 0xFF;
  sCfg.sAppCfg.u8ChMskPhaseAngle = 0xFF; // 2018-10-03-Kd new V1.0		
  sCfg.sAppCfg.u8ChMskResistance = 0x00; // 2018-10-03-Kd new V1.0		
}

void appBtnResetCallback(void)
{
  bBtnReset = true;
}

/**
* @brief  Initializes application;  ATTENTION : Switch off both LED's at the end of appInit or LEDLoRaCtrlFSM() will have problems
  */
void appInit(void)
{
  bool              bPOR = false; // PowerOnReset
  uint8_t           ui8cnt, ui8cntBtn1, ui8cntBtn2;  
  
  PRINTF("[info] APP INIT\r\n");	

  if (!__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    ui8ResetValue = 0;
  bPOR = __HAL_RCC_GET_FLAG(RCC_FLAG_PORRST);
  __HAL_RCC_CLEAR_RESET_FLAGS();
  
  App_RTC_Init(bPOR);     
  
  // All LEDs ON for LED Test
  SetLEDRed(true);   // LED on
  SetLEDGreen(true);  // LED on  
  HAL_Delay(ui8ResetValue ? 200 : 500); 
  // Button 1 still pressed (reset action) -> wait until released
  while (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_PIN))
  {
      SetLEDRed(false); 
      SetLEDGreen(false); 
      HAL_Delay(100);
      SetLEDRed(true); 
      SetLEDGreen(true); 
      HAL_Delay(100);       
  }
  
  ui8USB_Class_CDC = 1; // 2018-10-04-Kd V1.00 new default USB CDC -> COM Port 
  ui8cntBtn1=ui8cntBtn2=0;
  if (!ui8ResetValue)
  {
    // Format EEPROM Disk or somthing else ?
    // give 2s time to press buttons
    for (ui8cnt=0;ui8cnt<10;ui8cnt++)
    {
      SetLEDRed(true);   // LED on
      SetLEDGreen(false); // LED off
      HAL_Delay(100);
      SetLEDRed(false);  // LED off
      SetLEDGreen(true);  // LED on
      HAL_Delay(100);     
      if (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_PIN))
        ui8cntBtn1++;
      if (HAL_GPIO_ReadPin(BOOT_CUSTOM_GPIO_Port, BOOT_CUSTOM_Pin))
        ui8cntBtn2++;    
      if (ui8cntBtn1 && ui8cntBtn2)           
        break; // both button pressed
      else if ((ui8cntBtn1 > 2) || (ui8cntBtn2 > 2))
        break; // single button pressed for > 400ms 
    }
  }
  SetLEDRed(false);
  SetLEDGreen(false);
  
  u16BatteryVDDmV  = MEASGetUbat();  
  if (ui8cntBtn1 || ui8cntBtn2)
  {
    // Show reaction for pressing
    for (ui8cnt=0;ui8cnt<10;ui8cnt++)
    {
      SetLEDGreen(false); 
      HAL_Delay(50);
      SetLEDGreen(true);
      HAL_Delay(50);    
    } 
    SetLEDGreen(false); 
    if (ui8cntBtn1 && ui8cntBtn2)    
      ZeroDisk(); // < 8192/4 * 3.3ms <= 6.7s     
    else if (ui8cntBtn1)      
      ui8USB_Class_CDC = 0; // USB MSC -> Mass Storage Device    
  }  
  
  if (ui8USB_Class_CDC) // USB CDC -> COM Port
  {
// 2020-06-15-Kd V1.08 auf Wunsch Herr Kronenberg entfernen    
//    if (!ui8ResetValue && u16BatteryVDDmV < 1500)
//      bCom_SendMeasData = true; // Speisung per USB -> Messdaten verschicken falls DTR gesetzt ist    
    pui8UsbCdcRxBuffAddr = CDC_GetRxBufAddr();  // Get Address of the buffer    
  } 
  ui8ResetValue = 0;  
        
  /* EEPROM CHECK */
    /* Get the config data */
  CfgInit();  
  
  if (!ui8cntBtn1 && ui8cntBtn2)
    sLoRaParamInit.bDutyCycleOn = false; // When  SW-Update -> no DutyCycle control
  
	 /* Format EEPROM Disk if both buttons pressed (HWREV01 and 02) */
   if ( HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_PIN) && 
        HAL_GPIO_ReadPin(BOOT_CUSTOM_GPIO_Port, BOOT_CUSTOM_Pin) ){
      ZeroDisk(); // < 8192/4 * 3.3ms <= 6.7s  			 
	 }
				 
	 /* Read CFG.txt */		 
   MX_FATFS_Init();	 
	 
   /* LoRa parameter config */
   sLoRaParamInit.EnablePublicNetwork = sCfg.sLoRaWANCfg.u1PrivateNetwork ?  false : true;  
   sLoRaParamInit.AdrEnable = sCfg.sLoRaWANCfg.u1ADR ? true : false; 
  
	 /* Redundant parameter check */
   sCfg.sLoRaWANCfg.ui16SendInterval_m = 0;   
    
   BT_Init();   
   
   /* Initialize LoraModem */
   LoraInit();
	
   /* Initialize application interval timer*/
   TimerInit( &TxIntervalTimer, OnTxIntervalTimerEvent);
  
  /* Initialize USB Timer */
#if defined DEF_USB				
   TimerInit( &USBTimer, OnUSBTimerEvent );
	  /* If USB connected, InitUSB right away */
   if ( HAL_GPIO_ReadPin(EXT_5V_USB_DETECT_GPIO_Port, EXT_5V_USB_DETECT_Pin)){		
      TimerStop( &USBTimer );
      bUSBTimerOver = true;				
   } else {			
      /* Else, just start USBTimer */	
      TimerStop( &USBTimer );
      TimerSetValue( &USBTimer, 3000 ); // every 3 secs
      TimerStart ( &USBTimer );
   }
#endif			
		
	/* Initialize LED control timers */
	TimerInit( &LEDLoRaDisplayTimer, OnLEDLoRaDisplayTimerEvent);   
  

#if defined (DEF_TIMINGTEST)    
   TimerInit( &TestTimer, OnTestTimerEvent );
   TimerSetValue( &TestTimer, 200 );
   TimerStart( &TestTimer ); 
#endif

  /* Init RefreshIWDGTimer and start it right away */
	TimerInit( &RefreshIWDGTimer, OnRefreshIWDGTimerEvent );
	TimerStop ( &RefreshIWDGTimer );
	TimerSetValue(&RefreshIWDGTimer, IWDG_REFRESH_TIME);
	TimerStart(&RefreshIWDGTimer);		

  HW_DAC_FillSinusTable();
   
	/* Start IWDG */
#ifndef DEBUG	   
	HW_IWDG_Init();	  
#endif
  
	/* Initialize buttons & ext.con interrupts */
	BSP_ButtonIrqInit( appBtnResetCallback);	

  bLoggingOk = Log_Init(bPOR); 
  
   /* TX_ON_INIT */		
   LoraON();	
   LoraTxRxStart();

   /* set and start AppTxIntervalTimer after TxRxStart */
   RestartTxInterval();      
}

static bool App_UsbCdc_SendData(uint8_t *pu8TxBuf, uint16_t ui16Len, bool bCheckDTR)
{
  bool      bOk = false;
  uint8_t   u8Status;
  uint32_t  tickstart = 0;
  
  if (bUSBinitialized && ui8USB_Class_CDC)
  {  
    tickstart = HAL_GetTick();    
    do {
      u8Status = CDC_Transmit_FS((uint8_t*)pu8TxBuf, ui16Len, bCheckDTR);
      
      if (u8Status == USBD_OK)  
      {
        bOk = true;
        break;
      }               
    } while ((u8Status == USBD_BUSY) && ((HAL_GetTick() - tickstart) < 500));
  }
  
  return bOk;
}
 
static void App_UsbCdcProt(uint8_t* pui8RxBuffAddr, uint16_t ui16RxBuffLen)
{
static uint16_t ui16RxBuffLenLast = 0; 
static uint8_t  comTxBuffer[COM_TX_DATA_SIZE];  
  bool          bRequestTxProlong = false;
  uint16_t      ui16TxBuffLen;
  
  while ((ui16RxBuffLen && (ui16RxBuffLenLast != ui16RxBuffLen)) || bRequestTxProlong)
  {
    ui16TxBuffLen = 0;    
#if (USE_COM_ECHO_ON == 1)
    if (ui16RxBuffLen && (ui16RxBuffLenLast != ui16RxBuffLen))
    {
      App_UsbCdc_SendData(pui8RxBuffAddr+ui16RxBuffLenLast, ui16RxBuffLen-ui16RxBuffLenLast, false); // Tx Echo  
      ui16RxBuffLenLast = ui16RxBuffLen;
    }
#endif    
    if (ui16RxBuffLen && (pui8RxBuffAddr[ui16RxBuffLen-1] == '\n')) { // LF is the telegramm end
      if (pui8RxBuffAddr[0] == ':')             // : is the telegramm start -> discard
          ui16TxBuffLen = Com_HandleTelegramm( pui8RxBuffAddr+1, ui16RxBuffLen-1, comTxBuffer, &bRequestTxProlong);  
      CDC_ResetRxBuf();
      ui16RxBuffLen = 0;
      ui16RxBuffLenLast = 0;
    }
    else if (bRequestTxProlong)
      ui16TxBuffLen = Com_HandleTelegramm( NULL, 0, comTxBuffer, &bRequestTxProlong);
    
    if (ui16TxBuffLen)
      if (!App_UsbCdc_SendData( comTxBuffer, ui16TxBuffLen, false) && bRequestTxProlong) // Tx Answer  
        Com_HandleTelegramm( NULL, 1, NULL, &bRequestTxProlong); // Stop Cmd (RxLen > 0)  
          
    if (bRequestTxProlong)
      ui16RxBuffLen = CDC_GetRxBufLen();  
    else
      break; // 2018-09-03-Kd ab V0.04
  }
}

void App_TerminalSendMeasData(uint8_t *pu8TxBuf, uint16_t ui16Len, bool bCheckBT_CTS)
{
  static bool bSendBT = false;
    
  PRINTF((char*)pu8TxBuf);
  if (bCom_SendMeasData || bCom_GetMeasData)
  {
    if (bCheckBT_CTS)
      bSendBT = BT_GetUART_Cts();
    if (bSendBT)
      bSendBT = BT_SendData( pu8TxBuf, ui16Len);
    App_UsbCdc_SendData( pu8TxBuf, ui16Len, true); 
  }
}

void App_Measure(bool bLogging)
{    
   uint32_t u32ExtPowerOnTickstart_ms = 0;
   uint8_t  u8EnabledChMsk = 0xff; // 2020-06-26-Kd V1.09 new (only measure configured channels)
   uint8_t  u8EnabledTemperatureChMsk = 0xff; // 2020-06-26-Kd V1.09 new (only measure configured channels)
   uint8_t  u8EnabledVoltageChMsk = 0xff; // 2020-06-26-Kd V1.09 new (only measure configured channels)
   uint8_t  u8EnabledCurrentChMsk = 0xff; // 2020-06-26-Kd V1.09 new (only measure configured channels) 
   uint8_t  u8EnabledCorrVoltageChMsk = 0xff; // 2020-06-26-Kd V1.09 new (only measure configured channels)
   uint8_t  u8EnabledCorrCurrentChMsk = 0xff; // 2020-06-26-Kd V1.09 new (only measure configured channels)
  
   memset( &sMEASExtSenseData,0,sizeof(sMEASExtSenseData)); // 2020-06-26-Kd V1.09 new (only measure configured channels)
  
	//Switch all VCCs on according to the AppPeripheries 
   BSP_SystemVcc(true); 	    
   u32ExtPowerOnTickstart_ms = HAL_GetTick();// 2018-10-29-Kd V1.02 for ext. Sensor
  
   BSP_SleepDelayMs(APP_DATA_CAPTURE_TIME); // 0.1s sleep for the sensors to messure after switching on the sensors supply	  		
   // Read data from ADC
   u16BatteryVDDmV  = MEASGetUbat();   
    
   bColdJunctionTempErr = !MEASGetColdJunctionTemperature(&i16Temperature_GC_100);
   // Bis hier vergehen ca. 100ms
   
   if (!bCom_GetMeasData) // 2020-06-26-Kd V1.09 new (only measure configured channels)
      u8EnabledChMsk = sCfg.sAppCfg.u8ChMskImpedance | sCfg.sAppCfg.u8ChMskPhaseAngle | sCfg.sAppCfg.u8ChMskResistance;
   // Impedance (Resistance)
   if (u8EnabledChMsk) // 2020-06-26-Kd V1.09 new (only measure configured channels)
      MEASGetIntADCSensors(u8EnabledChMsk); // 0.22..2.65s a 40..50mA (2018-10-26-Kd ab V1.02 neu zuerst messen, damit ext. Sensoren mehr Vorlaufzeit haben)  
   
   // Temperature, Corr.U+I, Voltage + Current
   if (!bCom_GetMeasData) // 2020-06-26-Kd V1.09 new (only measure configured channels)
   { 
      u8EnabledTemperatureChMsk = sCfg.sAppCfg.u8ChMskTemperature; // 2020-06-26-Kd V1.09 new (only measure configured channels)
      u8EnabledVoltageChMsk = sCfg.sAppCfg.u8ChMskVoltage; // 2020-06-26-Kd V1.09 new (only measure configured channels)
      u8EnabledCurrentChMsk = sCfg.sAppCfg.u8ChMskCurrent; // 2020-06-26-Kd V1.09 new (only measure configured channels) 
      u8EnabledCorrVoltageChMsk = sCfg.sAppCfg.u8ChMskCorrVoltage; // 2020-06-26-Kd V1.09 new (only measure configured channels)
      u8EnabledCorrCurrentChMsk = sCfg.sAppCfg.u8ChMskCorrCurrent; // 2020-06-26-Kd V1.09 new (only measure configured channels)
   }
   if (u8EnabledTemperatureChMsk | u8EnabledVoltageChMsk | u8EnabledCurrentChMsk | u8EnabledCorrVoltageChMsk | u8EnabledCorrCurrentChMsk)
   {
      bExtAdcErr = !MEASGetExtADCSensors( sCfg.sAppCfg.au8TemperatureSensorType, i16Temperature_GC_100, u32ExtPowerOnTickstart_ms,
            u8EnabledTemperatureChMsk, u8EnabledVoltageChMsk, u8EnabledCurrentChMsk, u8EnabledCorrVoltageChMsk, u8EnabledCorrCurrentChMsk); // ca. 3s (ohne zusätzliche ext. Sensoren Vorlaufzeit) a 24mA 
   }
   else
      bExtAdcErr = false;
   
   if (bLogging)
     bLoggingOk = Log_Data(); // 25ms a 70mA

   if (bCom_GetMeasData)
   {
     Log_WriteDataLine(true);
     bCom_GetMeasData = false;
   }
   else
   {
     sprintf(msg_str, "Meassure data: battery [mV]=%d  temperature [degC]=%d.%d \r\n", u16BatteryVDDmV, i16Temperature_GC_100/100, abs(i16Temperature_GC_100%100));
     App_TerminalSendMeasData((uint8_t*)msg_str, strlen(msg_str), true);

     sprintf(msg_str, "Sensor data         :");
     for (int i=0; i<EXT_SENSE_CHANNELS; i++)
        sprintf(&msg_str[strlen(msg_str)], "      Ch%d", i+1);
     sprintf(&msg_str[strlen(msg_str)],"\r\n");
     App_TerminalSendMeasData((uint8_t*)msg_str, strlen(msg_str), false);
    
     sprintf(msg_str, "Temperature [degC]  :");
     for (int i=0; i<EXT_SENSE_CHANNELS; i++)
        sprintf(&msg_str[strlen(msg_str)], " %8.2f", sMEASExtSenseData.fTemp_gC[i]);
     sprintf(&msg_str[strlen(msg_str)],"\r\n");
     App_TerminalSendMeasData((uint8_t*)msg_str, strlen(msg_str), false);
       
     sprintf(msg_str, "Voltage [V]         :");
     for (int i=0; i<EXT_SENSE_CHANNELS; i++)
        sprintf(&msg_str[strlen(msg_str)], " %8.4f", sMEASExtSenseData.fU_V[i]);
     sprintf(&msg_str[strlen(msg_str)],"\r\n");
     App_TerminalSendMeasData((uint8_t*)msg_str, strlen(msg_str), false);
     
     sprintf(msg_str, "Current [mA]        :");
     for (int i=0; i<EXT_SENSE_CHANNELS; i++)
        sprintf(&msg_str[strlen(msg_str)], " %8.2f", sMEASExtSenseData.fI_mA[i]);
     sprintf(&msg_str[strlen(msg_str)],"\r\n");
     App_TerminalSendMeasData((uint8_t*)msg_str, strlen(msg_str), false);     

     sprintf(msg_str, "Corr. Potential [mV]:");
     for (int i=0; i<EXT_SENSE_CHANNELS; i++)
        sprintf(&msg_str[strlen(msg_str)], " %8.1f", sMEASExtSenseData.fCorrU_mV[i]);
     sprintf(&msg_str[strlen(msg_str)],"\r\n");
     App_TerminalSendMeasData((uint8_t*)msg_str, strlen(msg_str), false); 
     
     sprintf(msg_str, "Corr. Current [uA]  :");
     for (int i=0; i<EXT_SENSE_CHANNELS; i++)
        sprintf(&msg_str[strlen(msg_str)], " %8.1f", sMEASExtSenseData.fCorrI_uA[i]);
     sprintf(&msg_str[strlen(msg_str)],"\r\n");
     App_TerminalSendMeasData((uint8_t*)msg_str, strlen(msg_str), false);  

     sprintf(msg_str, "Impedance [Ohm]     :");
     for (int i=0; i<EXT_SENSE_CHANNELS; i++)
      sprintf(&msg_str[strlen(msg_str)], " %8.1f", sMEASExtSenseData.fImp_Ohm[i]);
     sprintf(&msg_str[strlen(msg_str)],"\r\n");
     App_TerminalSendMeasData((uint8_t*)msg_str, strlen(msg_str), false);
     
     sprintf(msg_str, "Resistance [Ohm]    :");
     for (int i=0; i<EXT_SENSE_CHANNELS; i++)
        sprintf(&msg_str[strlen(msg_str)], " %8.1f", sMEASExtSenseData.fImpActive_Ohm[i]);
     sprintf(&msg_str[strlen(msg_str)],"\r\n");
     App_TerminalSendMeasData((uint8_t*)msg_str, strlen(msg_str), false); 
   }
   
	//Turn off system power
   BSP_SystemVcc(false); 	
}

static bool App_HandleLoRaRxData(void)
{
  uint8_t               u8RxPos = 0;
  uint8_t               u8RxLen = sTFB_LoRaRx.sLoRaAppDataRx.BuffSize;
  uint8_t               *pRxBuff = sTFB_LoRaRx.aRxBuf;
  teAppLoRaDownlinkCmd  eAppLoRaDownlinkCmd;
  
  while (u8RxPos < u8RxLen)
  {
    eAppLoRaDownlinkCmd = (teAppLoRaDownlinkCmd)sTFB_LoRaRx.aRxBuf[u8RxPos++];
    if ((eAppLoRaDownlinkCmd == eAppLoRaDownlinkCmd_SetTxInterval) && (u8RxLen - u8RxPos >= 2))
    {
      uint16_t u16SendInterval = (uint16_t)(pRxBuff[u8RxPos])<<8 | (uint16_t)(pRxBuff[u8RxPos+1]);
      u8RxPos += 2;
      if (sCfg.sAppCfg.u32AppTxInterval_m != u16SendInterval)
      {
        if (u16SendInterval >= APP_MAX_DUTY_ON_DOWNLINK)
          sCfg.sAppCfg.u32AppTxInterval_m = APP_MAX_DUTY_ON_DOWNLINK;	
        else 
          sCfg.sAppCfg.u32AppTxInterval_m = u16SendInterval;
        RestartTxInterval();          
      }          
    }
    else if ((eAppLoRaDownlinkCmd == eAppLoRaDownlinkCmd_SetRTC) && (u8RxLen - u8RxPos >= 6))
    {
      // YYMMDDHHMMSS
      RTC_TimeTypeDef sTime = {0}; 
      RTC_DateTypeDef sDate = {0};   
      
      sDate.Year =  pRxBuff[u8RxPos++]; 
      sDate.Month = pRxBuff[u8RxPos++];       
      sDate.Date = pRxBuff[u8RxPos++];  
      sTime.Hours = pRxBuff[u8RxPos++];  
      sTime.Minutes = pRxBuff[u8RxPos++];  
      sTime.Seconds = pRxBuff[u8RxPos++];
      if (!App_RTC_SetDateTime(&sDate, &sTime))
        break;
    }
    else
      break;
  }
  if (memcmp(&sEEPROM.sApp.sAppCfg,&sCfg.sAppCfg,sizeof(tAppCfg))){
    memcpy(&sEEPROM.sApp.sAppCfg,&sCfg.sAppCfg,sizeof(tAppCfg));
    EEPROMWriteAppConfig();
    EEPROMReadAppConfig();  

    //now we rewrite the CFG.txt file to have these changes saved somewhere!
    ReWriteConfigFileData();    
  }	       
       
  //do some printfs
  PRINTF("[info] RX APP \r\n");	
  PRINTF("[info] Rx Payload size: %d\r\n", sTFB_LoRaRx.sLoRaAppDataRx.BuffSize);
    
  return true;
}

void appHandleIWDG_Refresh(void)
{
  if (bIWDG_Refresh)
  {
     // bIWDG_Refresh will be set from RTC timer, so we have also checked the IRQ - RTC system
     /* Reset Timer */
     TimerStop( &RefreshIWDGTimer );
     bIWDG_Refresh = false;
     TimerSetValue( &RefreshIWDGTimer, IWDG_REFRESH_TIME);
     TimerStart( &RefreshIWDGTimer );  
     
  #ifndef DEBUG	  
     HAL_IWDG_Refresh(&hiwdg);
  #else
     SetLEDGreen(true);
     HAL_Delay(10);
     SetLEDGreen(false);	 
  #endif      
  }
}

/**
  * @brief  was any callback called? 
  * @retval true if there is something to do, false if not
  */
bool appHasWork(void)
{
  bool bHasWork = false;
  
  bHasWork = ((GetTxType() && !LoraTxRxStatus() && !GetShowLED()) || GetShowLED() || BT_HasWork() || bBtnReset || bAppMeasIntervall || sTFB_LoRaRx.bDataRx || bIWDG_Refresh);
	
#if defined (DEF_TIMINGTEST)    	
  if (bTest)
    bHasWork = true;
  bHasWork =(bTest ||  (GetTxType() && !LoraTxRxStatus() && !GetShowLED()) || bUSBTimerOver || GetShowLED() || BT_HasWork());
#endif	

#if defined (DEF_USB)
    if (bUSBTimerOver || (bUSBinitialized && ui8USB_Class_CDC && CDC_GetRxBufLen()))
      bHasWork = true;
#endif  	

  return bHasWork;
}

void appHandleUI(void)
{
  // Resetbutton
  if (bBtnReset) {
    NVIC_SystemReset(); 
  }
  
  // Watchdog
  appHandleIWDG_Refresh(); // The App. should not work longer than 9s without calling this function
  
  // Bluetooth
  if (BT_HasWork()) {
    BT_Do();
  }
  
  // USB CDC
  if (ui8USB_Class_CDC && bUSBinitialized) {
    ui16UsbCdcRxBuffLen = CDC_GetRxBufLen();
    if (ui16UsbCdcRxBuffLen)    
      App_UsbCdcProt(pui8UsbCdcRxBuffAddr, ui16UsbCdcRxBuffLen); 
  }  	    
}

/**
  * @brief  Repetitive task, which should be called inside main while
  */
void appDo(void)
{ 
   bool bAppMeasure = false;
   bool bAppMeasureLogged = false;
  
   
#if defined (DEF_TIMINGTEST)  
  /* Check Test timer */
  if (bTest){
    bTest = false;
    TimerSetValue( &TestTimer, 200 ); // Test for timing accuracy
    TimerStart( &TestTimer ); 
  }
#endif
 
  appHandleUI();
  
#if defined (DEF_USB)		
  /* Check USB Timer */
  if (bUSBTimerOver){			
    // Bei deaktivierter USB-HW liegen bei eingestecktem USB ca. 2.5V am EXT_5V_USB_DETECT_Pin (VINhigh >= 2.3V@Vdd3.3)
    if (!bUSBinitialized)
    {
      if ( HAL_GPIO_ReadPin(EXT_5V_USB_DETECT_GPIO_Port, EXT_5V_USB_DETECT_Pin)) // EXT_5V_USB_DETECT_Pin is DI
          InitUSB(); 
    }
    // ACHTUNG : EXT_5V_USB_DETECT_Pin wird bei aktivierter USB über die USB Datenleitung vom uC mit ca. 1.25V (über die Schutzdioden) gespiesen (VINlow <= 1V@Vdd3.3)    
    else if ( MEASGetADCmV( ADC_CHANNEL_15, 1) <= 1800) // EXT_5V_USB_DETECT_Pin is AI
      DeinitUSB();	
  
    bUSBTimerOver = false;
    TimerSetValue( &USBTimer, 3000 ); // every 3 secs
    TimerStart( &USBTimer );
    if (bUSBinitialized && bCom_SendMeasData && (eLEDLoraFSMState == LED_SLEEP))
      bAppMeasure = true;
  }
#endif	
   
  if (bCom_GetMeasData)
    bAppMeasure = true;
  
  if (bAppMeasIntervall)
  {
    bAppMeasIntervall = false;
    bAppMeasureLogged = true;
    bAppMeasure = true;    
  }
  
  if (bAppMeasure)
  {
    appHandleIWDG_Refresh(); // The App. should not work longer than 9s without calling this function
    SetLEDGreen(true);
    App_Measure(bAppMeasureLogged);
    SetLEDGreen(false);
    BSP_SleepDelayMs(100);
    if (bAppMeasureLogged)
    {      
      SetTxType(TX_ON_TIMER);
      
      if (LoraTxRxStatus())
        SetShowLED(true);
    }	
  }      
   
  /* Check if TxRx to be done, Lora not busy, and no Lora LED */
  if (GetTxType() && !LoraTxRxStatus() && !GetShowLED()){
    
#if defined (DEF_RF_ON_OFF)		
     /* turn on Module before sending if off */
     if (!GetLoraPower())	
       LoraON(); 
#endif	
     LoraTxRxStart();			
  } 
      
  // LoRa Rx Data ?
  if (sTFB_LoRaRx.bDataRx)
  {
    sTFB_LoRaRx.bDataRx = !App_HandleLoRaRxData();
  }  
  
	/* Check LED FSM */	
  LEDLoRaCtrlFSM();  
}

/**
  * @brief  builds TxPayload
  * @param  AppData: LoRa app data, payload must be defined here
  *         IsTxConfirmed: TX confirm or unconfirm
  *         bOnEvent: message happened because of an event
  */
void appBuildTxPayload(lora_AppData_t *AppData, FunctionalState* IsTxConfirmed, uint8_t u8OnEvent)
{
static uint8_t  u8DataPacketNo = 0;
static uint8_t  u8DataPos = 0;
static uint8_t  u8DataLen = 0;
static uint8_t  u8DataBuf[255] = {0};  
static uint8_t  u8GrpEndPosBuf[eUplink_GrpID_Cnt] = {0}; 
       float    fVal;
       uint8_t  u8DataSize = 0; 
       uint8_t  u8GrpCnt = 0;
       uint8_t  u8MaxBuffSize;
       uint8_t  u8Ch, u8Msk, u8ChMsk;
//       uint32_t u32Val;
       uint16_t u16Val;
       int16_t  i16Val;
	   
	//configure TxConfirmed bit here
  // *IsTxConfirmed =  ((MEASGetDIPSw() & 0x01) == 0x01) ? ENABLE : DISABLE;

  u8MaxBuffSize = LoraGetMaxPayloadSize(*IsTxConfirmed); 

#ifdef USE_IMST_ExpLoRaStudio
  if (u8MaxBuffSize > 127)
    u8MaxBuffSize = 127; // das IMST Tool hat noch Probleme mit längeren Payloads
#endif

  AppData->BuffSize = 0;
  AppData->Buff[AppData->BuffSize++] = APP_MAJOR_VERSION;
  AppData->Buff[AppData->BuffSize++] = (bColdJunctionTempErr ? 0x80 : 0x00) | (bExtAdcErr ? 0x40 : 0x00) | (bUSBinitialized ? 0x20 : 0x00) | (u8OnEvent & TX_ON_EVENT ? 0x10 : 0x00) | GetHWRev();

  // Prio 1 (first finish last long message)
  if (u8OnEvent & TX_ON_TIMER_EXT)
  {
     AppData->Buff[AppData->BuffSize++] = u8DataPacketNo;
     AppData->Buff[AppData->BuffSize++] = u8DataPos;  
  }
  // Prio 2
  else if (u8OnEvent & TX_ON_TIMER)
  {
    AppData->Buff[AppData->BuffSize++] = ++u8DataPacketNo; // 2018-10-03-Kd V1.0 (old u8DataPacketNo++)
    AppData->Buff[AppData->BuffSize++] = 0;   
    u8DataPos = 0;
    u8DataLen = 0;
    memset(u8GrpEndPosBuf,0,sizeof(u8GrpEndPosBuf));

//------------------------------------------------------------------------------	 
/**                  Payload structure 2018-07-04-Kd
*  BYTE           FUNCTION            REMARK
*  [0]            Major Version       Must change, when change happens in data format    
*	 [1]            Status              [ ColdJunctionTempErr | ExtAdcErr | USB_CONNECTED | TX_ON_EVENT | HW_REV_3 | HW_REV_2 | HW_REV_1 | HW_REV_0 ]
*  [2]            DataPacketNo  DP    0..255 same for 1 complete datapacket and for all needed uplinks
*  [3]            DataPos  DP         [0..x]    DataPos in cas of multiple blocks must be send w do not start form 0   
*	 [DP 0]         H Battery           [mV]  
*	 [DP 1]         L Battery           [mV] 
*	 [DP 2]         H Internal Temp.    34.52° is 3452 signed and so on   
*  [DP 3]         L Internal Temp.    
*  [DP 4]         TemperatureMask         
*  [DP 5]         L Ch1 Temp.         34.52° is 3452 signed and so on   
**/
//------------------------------------------------------------------------------     
     u8DataBuf[u8DataLen++] = (uint8_t)((u16BatteryVDDmV >> 8) & 0x00FF);
     u8DataBuf[u8DataLen++] = (uint8_t)((u16BatteryVDDmV >> 0) & 0x00FF);	
     u8DataBuf[u8DataLen++] = (uint8_t)((i16Temperature_GC_100 >> 8) & 0x00FF);
     u8DataBuf[u8DataLen++] = (uint8_t)((i16Temperature_GC_100 >> 0) & 0x00FF);
     
     // 8 Bytes till here	
     if (sCfg.sAppCfg.u8ChMskTemperature)
     {
       u8DataBuf[u8DataLen++] = eUplink_GrpID_TEMPERATURE;
       u8DataBuf[u8DataLen++] = sCfg.sAppCfg.u8ChMskTemperature;
       for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++, u8Msk<<=1)
       {
         if (u8Msk & sCfg.sAppCfg.u8ChMskTemperature)
         {
           i16Val = (int16_t)(sMEASExtSenseData.fTemp_gC[u8Ch] * 100); // [10m°C] -50..+250°C -> -5000..+25000 
           u8DataBuf[u8DataLen++] = (uint8_t)((i16Val >> 8) & 0x00FF);
           u8DataBuf[u8DataLen++] = (uint8_t)((i16Val >> 0) & 0x00FF);	       
         }
       } 
       u8GrpEndPosBuf[u8GrpCnt++] = u8DataLen-1;
     } // 8 + 2 + 8*2 = max 26 Bytes till here
     u8ChMsk = sCfg.sAppCfg.u8ChMskVoltage & ~sCfg.sAppCfg.u8ChMskVoltageRangeHighRes;
     if (u8ChMsk)
     {
       u8DataBuf[u8DataLen++] = eUplink_GrpID_VOLTAGE;
       u8DataBuf[u8DataLen++] = u8ChMsk;
       for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++, u8Msk<<=1)
       {
         if (u8Msk & u8ChMsk)
         {       
           u16Val = (uint16_t)(sMEASExtSenseData.fU_V[u8Ch] * 1000); // [mV] 0..48V -> 0..48000
           u8DataBuf[u8DataLen++] = (uint8_t)((u16Val >> 8) & 0x00FF);
           u8DataBuf[u8DataLen++] = (uint8_t)((u16Val >> 0) & 0x00FF);	       
         }
       }
       u8GrpEndPosBuf[u8GrpCnt++] = u8DataLen-1;
     } // 26 + 2 + 8*2 = 44 Bytes till here
     if (sCfg.sAppCfg.u8ChMskCurrent)
     {
       u8DataBuf[u8DataLen++] = eUplink_GrpID_CURRENT;
       u8DataBuf[u8DataLen++] = sCfg.sAppCfg.u8ChMskCurrent;
       for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++, u8Msk<<=1)
       {
         if (u8Msk & sCfg.sAppCfg.u8ChMskCurrent)
         {         
           u16Val = (uint16_t)(sMEASExtSenseData.fI_mA[u8Ch] * 100); // [10uA] 0..100mA -> 0..10000
           u8DataBuf[u8DataLen++] = (uint8_t)((u16Val >> 8) & 0x00FF);
           u8DataBuf[u8DataLen++] = (uint8_t)((u16Val >> 0) & 0x00FF);	       
         }
       }
       u8GrpEndPosBuf[u8GrpCnt++] = u8DataLen-1;
     } // 44 + 2 + 8*2 = 62 Bytes till here   
     if (sCfg.sAppCfg.u8ChMskCorrVoltage)
     {
       u8DataBuf[u8DataLen++] = eUplink_GrpID_CORRVOLTAGE;
       u8DataBuf[u8DataLen++] = sCfg.sAppCfg.u8ChMskCorrVoltage;       
       for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++, u8Msk<<=1)
       {
         if (u8Msk & sCfg.sAppCfg.u8ChMskCorrVoltage)
         {          
           i16Val = (int16_t)(sMEASExtSenseData.fCorrU_mV[u8Ch] * 10); // [100uV] -1000..1000mV -> -10000..+10000
           u8DataBuf[u8DataLen++] = (uint8_t)((i16Val >> 8) & 0x00FF);
           u8DataBuf[u8DataLen++] = (uint8_t)((i16Val >> 0) & 0x00FF);	       
         }
       }
       u8GrpEndPosBuf[u8GrpCnt++] = u8DataLen-1;
     } // 62 + 2 + 8*2 = 80 Bytes till here    
     if (sCfg.sAppCfg.u8ChMskCorrCurrent)
     {
       u8DataBuf[u8DataLen++] = eUplink_GrpID_CORRCURRENT;
       u8DataBuf[u8DataLen++] = sCfg.sAppCfg.u8ChMskCorrCurrent;
       for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++, u8Msk<<=1)
       {
         if (u8Msk & sCfg.sAppCfg.u8ChMskCorrCurrent)
         {          
           i16Val = (int16_t)(sMEASExtSenseData.fCorrI_uA[u8Ch] * 10); // [100nA] -1000..1000uA -> -10000..+10000
           u8DataBuf[u8DataLen++] = (uint8_t)((i16Val >> 8) & 0x00FF);
           u8DataBuf[u8DataLen++] = (uint8_t)((i16Val >> 0) & 0x00FF);	       
         }
       }
       u8GrpEndPosBuf[u8GrpCnt++] = u8DataLen-1;
     } // 80 + 2 + 8*2 = 98 Bytes till here 
     if (sCfg.sAppCfg.u8ChMskImpedance)
     {
       u8DataBuf[u8DataLen++] = eUplink_GrpID_IMPEDANCE;
       u8DataBuf[u8DataLen++] = sCfg.sAppCfg.u8ChMskImpedance;
       for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++, u8Msk<<=1)
       {
         if (u8Msk & sCfg.sAppCfg.u8ChMskImpedance)
         {   
           fVal = sMEASExtSenseData.fImp_Ohm[u8Ch] + 1;
           if (fVal < 1.0)
             u16Val = 0; // 0 Ohm
           else
             u16Val = (uint16_t)(10000 * log10(fVal)); // [log(Z+1)*10‘000] 0..1'000'000 Ohm -> 0..60'000
           u8DataBuf[u8DataLen++] = (uint8_t)((u16Val >> 8)  & 0x00FF);
           u8DataBuf[u8DataLen++] = (uint8_t)((u16Val >> 0)  & 0x00FF);	                
         }
       }
       u8GrpEndPosBuf[u8GrpCnt++] = u8DataLen-1;
     } // 98 + 2 + 8*2 = 116 Bytes till here          
     if (sCfg.sAppCfg.u8ChMskPhaseAngle)
     {
       u8DataBuf[u8DataLen++] = eUplink_GrpID_IMPEDANCEPHASE;
       u8DataBuf[u8DataLen++] = sCfg.sAppCfg.u8ChMskPhaseAngle;
       for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++, u8Msk<<=1)
       {
         if (u8Msk & sCfg.sAppCfg.u8ChMskPhaseAngle)         
           u8DataBuf[u8DataLen++] = sMEASExtSenseData.i8ImpPhaseShiftAngle_g[u8Ch];                          
       }
       u8GrpEndPosBuf[u8GrpCnt++] = u8DataLen-1;
     } // 116 + 2 + 8*1 = 126 Bytes till here    
     if (sCfg.sAppCfg.u8ChMskResistance)
     {
       u8DataBuf[u8DataLen++] = eUplink_GrpID_RESISTANCE;
       u8DataBuf[u8DataLen++] = sCfg.sAppCfg.u8ChMskResistance;
       for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++, u8Msk<<=1)
       {
         if (u8Msk & sCfg.sAppCfg.u8ChMskResistance)
         {   
           fVal = sMEASExtSenseData.fImpActive_Ohm[u8Ch] + 1;
           if (fVal < 1.0)
             u16Val = 0; // 0 Ohm
           else
             u16Val = (uint16_t)(10000 * log10(fVal)); // [log(R+1)*10‘000] 0..1'000'000 Ohm -> 0..60'000
           u8DataBuf[u8DataLen++] = (uint8_t)((u16Val >> 8)  & 0x00FF);
           u8DataBuf[u8DataLen++] = (uint8_t)((u16Val >> 0)  & 0x00FF);	                
         }
       }
       u8GrpEndPosBuf[u8GrpCnt++] = u8DataLen-1;
     } // 126 + 2 + 8*2 = 144 Bytes till here
     // 2019-01-07-Kd new V1.03
     u8ChMsk = sCfg.sAppCfg.u8ChMskVoltage & sCfg.sAppCfg.u8ChMskVoltageRangeHighRes;
     if (u8ChMsk)
     {     
       u8DataBuf[u8DataLen++] = eUplink_GrpID_VOLTAGEHIGHRES;
       u8DataBuf[u8DataLen++] = u8ChMsk;
       for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++, u8Msk<<=1)
       {
         if (u8Msk & u8ChMsk)
         {       
           u16Val = (uint16_t)(sMEASExtSenseData.fU_V[u8Ch] * 10000); // [100uV] 0..6.5V -> 0..65000
           u8DataBuf[u8DataLen++] = (uint8_t)((u16Val >> 8) & 0x00FF);
           u8DataBuf[u8DataLen++] = (uint8_t)((u16Val >> 0) & 0x00FF);	       
         }
       }
       u8GrpEndPosBuf[u8GrpCnt++] = u8DataLen-1;
     }
     // 144 + 2 + 8*2 = 162 Bytes till here
    //                                SF   7   8   9   10   11   12
    // MaxPayloadOfDatarateRepeater[] = { 51, 51, 51, 115, 222, 222, 222, 222 }; without MAC-Data
  } 
  else // Prio 3 Tasterevent
  {
    u8DataPos = 0;
    u8DataLen = 0;
  }
       
  if (u8DataPos < u8DataLen)
  {
    u8DataSize = u8DataLen - u8DataPos;
    if (u8DataSize > (u8MaxBuffSize - AppData->BuffSize))   
    {     
      u8DataSize = u8MaxBuffSize - AppData->BuffSize;
      // Set to next GroupPos (do not split groups in seperate uplinks)
      u8GrpCnt=sizeof(u8GrpEndPosBuf); // Start from last group pos
      do { 
        u8GrpCnt--;
        if (u8GrpEndPosBuf[u8GrpCnt] && (u8GrpEndPosBuf[u8GrpCnt] < (u8DataPos+u8DataSize)))
        {
          u8DataSize = u8GrpEndPosBuf[u8GrpCnt]+1-u8DataPos;
          break;
        }        
      } while (u8GrpCnt);
      SetTxType(TX_ON_TIMER_EXT); // Need more Uplinks
    }
    else
      u8DataSize = u8DataLen - u8DataPos;
    memcpy(&AppData->Buff[AppData->BuffSize], &u8DataBuf[u8DataPos], u8DataSize);
    u8DataPos += u8DataSize; 

    AppData->BuffSize += u8DataSize; 
	}
  
  AppData->Port = LoraGetTxPort();  
  
  /* Printfs start here, this is done only for debug purposes */
  if (u8OnEvent & TX_ON_TIMER){		
    PRINTF("TX Type = TX_ON_TIMER\r\n");
  } else if (u8OnEvent & TX_ON_TIMER_EXT){		
    PRINTF("TX Type = TX_ON_TIMER_EXT\r\n");
  } else if (u8OnEvent & TX_ON_EVENT) {
    PRINTF("TX Type = TX_ON_EVENT\r\n");    
  } else {
    PRINTF("TX Type = TX_ON_INIT\r\n");	
  }	
}	

/**
  * @brief  Parses Rx Payload
  *         
  * @param  AppData: LoRa app data, payload must be read from here
  */
void appParseRxPayloadIsr(lora_AppData_t *pAppData)
{	

  // first get the correct RxPort from the loraModem
  uint8_t u8RxPort = LoraGetRxPort();

  // Parsing starts in appDo() loop
  if ((pAppData->Port == u8RxPort) && (pAppData->BuffSize < APP_RX_BUF_SIZE) && !sTFB_LoRaRx.bDataRx)
  {
    memcpy(&sTFB_LoRaRx.sLoRaAppDataRx, pAppData, sizeof(lora_AppData_t));
    memcpy(&sTFB_LoRaRx.aRxBuf, pAppData->Buff, pAppData->BuffSize);      
    sTFB_LoRaRx.bDataRx = true;
  }			 	
}	

/* Static functions --------------------------------------------------------------*/

/**
  * @brief  FSM, which controls the usage of the LED for LoRa functionalities
  */
static void LEDLoRaCtrlFSM( void )
{
	static uint8_t u8ProcessCount = 0;	
	
	//according to ModemFlags we set something
	if (GetShowLED() && (eLEDLoraFSMState == LED_SLEEP) && (eLEDLoraFSMStateLast == LED_SLEEP)){
		//according to ModemFlags we set something	
		if (LoraTxRxStatus()){
		  if ((lora_getModemFlags() & JOINERROR)){
        eLEDLoraFSMState = LED_JOIN_ERROR;
      } else {
        eLEDLoraFSMState = LED_TX_DELAY;
			}			
		} else {	
	    if ((lora_getModemFlags() & TXDONE) && (lora_getModemFlags() & RXDONE)){
		    eLEDLoraFSMState = LED_TX_RX_OK;
	    }	else if ((lora_getModemFlags() & TXDONE) && (lora_getModemFlags() & RX2TIMEOUT)){
			  eLEDLoraFSMState = LED_TX_OK;
      }
    }		
	}

	//now copy last LEDFSMState to a local variable to process it
	tLEDFSMStates eFSMStateNow = eLEDLoraFSMState;
    	
	//process local state
  switch (eFSMStateNow)
	{
    case LED_NONE:
      SetLEDGreen(false);			
      SetLEDRed(false);
			eLEDLoraFSMStateLast = LED_NONE;
      eLEDLoraFSMState     = LED_SLEEP;			
    break;
   
    case LED_TX_OK:
			if(!GetLEDGreen()){
				SetLEDGreen(true);
				TimerSetValue(&LEDLoRaDisplayTimer, LED_OK_BLINK);
			  TimerStart(&LEDLoRaDisplayTimer);
     	}
			eLEDLoraFSMStateLast = LED_TX_OK;
      eLEDLoraFSMState     = LED_WAIT;			
    break;

    case LED_TX_RX_OK:
			if((!GetLEDGreen()) && (!GetLEDRed())){
				SetLEDGreen(true);
				SetLEDRed(true);
				TimerSetValue(&LEDLoRaDisplayTimer, LED_OK_BLINK);
			  TimerStart(&LEDLoRaDisplayTimer);				
     	}
			eLEDLoraFSMStateLast = LED_TX_RX_OK;			
      eLEDLoraFSMState     = LED_WAIT;			
    break;

    case LED_JOIN_ERROR:
			if(!GetLEDRed()){
				SetLEDRed(true);
				TimerSetValue(&LEDLoRaDisplayTimer, LED_NOK_BLINK);			
			  TimerStart(&LEDLoRaDisplayTimer);			
			} else {
				SetLEDRed(false);
				TimerSetValue(&LEDLoRaDisplayTimer, LED_NOK_BLINK * 4);			
			  TimerStart(&LEDLoRaDisplayTimer);						
			}	
			eLEDLoraFSMStateLast = LED_JOIN_ERROR;			
      eLEDLoraFSMState     = LED_WAIT;				
    break;

    case LED_TX_DELAY:
			if(!GetLEDGreen()){
				SetLEDGreen(true);
				TimerSetValue(&LEDLoRaDisplayTimer, LED_NOK_BLINK);			
			  TimerStart(&LEDLoRaDisplayTimer);			
			} else {
				SetLEDGreen(false);
				TimerSetValue(&LEDLoRaDisplayTimer, LED_NOK_BLINK * 4);			
			  TimerStart(&LEDLoRaDisplayTimer);						
			}	
			eLEDLoraFSMStateLast = LED_TX_DELAY;			
      eLEDLoraFSMState     = LED_WAIT;				
    break;
		
    case LED_PROCESS:
		switch (eLEDLoraFSMStateLast)   
      {
			  case LED_TX_OK:
					eLEDLoraFSMState = LED_NONE;
        break;					
				
				case LED_TX_RX_OK:
          eLEDLoraFSMState = LED_NONE;					
				break;

        case LED_JOIN_ERROR:
					u8ProcessCount++;
				  if (u8ProcessCount <= 3){
					  eLEDLoraFSMState = LED_JOIN_ERROR;
          } else {
            u8ProcessCount = 0;
						eLEDLoraFSMState = LED_NONE;
          }						
        break;

        case LED_TX_DELAY:
					u8ProcessCount++;
				  if (u8ProcessCount <= 3){
					  eLEDLoraFSMState = LED_TX_DELAY;
          } else {
            u8ProcessCount = 0;
						eLEDLoraFSMState = LED_NONE;
          }					
        break;		

        default:
					eLEDLoraFSMState = LED_NONE;
        break; 					
			}			
		break;	
		
		case LED_WAIT:
			
		break;
		
		case LED_SLEEP:
			eLEDLoraFSMStateLast = LED_SLEEP;
			if (GetShowLED()){
			  SetShowLED(false);	
			}		
		break;	
		
		default:
		break;	 
	}		
}	

/**
  * @brief  calculate actual TxInterval for next TX_ON_TIMER
  */
static uint32_t CalculateTxInterval(void)
{
  uint32_t u32RetVal = 0;
  
	if (sCfg.sAppCfg.u32AppTxInterval_m){
    u32RetVal = (sCfg.sAppCfg.u32AppTxInterval_m * 60 * 1000) + randr( -sLoRaParamInit.DutyCycleRandomTime, sLoRaParamInit.DutyCycleRandomTime ) - sLoRaParamInit.DutyCycleSensorCaptureTime;	
	}
	
	return u32RetVal;
}	
   
#if defined DEF_USB	
/**
  * @brief  Init USB block
  */
static void InitUSB(void)
{
  GPIO_InitTypeDef initStruct={0};
  
  __COMP_CLK_ENABLE();
  __SYSCFG_CLK_ENABLE();
  MX_USB_DEVICE_Init();
  bUSBinitialized = true;
  LowPower_Disable(e_LOW_POWER_USB);
  
  initStruct.Pull   = GPIO_NOPULL;      
  initStruct.Mode  = GPIO_MODE_ANALOG;
  HW_GPIO_Init(EXT_5V_USB_DETECT_GPIO_Port, EXT_5V_USB_DETECT_Pin, &initStruct);  
}  

/**
  * @brief  Deinit USB block
  */
static void DeinitUSB(void)
{
  GPIO_InitTypeDef initStruct={0};
  bUSBinitialized = false;
  LowPower_Enable(e_LOW_POWER_USB);
  USBD_DeInit(&hUsbDeviceFS);   
  initStruct.Mode   = GPIO_MODE_INPUT;
  initStruct.Speed  = GPIO_SPEED_MEDIUM;  
  initStruct.Pull   = GPIO_PULLDOWN;  
  HW_GPIO_Init(USB_DM_GPIO_PORT, USB_DM_PIN, &initStruct);  // activate PullDown  
  HW_GPIO_Init(USB_DP_GPIO_PORT, USB_DP_PIN, &initStruct);  // activate PullDown   
  initStruct.Pull   = GPIO_NOPULL;      
  HW_GPIO_Init(EXT_5V_USB_DETECT_GPIO_Port, EXT_5V_USB_DETECT_Pin, &initStruct);  
}
#endif

/*****END OF FILE****/
