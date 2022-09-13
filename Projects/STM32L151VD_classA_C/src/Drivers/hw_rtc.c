/**============================================================================
* @file      hw_rtc.c
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     rtc definition and initialization for this project

*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File
* V0.02      2017-01-19-Ra      New ST release changes added to GetCalendarValue  	
*
*============================================================================*/

/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "low_power.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* subsecond number of bits */
#define N_PREDIV_S                   10
/* Synchonuous prediv  */
#define PREDIV_S                     ((1<<N_PREDIV_S)-1)
/* Asynchonuous prediv   */
#define PREDIV_A                     (1<<(15-N_PREDIV_S))-1
/* RTC Time base in us */
#define USEC_NUMBER                  1000000
#define MSEC_NUMBER                  (USEC_NUMBER/1000)
#define RTC_ALARM_TIME_BASE          (USEC_NUMBER>>N_PREDIV_S)
/* RTC tick2ms and viceversa convertion factors */
#define COMMON_FACTOR                3
#define CONV_NUMER                   (MSEC_NUMBER>>COMMON_FACTOR)
#define CONV_DENOM                   (1<<(N_PREDIV_S-COMMON_FACTOR))

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*!
 * \brief Indicates if the RTC is already Initalized or not
 */
static bool HW_RTC_Initalized = false;
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
 * Number of days in a standard year
 */
static const uint16_t DaysInYear = 365;
/*!
 * Number of days in a leap year
 */
static const uint16_t DaysInLeapYear = 366;
/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 * Set with the HW_RTC_SetTimerContext function
 * Value is kept as a Reference to calculate alarm
 */
RtcTimerContext_t RtcTimerContext;
/*!
 * RTC Handle instance
 */
RTC_HandleTypeDef hrtc={0};

/* Private function prototypes -----------------------------------------------*/
static void HW_RTC_SetConfig( bool bBackupPowerOn );
static void HW_RTC_SetAlarmConfig( void );

/* Exported functions ---------------------------------------------------------*/
/*!
 * @brief Initializes the RTC timer
 * @note The timer is based on the RTC
 * @param none
 * @retval none
 */
void HW_RTC_Init( bool bBackupPowerOn )
{
  if( HW_RTC_Initalized == false )
  {
    HW_RTC_SetConfig( bBackupPowerOn);
    HW_RTC_SetAlarmConfig( );
    HW_RTC_SetTimerContext( );
    HW_RTC_Initalized = true;
  }
}

/*!
 * @brief Get RTC time and date
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval success
 */
bool HW_RTC_GetDateTime(RTC_DateTypeDef* psDate, RTC_TimeTypeDef* psTime)
{
  bool     bOk = false;
  uint32_t first_read;
  
  /* Get Time and Date*/
  if (HAL_RTC_GetTime( &hrtc, psTime, RTC_FORMAT_BIN ) == HAL_OK)
  {
    bOk = true;
     /* make sure it is correct due to asynchronus nature of RTC*/
    do {
      first_read = psTime->SubSeconds;
      HAL_RTC_GetDate( &hrtc, psDate, RTC_FORMAT_BIN );
      HAL_RTC_GetTime( &hrtc, psTime, RTC_FORMAT_BIN );
    } while (first_read != psTime->SubSeconds); 
  }
  
  return bOk;
}

/*!
 * @brief a delay of delay ms by polling RTC
 * @param delay in ms
 * @retval none
 */
void HW_RTC_DelayMs( uint32_t delay )
{
  TimerTime_t delayValue = 0;
  TimerTime_t timeout = 0;

  delayValue = HW_RTC_ms2Tick( delay );

  /* Wait delay ms */
  timeout = HW_RTC_GetTimerValue( );
  while( ( ( HW_RTC_GetTimerValue( ) - timeout ) ) < delayValue )
  {
    __NOP( );
  }
}

/*!
 * @brief set Time Reference set also the RTC_DateStruct and RTC_TimeStruct
 * @param none
 * @retval Timer Value
 */
uint32_t HW_RTC_SetTimerContext( void )
{
  RtcTimerContext.Rtc_Time = HW_RTC_GetCalendarValue( &RtcTimerContext.RTC_Calndr_Date, &RtcTimerContext.RTC_Calndr_Time );
  return ( uint32_t ) RtcTimerContext.Rtc_Time;
}

/*!
 * @brief Get the RTC timer Reference
 * @param none
 * @retval Timer Value in  Ticks
 */
