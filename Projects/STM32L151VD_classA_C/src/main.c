/**============================================================================
* @file      main.c
* @date      2016-12-07
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
* V0.01            2016-12-07-Ra      Create File 		
* V0.XX(see repo)  2016-12-27-Ra      Merged with loraModem concept of sensile
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "low_power.h"
#include "app.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/                                   
/* Global functions ---------------------------------------------------------*/
/* Timer callback functions ---------------------------------------------------------*/
/* IRQ callback functions ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/	
/*****************************************************************************
*****************************   MAIN   ***************************************
******************************************************************************/

int main( void )
{   
  // this was a dani comment 2016-11-17-Ra // Der Bootloader aktiviert den 3V3 Switcher !
  /* STM32 HAL library initialization*/
  HAL_Init( );
  /* Configure the system clock to 32 MHz*/
  SystemClock_Config( );  
  
  /* Configure the debug mode*/
  DBG_Init( );      
  /* Configure the hardware*/
  HW_Init(__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST));    
  /* Application initialized here */      
  appInit();
  		
  /* MAIN LOOP */
  while( 1 )
  {	
    /* check the different timers and events */	
    appDo();
    /* if an interrupt has occurred after DISABLE_IRQ, it is kept pending 
     * and cortex will not enter low power anyway  */		
    DISABLE_IRQ( );   
    if (!appHasWork()){
#ifndef LOW_POWER_DISABLE
      /* Sleep or Stop */
      LowPower_Handler( );
#endif
    }
    ENABLE_IRQ();
  }
}  

/*****END OF FILE****/
