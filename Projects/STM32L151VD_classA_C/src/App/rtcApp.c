/**============================================================================
* @file      rtcApp.c
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     rtc application used by lora part of the code

*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File  	
*
*============================================================================*/

/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "low_power.h"
#include "rtcApp.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* MCU Wake Up Time */
#define MIN_ALARM_DELAY              3 /* in ticks */
/* subsecond number of bits */
#define N_PREDIV_S                   10
/* Synchonuous prediv  */
#define PREDIV_S                     ((1<<N_PREDIV_S)-1)
/* Asynchonuous prediv   */
#define PREDIV_A                     (1<<(15-N_PREDIV_S))-1
/* Sub-second mask definition  */
#if  (N_PREDIV_S == 10)
#define HW_RTC_ALARMSUBSECONDMASK    RTC_ALARMSUBSECONDMASK_SS14_10
#else
#error "Please define HW_RTC_ALARMSUBSECONDMASK"
#endif

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

uint32_t gu32TimeDiffSub_s; // __attribute__((section("NoInit"), zero_init));  // oder in BatterybackupRAM mit Überschreibschutz
uint32_t gu32TimeDiffAdd_s; // __attribute__((section("NoInit"), zero_init));
        
        
/*!
 * \brief compensates MCU wakeup time
 */
static bool McuWakeUpTimeInitialized = false;
/*!
 * \brief compensates MCU wakeup time
 */
static int16_t McuWakeUpTimeCal = 0;
/*!
 * Number of seconds in a minute
 */
static const uint8_t SecondsInMinute = 60;
/*!
 * Number of seconds in an hour
 */
static const uint16_t SecondsInHour = 3600;
/*!
 * Number of seconds in a day
 */
static const uint32_t SecondsInDay = 86400;
/*!
 * Number of hours in a day
 */
static const uint8_t HoursInDay = 24;
/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
/*!
 * AlarmTypeDef variable
 */
static RTC_AlarmTypeDef RTC_AlarmStructure;

/* Private function prototypes -----------------------------------------------*/
static void App_RTC_StartWakeUpAlarm( uint32_t timeoutValue );

/* Exported functions ---------------------------------------------------------*/
/*!
 * @brief calculates the wake up time between wake up and mcu start
 * @note resulotion in RTC_ALARM_TIME_BASE in timer ticks
 * @param none
 * @retval none
 */
void App_RTC_setMcuWakeUpTime( void )
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;
  
  TimerTime_t now, hit;
  int16_t McuWakeUpTime;
  
  if ((McuWakeUpTimeInitialized == false) &&
      ( HAL_NVIC_GetPendingIRQ( RTC_Alarm_IRQn ) == 1)) { 
    /* warning: works ok if now is below 30 days
    it is ok since it's done once at first alarm wake-up*/
    McuWakeUpTimeInitialized = true;
    now = HW_RTC_GetCalendarValue( &RTC_DateStruct, &RTC_TimeStruct );

    HAL_RTC_GetAlarm(&hrtc, &RTC_AlarmStructure, RTC_ALARM_A, RTC_FORMAT_BIN );
    hit = RTC_AlarmStructure.AlarmTime.Seconds+
          60*(RTC_AlarmStructure.AlarmTime.Minutes+
          60*(RTC_AlarmStructure.AlarmTime.Hours+
          24*(RTC_AlarmStructure.AlarmDateWeekDay)));
    hit = ( hit << N_PREDIV_S ) + (PREDIV_S - RTC_AlarmStructure.AlarmTime.SubSeconds);
      
    McuWakeUpTime = (int16_t) ((now-hit));
    McuWakeUpTimeCal += McuWakeUpTime;
    DBG_PRINTF("Cal=%d, %d\n",McuWakeUpTimeCal, McuWakeUpTime);
  }
}

int16_t App_RTC_getMcuWakeUpTime( void )
{
  return McuWakeUpTimeCal;
}

/*!
 * @brief returns the wake up time in ticks
 * @param none
 * @retval wake up time in ticks
 */
uint32_t App_RTC_GetMinimumTimeout( void )
{
  return( MIN_ALARM_DELAY );
}

/*!
 * @brief Set the alarm
 * @note The alarm is set at now (read in this funtion) + timeout
 * @param timeout Duration of the Timer ticks
 */
