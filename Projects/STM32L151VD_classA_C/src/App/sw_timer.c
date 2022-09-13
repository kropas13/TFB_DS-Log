/**============================================================================
* @file      sw_timer.c
* @date      2016-11-18
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     software timer implementation using TIM10 as clock (10ms ticks)

*            
* VERSION:   
* 
* V0.01      2016-11-18-Ra      Create File  	
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "sw_timer.h"
  
/* Private typedef -----------------------------------------------------------*/

typedef struct 
{
  uint32_t actual_val_ms;
  uint32_t rel_val_ms;
  void (*CB_function)(void);
}tSwTimer_Struct;


/* Private define ------------------------------------------------------------*/
#define NR_OF_TIMERS          8 //just any number?
#define TIMER_ZERO            NULL
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/  

bool bTIM10Tick = false;
static uint8_t SwTimerIsActive = 0; 
static tSwTimer_Struct sSwTimers[NR_OF_TIMERS];


/* Private function prototypes -----------------------------------------------*/
/* Exported functions ---------------------------------------------------------*/


/*!
 * @brief Initializes all 10 possible SW timers
 * @note The timer clock is the TIM10 update interrupt (10ms tick) 
 * @param none
 * @retval none
 */
void SwTimerInitAll(void)
{
  uint8_t i;
  for (i= 0; i < NR_OF_TIMERS; i++){
    SwTimerSet(i, TIMER_ZERO, NULL);
  }
	SwTimerIsActive = 0;
}

/*!
 * @brief Sets one of the SW timers
 * @note The timer clock is the TIM10 update interrupt (10ms tick)
 * @param TimerName  :  Name of timer as defined in enum at h. file
 *        time_in_ms :  timer in ms (smallest tick is 10ms)
 *        *CB_func   :  callback fuction at timer done
 * @retval none
 */
void SwTimerSet(Defined_Sw_Timers_t TimerName, uint32_t time_in_ms, void (*CB_func) ())
{
  sSwTimers[TimerName].actual_val_ms = time_in_ms/10;   
  sSwTimers[TimerName].rel_val_ms = time_in_ms/10;
  sSwTimers[TimerName].CB_function = CB_func;
	SwTimerIsActive |= (1 << TimerName);
}  

/*!
 * @brief TIM10 Update event handler
 * @note Event comes every 10ms, as long as TIM10 is on
 * @param none
 * @retval none
 */
void HwSw_TimerIrq(void)
{
  //swtimers get actualized here
  uint8_t i;
  bTIM10Tick = true;
  //call sigfox dummy function here
  // or do something on main with the flag
  
  //here we actualize all sw timers
	if (SwTimerIsActive){
    for(i=0; i<NR_OF_TIMERS; i++){
      if (sSwTimers[i].actual_val_ms != 0){
        sSwTimers[i].actual_val_ms--;
      }
      if ((sSwTimers[i].actual_val_ms == 0) && (sSwTimers[i].rel_val_ms != 0)){
	      sSwTimers[i].rel_val_ms = 0;
				SwTimerIsActive &= ~(1<<i);				
        sSwTimers[i].CB_function();
      }  
    }
  }		
}