uint32_t HW_RTC_GetTimerContext( void )
{
  return (uint32_t) RtcTimerContext.Rtc_Time;
}

/*!
 * @brief Get the RTC timer value
 * @param none
 * @retval RTC Timer value in ticks
 */
uint32_t HW_RTC_GetTimerValue( void )
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;
  
  uint32_t CalendarValue = (uint32_t) HW_RTC_GetCalendarValue(&RTC_DateStruct, &RTC_TimeStruct );

  return( CalendarValue );
}

/*!
 * @brief Get the RTC timer elapsed time since the last Alarm was set
 * @param none
 * @retval RTC Elapsed time in ticks
 */
uint32_t HW_RTC_GetTimerElapsedTime( void )
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;
  
  TimerTime_t CalendarValue = HW_RTC_GetCalendarValue(&RTC_DateStruct, &RTC_TimeStruct );

  return( ( uint32_t )( CalendarValue - RtcTimerContext.Rtc_Time ));
}

/*!
 * @brief converts time in ms to time in ticks
 * @param [IN] time in milliseconds
 * @retval returns time in timer ticks
 */
uint32_t HW_RTC_ms2Tick( TimerTime_t timeMicroSec )
{
  return ( uint32_t) ( ( ((uint64_t)timeMicroSec) * CONV_DENOM ) / CONV_NUMER );
}

/*!
 * @brief converts time in ticks to time in ms
 * @param [IN] time in timer ticks
 * @retval returns time in milliseconds
 */
TimerTime_t HW_RTC_Tick2ms( uint32_t tick )
{
  return  ( ( (uint64_t)( tick )* CONV_NUMER ) / CONV_DENOM );
}

/*!
* @brief calc date time to s since 1.1.2000 00:00:00
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval time in s
 */
TimerTime_t HW_RTC_CalcCalendarValue_s( RTC_DateTypeDef* RTC_DateStruct, RTC_TimeTypeDef* RTC_TimeStruct )
{
  TimerTime_t calendarValue = 0;  
  
#define  DAYS_IN_LEAP_YEAR (uint32_t) 366

#define  DAYS_IN_YEAR      (uint32_t) 365

#define  SECONDS_IN_1DAY   (uint32_t) 86400

#define  SECONDS_IN_1HOUR   (uint32_t) 3600

#define  SECONDS_IN_1MINUTE   (uint32_t) 60

#define  MINUTES_IN_1HOUR    (uint32_t) 60

#define  HOURS_IN_1DAY      (uint32_t) 24

#define  DAYS_IN_MONTH_CORRECTION_NORM     ((uint32_t) 0x99AAA0 )
#define  DAYS_IN_MONTH_CORRECTION_LEAP     ((uint32_t) 0x445550 )


/* Calculates ceiling(X/N) */
#define DIVC(X,N)   ( ( (X) + (N) -1 ) / (N) )
  uint32_t correction;
  
  /* calculte amount of elapsed days since 01/01/2000 */
  calendarValue= DIVC( (DAYS_IN_YEAR*3 + DAYS_IN_LEAP_YEAR)* RTC_DateStruct->Year , 4);

  correction = ( (RTC_DateStruct->Year % 4) == 0 ) ? DAYS_IN_MONTH_CORRECTION_LEAP : DAYS_IN_MONTH_CORRECTION_NORM ;
 
  calendarValue +=( DIVC( (RTC_DateStruct->Month-1)*(30+31) ,2 ) - (((correction>> ((RTC_DateStruct->Month-1)*2) )&0x3)));

  calendarValue += (RTC_DateStruct->Date -1);
  
  /* convert from days to seconds */
  calendarValue *= SECONDS_IN_1DAY; 

  calendarValue += ( ( uint32_t )RTC_TimeStruct->Seconds + 
                     ( ( uint32_t )RTC_TimeStruct->Minutes * SECONDS_IN_1MINUTE ) +
                     ( ( uint32_t )RTC_TimeStruct->Hours * SECONDS_IN_1HOUR ) ) ;
                     

//  // DANI org
//  uint32_t i = 0;
//  
//  /* years (calc valid up to year 2099)*/
//  for( i = 0; i < RTC_DateStruct->Year; i++ )
//  {
//    if( (i % 4) == 0 )
//    {
//      calendarValue += DaysInLeapYear * SecondsInDay;
//    }
//    else
//    {
//      calendarValue += DaysInYear * SecondsInDay;
//    }
//  }

//  /* months (calc valid up to year 2099)*/
//  if(( (RTC_DateStruct->Year % 4) == 0 ) )
//  {
//    for( i = 0; i < ( RTC_DateStruct->Month - 1 ); i++ )
//    {
//      calendarValue += DaysInMonthLeapYear[i] * SecondsInDay;
//    }
//  }
//  else
//  {
//    for( i = 0;  i < ( RTC_DateStruct->Month - 1 ); i++ )
//    {
//      calendarValue += DaysInMonth[i] * SecondsInDay;
//    }
//  }

//  /* days */
//  calendarValue += ( ( uint32_t )RTC_TimeStruct->Seconds + 
//                     ( ( uint32_t )RTC_TimeStruct->Minutes * SecondsInMinute ) +
//                     ( ( uint32_t )RTC_TimeStruct->Hours * SecondsInHour ) + 
//                     ( ( uint32_t )((RTC_DateStruct->Date - 1) * SecondsInDay ) ) ); // 2018-09-05-Kd ab V.05
  
  return calendarValue;
}