void App_RTC_SetAlarm( uint32_t timeout )
{
  /* we don't go in Low Power mode for timeout below MIN_ALARM_DELAY */
  if ( (MIN_ALARM_DELAY + McuWakeUpTimeCal ) < ((timeout - HW_RTC_GetTimerElapsedTime( ) )) ) {
    LowPower_Enable( e_LOW_POWER_RTC );
  } else {
    LowPower_Disable( e_LOW_POWER_RTC );
  }

  if( LowPower_GetState() == 0 ) {
    LowPower_Enable( e_LOW_POWER_RTC );
    timeout = timeout -  McuWakeUpTimeCal;
  }

  App_RTC_StartWakeUpAlarm( timeout );
}

/*!
 * @brief Stop the Alarm A
 * @param none
 * @retval none
 */
void App_RTC_StopAlarm( void )
{
  /* Clear RTC Alarm Flag */
  __HAL_RTC_ALARM_CLEAR_FLAG( &hrtc, RTC_FLAG_ALRAF);

  /* Disable the Alarm A interrupt */
  HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A );
}

/* Private functions ---------------------------------------------------------*/
/*!
 * @brief start wake up alarm A
 * @note  alarm in RtcTimerContext.Rtc_Time + timeoutValue
 * @param timeoutValue in ticks
 * @retval none
 */
static void App_RTC_StartWakeUpAlarm( uint32_t timeoutValue )
{
  uint16_t rtcAlarmSubSeconds = 0;
  uint16_t rtcAlarmSeconds = 0;
  uint16_t rtcAlarmMinutes = 0;
  uint16_t rtcAlarmHours = 0;
  uint16_t rtcAlarmDays = 0;
  RTC_TimeTypeDef RTC_TimeStruct = RtcTimerContext.RTC_Calndr_Time;
  RTC_DateTypeDef RTC_DateStruct = RtcTimerContext.RTC_Calndr_Date;

  App_RTC_StopAlarm( );
  
  /*reverse counter */
  rtcAlarmSubSeconds =  PREDIV_S - RTC_TimeStruct.SubSeconds;
  rtcAlarmSubSeconds += ( timeoutValue & PREDIV_S);
  /* convert timeout  to seconds */
  timeoutValue >>= N_PREDIV_S;  /* convert timeout  in seconds */
  
  /*convert microsecs to RTC format and add to 'Now' */
  rtcAlarmDays =  RTC_DateStruct.Date;
  while (timeoutValue >= SecondsInDay)
  {
    timeoutValue -= SecondsInDay;
    rtcAlarmDays++;
  }
  
  /* calc hours */
  rtcAlarmHours = RTC_TimeStruct.Hours;
  while (timeoutValue >= SecondsInHour)
  {
    timeoutValue -= SecondsInHour;
    rtcAlarmHours++;
  }
  
  /* calc minutes */
  rtcAlarmMinutes = RTC_TimeStruct.Minutes;
  while (timeoutValue >= SecondsInMinute)
  {
    timeoutValue -= SecondsInMinute;
    rtcAlarmMinutes++;
  }
   
  /* calc seconds */
  rtcAlarmSeconds =  RTC_TimeStruct.Seconds + timeoutValue;

  /***** correct for modulo********/
  while (rtcAlarmSubSeconds >= (PREDIV_S+1))
  {
    rtcAlarmSubSeconds -= (PREDIV_S+1);
    rtcAlarmSeconds++;
  }
  
  while (rtcAlarmSeconds >= 60)
  { 
    rtcAlarmSeconds -= 60;
    rtcAlarmMinutes++;
  }

  while (rtcAlarmMinutes >= 60)
  {
    rtcAlarmMinutes -= 60;
    rtcAlarmHours++;
  }
  
  while (rtcAlarmHours >= HoursInDay)
  {
    rtcAlarmHours -= HoursInDay;
    rtcAlarmDays++;
  }

  if( RTC_DateStruct.Year % 4 == 0 ) {
    if( rtcAlarmDays > DaysInMonthLeapYear[ RTC_DateStruct.Month - 1 ] ) {
      rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[ RTC_DateStruct.Month - 1 ];
    }
  } else {
    if( rtcAlarmDays > DaysInMonth[ RTC_DateStruct.Month - 1 ] ) {   
      rtcAlarmDays = rtcAlarmDays % DaysInMonth[ RTC_DateStruct.Month - 1 ];
    }
  }
  /* Set RTC_AlarmStructure with calculated values*/
  RTC_AlarmStructure.AlarmTime.SubSeconds = PREDIV_S-rtcAlarmSubSeconds;
  RTC_AlarmStructure.AlarmSubSecondMask  = HW_RTC_ALARMSUBSECONDMASK; 
  RTC_AlarmStructure.AlarmTime.Seconds = rtcAlarmSeconds;
  RTC_AlarmStructure.AlarmTime.Minutes = rtcAlarmMinutes;
  RTC_AlarmStructure.AlarmTime.Hours   = rtcAlarmHours;
  RTC_AlarmStructure.AlarmDateWeekDay    = ( uint8_t )rtcAlarmDays;
  RTC_AlarmStructure.AlarmTime.TimeFormat   = RTC_TimeStruct.TimeFormat;
  RTC_AlarmStructure.AlarmDateWeekDaySel   = RTC_ALARMDATEWEEKDAYSEL_DATE; 
  RTC_AlarmStructure.AlarmMask       = RTC_ALARMMASK_NONE;
  RTC_AlarmStructure.Alarm = RTC_ALARM_A;
  RTC_AlarmStructure.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  RTC_AlarmStructure.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  
  /* Set RTC_Alarm */
  HAL_RTC_SetAlarm_IT( &hrtc, &RTC_AlarmStructure, RTC_FORMAT_BIN );
  
  /* Debug Printf*/
  DBG( HW_RTC_GetCalendarValue( &RTC_DateStruct, &RTC_TimeStruct ); );
  DBG_PRINTF("it's %d:%d:%d:%d ", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds, ((PREDIV_S - RTC_TimeStruct.SubSeconds)*1000)>>N_PREDIV_S);
  DBG_PRINTF("WU@ %d:%d:%d:%d\n", rtcAlarmHours, rtcAlarmMinutes, rtcAlarmSeconds, (rtcAlarmSubSeconds*1000)>>N_PREDIV_S ); 
}

