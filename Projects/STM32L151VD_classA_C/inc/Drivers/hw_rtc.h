/**============================================================================
* @file      hw_rtc.h
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of hw_rtc.c

*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File  	
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_RTC_H__
#define __HW_RTC_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "utilities.h"
   
/* Exported types ------------------------------------------------------------*/
typedef struct
{
  TimerTime_t Rtc_Time; /* Reference time */
  RTC_TimeTypeDef RTC_Calndr_Time; /* Reference time in calendar format */
  RTC_DateTypeDef RTC_Calndr_Date; /* Reference date in calendar format */
} RtcTimerContext_t;

/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern RtcTimerContext_t RtcTimerContext;	 
extern RTC_HandleTypeDef hrtc;
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

/*!
 * @brief Initializes the RTC timer
 * @note The timer is based on the RTC
 * @param bool bBackupPowerOn 
 * @retval none
 */
void HW_RTC_Init( bool bBackupPowerOn  );

/**
  * @brief          Gets DateTimeStamp (Microsoft Windows 32-bit date and time format) from Date and Time
  * @param[in]		  psDate as RTC_DateTypeDef*
  * @param[in]	    psTime as RTC_TimeTypeDef*
  * @retval         dateTimeStamp as uint32_t
  */
uint32_t HW_RTC_GetDateTimeStampFromDateTime(RTC_DateTypeDef *psDate, RTC_TimeTypeDef *psTime);

/*!
 * @brief Get RTC time and date
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval success
 */
bool HW_RTC_GetDateTime(RTC_DateTypeDef* psDate, RTC_TimeTypeDef* psTime);

/**
  * @brief          Set the data and time to the RTC 
  * @param[in]		  psDate as RTC_DateTypeDef*
  * @param[in]		  psTime as RTC_TimeTypeDef*
  * @retval	        None
  */
void HW_RTC_SetDateTime(RTC_DateTypeDef *psDate, RTC_TimeTypeDef *psTime);

/*!
* @brief calc date time to s since 1.1.2000 00:00:00
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval time in s
 */
TimerTime_t HW_RTC_CalcCalendarValue_s( RTC_DateTypeDef* RTC_DateStruct, RTC_TimeTypeDef* RTC_TimeStruct );

/*!
* @brief get current time from calendar in s since 1.1.2000 00:00:00
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval time in s
 */
TimerTime_t HW_RTC_GetCalendarValue_s( RTC_DateTypeDef* RTC_DateStruct, RTC_TimeTypeDef* RTC_TimeStruct );
  
/*!
 * @brief a delay of delay ms by polling RTC
 * @param delay in ms
 * @param none
 * @retval none
 */
void HW_RTC_DelayMs( uint32_t delay );

/*!
 * @brief Set the RTC timer Reference
 * @retval  Timer Reference Value in  Ticks
 */
uint32_t HW_RTC_SetTimerContext( void );
  
/*!
 * @brief Get the RTC timer Reference
 * @retval Timer Value in  Ticks
 */
uint32_t HW_RTC_GetTimerContext( void );

/*!
 * @brief Get the RTC timer value
 * @retval none
 */
uint32_t HW_RTC_GetTimerValue( void );

/*!
 * @brief Get the RTC timer elapsed time since the last Reference was set
 * @retval RTC Elapsed time in ticks
 */
uint32_t HW_RTC_GetTimerElapsedTime( void );

/*!
 * @brief converts time in ms to time in ticks
 * @param [IN] time in milliseconds
 * @retval returns time in timer ticks
 */
uint32_t HW_RTC_ms2Tick( TimerTime_t timeMicroSec );

/*!
 * @brief converts time in ticks to time in ms
 * @param [IN] time in timer ticks
 * @retval returns time in timer milliseconds
 */
TimerTime_t HW_RTC_Tick2ms( uint32_t tick );

/*!
 * @brief get current time from calendar in ticks
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval time in ticks
 */
TimerTime_t HW_RTC_GetCalendarValue( RTC_DateTypeDef* RTC_DateStruct, RTC_TimeTypeDef* RTC_TimeStruct );

/*!
 * @brief get current time from calendar in Unix time -> Seconds since 1.1.1970 00:00:00 
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval time in s
 */
TimerTime_t HW_RTC_GetCalendarUnix( void );


/**
  * @brief          Gets DateTimeStamp from RTC
  * @retval         dateTimeStamp as uint32_t
  */
uint32_t HW_RTC_GetDateTimeStamp(void);


#ifdef __cplusplus
}
#endif

#endif /* __HW_RTC_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