/*!
* @brief get current time from calendar in s since 1.1.2000 00:00:00
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval time in s
 */
TimerTime_t HW_RTC_GetCalendarValue_s( RTC_DateTypeDef* RTC_DateStruct, RTC_TimeTypeDef* RTC_TimeStruct )
{ 
  /* Get Time and Date*/
  HW_RTC_GetDateTime( RTC_DateStruct, RTC_TimeStruct);
 
  
  return HW_RTC_CalcCalendarValue_s( RTC_DateStruct, RTC_TimeStruct );
}

/*!
 * @brief get current time from calendar in Unix time -> Seconds since 1.1.1970 00:00:00 
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval time in s
 */
TimerTime_t HW_RTC_GetCalendarUnix( void )
{
  RTC_DateTypeDef RTC_DateStruct;
  RTC_TimeTypeDef RTC_TimeStruct;
  TimerTime_t calendarValue;

  calendarValue = HW_RTC_GetCalendarValue_s( &RTC_DateStruct, &RTC_TimeStruct ); // Seconds since 1.1.2000 00:00:00 (running since startup with 1.1.2016 00:00:00)
  
  calendarValue += 946684800L;
  
  return calendarValue;
}

/*!
 * @brief get current time from calendar in ticks
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval time in ticks
 */
TimerTime_t HW_RTC_GetCalendarValue( RTC_DateTypeDef* RTC_DateStruct, RTC_TimeTypeDef* RTC_TimeStruct )
{
  TimerTime_t calendarValue;

  calendarValue = HW_RTC_GetCalendarValue_s( RTC_DateStruct, RTC_TimeStruct );
  
  calendarValue = (calendarValue<<N_PREDIV_S) + ( PREDIV_S - RTC_TimeStruct->SubSeconds);

  return( calendarValue );
}

/**
  * @brief          Gets DateTimeStamp (Microsoft Windows 32-bit date and time format) from Date and Time
  * @param[in]		  psDate as RTC_DateTypeDef*
  * @param[in]	    psTime as RTC_TimeTypeDef*
  * @retval         dateTimeStamp as uint32_t
  */
uint32_t HW_RTC_GetDateTimeStampFromDateTime(RTC_DateTypeDef *psDate, RTC_TimeTypeDef *psTime)
{
  uint32_t dateTimeStamp;
  // The time stamp is packed in accordance to the Microsoft Windows?32-bit date and time format
  dateTimeStamp = psDate->Year + 20;                        // year relative to 1980 (range 0 - 119) 
  dateTimeStamp = (dateTimeStamp<<4) | psDate->Month;       // month (range 1 - 12)
  dateTimeStamp = (dateTimeStamp<<5) | psDate->Date;        // day (range 1 - 31)
  dateTimeStamp = (dateTimeStamp<<5) | psTime->Hours;       // hour (range 0 - 23)
  dateTimeStamp = (dateTimeStamp<<6) | psTime->Minutes;     // minute (range 0 - 59)
  dateTimeStamp = (dateTimeStamp<<5) | (psTime->Seconds>>1);// second (range 0 - 29 in 2-second intervals) 

  return dateTimeStamp;  
}