void App_RTC_SaveBackupRegister( teRtcBackupRamReg eRtcBackupRamReg, uint32_t Data )
{
  HAL_RTCEx_BKUPWrite( &hrtc, eRtcBackupRamReg, Data);
}

uint32_t App_RTC_ReadBackupRegister( teRtcBackupRamReg eRtcBackupRamReg)
{
  return HAL_RTCEx_BKUPRead(&hrtc, eRtcBackupRamReg);
}

void App_RTC_Init(bool bBackupPowerOn)
{  
  if (bBackupPowerOn)
  {
    App_RTC_SaveBackupRegister( RtcBackupRamReg_TimeDiffAdd_s, 0);
    App_RTC_SaveBackupRegister( RtcBackupRamReg_TimeDiffSub_s, 0);
    gu32TimeDiffSub_s = 0;
    gu32TimeDiffAdd_s = 0;
  }
  else
  {
    gu32TimeDiffAdd_s = App_RTC_ReadBackupRegister( RtcBackupRamReg_TimeDiffAdd_s);
    gu32TimeDiffSub_s = App_RTC_ReadBackupRegister( RtcBackupRamReg_TimeDiffSub_s);
  }
}

void App_RTC_GetDateTimeFromCalendarValue_s( TimerTime_t timeStamp_s, RTC_DateTypeDef *psDate, RTC_TimeTypeDef *psTime)
{
    /* Calculates ceiling(X/N) */
    #define DIVC(X,N)   ( ( (X) + (N) -1 ) / (N) )
  
    const uint32_t TAGE_IM_GEMEINJAHR =    365ul; /* kein Schaltjahr */
    const uint32_t TAGE_IN_4_JAHREN   =   1461ul; /*   4*365 +   1 */

    uint32_t ui32TagN = timeStamp_s/SecondsInDay;
    uint32_t ui32SecondsSinceMidnight = timeStamp_s%SecondsInDay;
    uint32_t ui32Temp;

    /* Schaltjahrregel des Julianischen Kalenders:
       Jedes durch 4 teilbare Jahr ist ein Schaltjahr. */
    if (ui32TagN)
      ui32Temp = 4 * (ui32TagN + TAGE_IM_GEMEINJAHR) / TAGE_IN_4_JAHREN - 1;
    else
      ui32Temp = 0;
    
    if ( ui32Temp < 100)
    {
      psDate->Year = ui32Temp;
      psDate->Month = 0; // 0..11    
      ui32TagN++; // 1..n 2018-09-05-Kd
      
        /* calculate amount of elapsed days since 01/01/2000 */
       ui32TagN -= DIVC( TAGE_IN_4_JAHREN * psDate->Year , 4);            

      /* TagN enthaelt jetzt nur noch die Tage des errechneten Jahres bezogen auf den 1. Januar */    
      if (psDate->Year % 4 == 0 )
      {
        while (ui32TagN > DaysInMonthLeapYear[psDate->Month])
          ui32TagN -= DaysInMonthLeapYear[psDate->Month++];      
      }
      else
      {
        while (ui32TagN > DaysInMonth[psDate->Month])
          ui32TagN -= DaysInMonth[psDate->Month++];     
      }
      
      if (psDate->Month < 12)
        psDate->Month++; // 2018-09-05-Kd ab V0.5 korr. Month from 1..12
          
      psDate->Date = ui32TagN;       
    }
    else
    {
      psDate->Year = 99;
      psDate->Month = 12;     
      psDate->Date = 31;
    }
    psTime->Hours  = ui32SecondsSinceMidnight / 3600;
    psTime->Minutes  = ui32SecondsSinceMidnight % 3600 / 60;
    psTime->Seconds = ui32SecondsSinceMidnight % 60;  
}

