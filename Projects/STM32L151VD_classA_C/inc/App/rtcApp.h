/**============================================================================
* @file      rtcApp.h
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of rtcApp.c

*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File  	
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_APP_H__
#define __RTC_APP_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "utilities.h"
#include <stdbool.h>
   
/* Exported types ------------------------------------------------------------*/
typedef enum {
  RtcBackupRamReg_TimeDiffSub_s = 0,
  RtcBackupRamReg_TimeDiffAdd_s = 1,
  RtcBackupRamReg_LogNr         = 2,
}teRtcBackupRamReg; // Max. 32 Stk 4 Byte Register

/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void App_RTC_SaveBackupRegister( teRtcBackupRamReg eRtcBackupRamReg, uint32_t Data );

uint32_t App_RTC_ReadBackupRegister( teRtcBackupRamReg eRtcBackupRamReg);

/*!
 * @brief calculates the wake up time between wake up and mcu start
 * @note resolution in RTC_ALARM_TIME_BASE
 * @param none
 * @retval none
 */
void App_RTC_setMcuWakeUpTime( void );

/*!
 * @brief returns the wake up time in us
 * @param none
 * @retval wake up time in ticks
 */
int16_t App_RTC_getMcuWakeUpTime( void );

/*!
 * @brief Return the minimum timeout the RTC is able to handle
 * @param none
 * @retval minimum value for a timeout
 */
uint32_t App_RTC_GetMinimumTimeout( void );

/*!
 * @brief Set the alarm A
 * @note The alarm is set at Reference + timeout
 * @param timeout Duration of the Timer in ticks
 */
void App_RTC_SetAlarm( uint32_t timeout );

/*!
 * @brief Stop the Alarm A
 * @param none
 * @retval none
 */
void App_RTC_StopAlarm( void );

/*!
 * @brief RTC IRQ Handler on the RTC Alarm A
 * @param none
 * @retval none
 */
void App_RTC_IrqHandler ( void );

void App_RTC_Init(bool bBackupPowerOn);
void App_RTC_GetDateTime(RTC_DateTypeDef* psDate, RTC_TimeTypeDef* psTime);
bool App_RTC_SetDateTime(RTC_DateTypeDef* psDate, RTC_TimeTypeDef* psTime);
uint32_t App_RTC_GetDateTimeStamp(void);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_APP_H__ */

/*****END OF FILE****/
