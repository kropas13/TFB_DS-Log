/**============================================================================
* @file      app.h
* @date      2017-08-30
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of app.c
*            
* VERSION:   
* 
* V0.10      2017-08-30-Ra      Create File 			
*
*============================================================================*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/	
#include "lora.h"
#include "meas.h"
	 
/* Exported types ------------------------------------------------------------*/
      
typedef enum
{
	LED_NONE,
	LED_TX_OK,
	LED_TX_RX_OK,
	LED_JOIN_ERROR,
	LED_TX_DELAY,
	LED_PROCESS,
	LED_WAIT,
	LED_SLEEP,
} tLEDFSMStates;	

/* Exported constants --------------------------------------------------------*/
#define APP_MINOR_VERSION   12
#define APP_MAJOR_VERSION   1

#define DEVICE_ID	      17  
#define APP_ID          0  

/*!
 * default Application send interval in minutes
 */
#define APP_DEF_TX_INTERVAL                      15

/*!
 * Max application send interval, which can be set by downlink
 */
#define APP_MAX_DUTY_ON_DOWNLINK                 1440 // One day 
/*!
 * Defines the application's rx buffer size
 */
#define APP_RX_BUF_SIZE                          64
/*!
 * Defines RX Received port for application
 */
#define APP_RX_RECEIVED_PORT                     2
/*!
 * Defines a random delay for application data transmission duty cycle. 1s,
 * value in [ms].
 */
#define APP_TX_DUTYCYCLE_RND                     1000

#if defined( USE_BAND_915 )
  /*!
   * Max Tx-datarate
   */
  #define APP_MAX_DATARATE                         DR_4 
  /*!
   * Min Tx-datarate
   */
  #define APP_MIN_DATARATE                         DR_0
  /*!
   * Default datarate
   */
  #define APP_DEFAULT_DATARATE                     DR_0 
  /*!
   * Default datarate
   */
  #define APP_DEFAULT_RX2_DATARATE                 DR_8

#else
  /*!
   * Max Tx-datarate
   */
  #define APP_MAX_DATARATE                         DR_5 
  /*!
   * Min Tx-datarate
   */
  #define APP_MIN_DATARATE                         DR_0
  /*!
   * Default datarate
   */
  #define APP_DEFAULT_DATARATE                     DR_0 
  /*!
   * Default datarate
   */
  #define APP_DEFAULT_RX2_DATARATE                 DR_3 // Für Loriot und IMST_Studio 

#endif

/*!
 * Defines a random delay for application data transmission duty cycle and GPS fetches.
 * value in [ms].
 */
#define APP_DATA_CAPTURE_TIME                    200 // V1.11 2024-03-18-Kd die 1.65V ist erst nach ca. 200ms stabil old 80 // Cold Junction Temp sensor needs min 80 ms


/* Exported variables -----------------------------------------------------------*/
//permanent appication and device options
typedef struct
{
   uint32_t                 u32AppTxInterval_m; 
   uint32_t                 u32AppMeasInterval_m; // Not used till now, but when like to have different measure intervall to 
   t_eMEASExtSenseTempType  au8TemperatureSensorType[EXT_SENSE_CHANNELS];
   uint8_t                  u8ChMskTemperature;
   uint8_t                  u8ChMskVoltage;
   uint8_t                  u8ChMskCurrent;
   uint8_t                  u8ChMskCorrVoltage;
   uint8_t                  u8ChMskCorrCurrent;
   uint8_t                  u8ChMskImpedance;
   uint8_t                  u8ChMskPhaseAngle; // 2018-10-03-Kd new V1.00
   uint8_t                  u8ChMskResistance; // 2018-10-03-Kd new V1.00 (real part of impedance)
   uint8_t                  u8ExtSensorsStartupTime_s; // 2018-10-26-Kd new V1.02
   uint8_t                  u8ChMskVoltageRangeHighRes; // 2019-01-07-Kd new V1.03
} tAppCfg; 

extern uint8_t  ui8ResetValue;

extern uint16_t u16BatteryVDDmV;  
extern int16_t  i16Temperature_GC_100;

extern bool     bUSBinitialized;
extern bool     bColdJunctionTempErr, bExtAdcErr;


/* Exported functions ------------------------------------------------------- */ 	
/**
  * @brief  sets application's default values
  */
void appSetDefault( void );
/**
  * @brief  Initializes application 
  */
void appInit(void);
/**
  * @brief  Handles the watchdog refresh 
  *         After IWDG refresh (19..40s) the next refresh will be timer triggered in 10s and the app can take additive max. 9s to call this function
  */	 
void appHandleIWDG_Refresh(void);
/**
  * @brief  Repetitive task, which should be called inside main while
  */
void appDo(void);
	 
/**
 * @brief  Is there anything to do?
 * @retval true --> yes, false --> no
 */
bool appHasWork(void);	 

/**
  * @brief  builds TxPayload before sending
  */
void appBuildTxPayload(lora_AppData_t *AppData, FunctionalState* IsTxConfirmed, uint8_t u8OnEvent);

/**
  * @brief  parses received payload
  */
void appParseRxPayloadIsr(lora_AppData_t *AppData);	 	
	 
#ifdef __cplusplus
}
#endif

#endif /*__APP_H */

/*****END OF FILE****/