void App_RTC_GetDateTime(RTC_DateTypeDef* psDate, RTC_TimeTypeDef* psTime)
{  
  TimerTime_t rtcTime = HW_RTC_GetCalendarValue_s( psDate, psTime); // Seconds since PowerOn starting with 1.1.2000 00:00:00 
  if (gu32TimeDiffSub_s || gu32TimeDiffAdd_s)
  {       
    rtcTime -= gu32TimeDiffSub_s;
    rtcTime += gu32TimeDiffAdd_s;
    
    // 2018-09-05-Kd ab V0.5 entfernt App_RTC_SaveBackupRegister( RtcBackupRamReg_TimeDiffAdd_s, gu32TimeDiffAdd_s);
    // 2018-09-05-Kd ab V0.5 entfernt App_RTC_SaveBackupRegister( RtcBackupRamReg_TimeDiffSub_s, gu32TimeDiffSub_s);    
    
    App_RTC_GetDateTimeFromCalendarValue_s( rtcTime, psDate, psTime);    
  }
}

uint32_t App_RTC_GetDateTimeStamp(void)
{
  uint32_t        dateTimeStamp = 0L;
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  
  /* Get Time and Date*/
  App_RTC_GetDateTime( &sDate, &sTime );
  dateTimeStamp = HW_RTC_GetDateTimeStampFromDateTime(&sDate, &sTime); 
   
  return dateTimeStamp;   
}

bool App_RTC_SetDateTime(RTC_DateTypeDef* psDate, RTC_TimeTypeDef* psTime)
{
  bool            bOk = true;
  
  RTC_DateTypeDef sRtcDate;
  RTC_TimeTypeDef sRtcTime;
    
  if (psDate->Year > 99)
    bOk = false;
  if ((psDate->Month < 1) || (psDate->Month > 12))
    bOk = false;        
  if ((psDate->Date < 1) || (psDate->Date > 31))
    bOk = false;
  if (psTime->Hours > 23)
    bOk = false;
  if (psTime->Minutes > 59)
    bOk = false;
  if (psTime->Seconds > 59)
     bOk = false;   
  if (bOk)
  {
    TimerTime_t rtcTime = HW_RTC_GetCalendarValue_s( &sRtcDate, &sRtcTime); // Seconds since PowerOn starting with 1.1.2000 00:00:00
    TimerTime_t timeNew = HW_RTC_CalcCalendarValue_s( psDate, psTime);
    
    if (timeNew > rtcTime)
    {
      gu32TimeDiffSub_s = 0;
      gu32TimeDiffAdd_s =  timeNew - rtcTime;
      App_RTC_SaveBackupRegister( RtcBackupRamReg_TimeDiffAdd_s, gu32TimeDiffAdd_s); // 2018-09-05-Kd ab V0.5
      App_RTC_SaveBackupRegister( RtcBackupRamReg_TimeDiffSub_s, gu32TimeDiffSub_s); // 2018-09-05-Kd ab V0.5
    }
    else if (timeNew < rtcTime)
    {
      gu32TimeDiffAdd_s = 0;
      gu32TimeDiffSub_s =  rtcTime - timeNew;  
      App_RTC_SaveBackupRegister( RtcBackupRamReg_TimeDiffAdd_s, gu32TimeDiffAdd_s); // 2018-09-05-Kd ab V0.5
      App_RTC_SaveBackupRegister( RtcBackupRamReg_TimeDiffSub_s, gu32TimeDiffSub_s); // 2018-09-05-Kd ab V0.5
    }   
  }
  
  return bOk;
}

/*****END OF FILE****/
