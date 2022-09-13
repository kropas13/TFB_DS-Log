/**============================================================================
* @file      sw_timer.h
* @date      2016-11-18
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header of sw_timer.c file 

*            
* VERSION:   
* 
* V0.01      2016-11-18-Ra      Create File  	
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SW_TIMER_H__
#define __SW_TIMER_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "utilities.h"
/* Exported types ------------------------------------------------------------*/
//change the name according to the use!!
typedef enum
{
  SW_TIM_DUMMY0 = 0,	
  SW_TIM_DUMMY1,
  SW_TIM_DUMMY2,
  SW_TIM_DUMMY3,
  SW_TIM_DUMMY4,
  SW_TIM_DUMMY5,
	SW_TIM_DUMMY6,
  SW_TIM_DUMMY7,	
} Defined_Sw_Timers_t;


/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern bool bTIM10Tick;
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

/*!
 * @brief Initializes all 10 possible SW timers
 */
void SwTimerInitAll(void);

/*!
 * @brief Sets one of the SW timers
 * @param TimerName  [IN]  Name of timer as defined in enum at h. file
 *        time_in_ms [IN]  timer in ms (smallest tick is 10ms)
 *        *CB_func   [IN]  callback fuction at timer done
 */
void SwTimerSet(Defined_Sw_Timers_t TimerName, uint32_t time_in_ms, void (*CB_func)());

/*!
 * @brief TIM10 Update event handler
 */
void HwSw_TimerIrq(void);

#ifdef __cplusplus
}
#endif

#endif /* __SW_TIMER_H__ */