/* Private functions ---------------------------------------------------------*/
/*!
 * @brief Configures the RTC timer
 * @note The timer is based on the RTC
 * @param bBackupPowerOn
 * @retval none
 */
static void HW_RTC_SetConfig( bool bBackupPowerOn)
{


  hrtc.Instance = RTC;

  hrtc.Init.HourFormat     = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv   = PREDIV_A; /* RTC_ASYNCH_PREDIV; */
  hrtc.Init.SynchPrediv    = PREDIV_S; /* RTC_SYNCH_PREDIV; */
  hrtc.Init.OutPut         = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;

  if (bBackupPowerOn)
  {
    // When we got a  POR/PDR reset -> reinit RTC
    
    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
    
    HAL_RTC_Init( &hrtc ); // -> while writing the RTC Register the time will be stopped for about 0.5s -> generates a time shift for each Power-On
   
    // Monday 1st January 2018
    RTC_DateStruct.Year = 18;
    RTC_DateStruct.Month = RTC_MONTH_JANUARY;
    RTC_DateStruct.Date = 1;
    RTC_DateStruct.WeekDay = RTC_WEEKDAY_MONDAY;
    HAL_RTC_SetDate(&hrtc , &RTC_DateStruct, RTC_FORMAT_BIN);
    
    //at 0:0:0
    RTC_TimeStruct.Hours = 0;
    RTC_TimeStruct.Minutes = 0;

    RTC_TimeStruct.Seconds = 0;
    RTC_TimeStruct.TimeFormat = 0;
    RTC_TimeStruct.SubSeconds = 0;
    RTC_TimeStruct.StoreOperation = RTC_DAYLIGHTSAVING_NONE;
    RTC_TimeStruct.DayLightSaving = RTC_STOREOPERATION_RESET;
    
    HAL_RTC_SetTime(&hrtc , &RTC_TimeStruct, RTC_FORMAT_BIN);
  }
  else
  {
    // The battery backedup RTC will keep the config. After a battery-change, the HAL_RTC_SetTime() in the UI.c will also do the HAL_RTC_Init()-Stuff  
    HAL_RTC_MspInit(&hrtc); // activates the RCC clock path to the RTC    
  }
  
 /*Enable Direct Read of the calendar registers (not through Shadow) */
  HAL_RTCEx_EnableBypassShadow(&hrtc);
}


/**
  * @brief          Gets DateTimeStamp from RTC
  * @retval         dateTimeStamp as uint32_t
  */
uint32_t HW_RTC_GetDateTimeStamp(void)
{
  uint32_t        dateTimeStamp = 0L;
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  
  /* Get Time and Date*/
  if (HW_RTC_GetDateTime( &sDate, &sTime ))
    dateTimeStamp = HW_RTC_GetDateTimeStampFromDateTime(&sDate, &sTime); 
   
  return dateTimeStamp; 
}

/** 
  * @brief          Set the data and time to the RTC    ATTENTION : LoRaMAc uses Timestamps and all Timers -> when used -> NVIC_SystemReset() -> Or use a Timediff saved in BackupRam
  * @param[in]		  psDate as RTC_DateTypeDef*
  * @param[in]		  psTime as RTC_TimeTypeDef*
  * @retval	        None
  */
void HW_RTC_SetDateTime(RTC_DateTypeDef *psDate, RTC_TimeTypeDef *psTime)
{
    // disable backup domain write protection          
    // HAL_PWREx_EnableBkUpReg();
    HAL_PWR_EnableBkUpAccess();      // set PWR->CR.dbp = 1;  
    HAL_RTC_SetTime(&hrtc, psTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, psDate, RTC_FORMAT_BIN);
    // enable backup domain write protection
    HAL_PWR_DisableBkUpAccess();      // reset PWR->CR.dbp = 0;
}

/*!
 * @brief configure alarm at init
 * @param none
 * @retval none
 */
static void HW_RTC_SetAlarmConfig( void )
{
  HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
}

/* MspInit and DeInit function definitions -----------------------------------*/
/**
  * @brief RTC MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();
  __NOP( );
  __NOP( );  
  
  /* configure the interrupt for the alarms */
  HAL_NVIC_SetPriority(RTC_Alarm_IRQn, IRQ_HIGH_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

/**
  * @brief RTC MSP De-Initialization 
  *        This function freeze the hardware resources used in RTC
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /* Reset peripherals */
  __HAL_RCC_RTC_DISABLE();
}

/*****END OF FILE****/
