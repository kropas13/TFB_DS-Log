/**============================================================================
* @file      hw.h
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     main h file for all hw modules
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 
* V0.02      2016-12-28-Ra 			hw_htim10 added
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_H__
#define __HW_H__

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>	
#include <stdlib.h>	 
//hw conf
#include "hw_conf.h"	 
//hw files
#include "hw_gpio.h"
#include "hw_tim.h"
#include "hw_adc.h"
#include "hw_dac.h"  
#include "hw_rtc.h"
#include "hw_uart.h"
#include "hw_spi.h"
#include "hw_ios.h"
#include "hw_msp.h"	 
#include "hw_iwdg.h"	
#include "hw_sdio.h"   
#include "hw_i2c.h"  
//debug 
#include "debug.h"
	 
/* Exported types ------------------------------------------------------------*/
/*!
 * \brief used at low_power.c to avoid entering into stop mode
 */
typedef enum
  {
    e_LOW_POWER_RTC        =  (1<<0), /* RTC */
    e_LOW_POWER_I2C        =  (1<<1), /* I2C */
    e_LOW_POWER_UART_BT    =  (1<<2), /* UART BT */
    e_LOW_POWER_SPI        =  (1<<3), /* SPI */
	  e_LOW_POWER_TIM10      =  (1<<4), /* TIM10 */
		e_LOW_POWER_USB        =  (1<<5), /* USB */
    e_LOW_POWER_3V3_SYS    =  (1<<6)  /* 3V3_Sys */  
  } e_LOW_POWER_State_Id_t;
  
/* Exported constants --------------------------------------------------------*/ 
#define IRQ_VERY_HIGH_PRIORITY       1
#define IRQ_HIGH_PRIORITY            3
#define IRQ_MEDIUM_PRIORITY          7
#define IRQ_LOW_PRIORITY             15     

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 	
/*!
 * \brief Initializes the boards peripherals.
 */
void HW_Init( bool bBackupPowerOn  );

  /*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */ 
void HW_DeInit( void );

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t HW_GetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID 
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void HW_GetUniqueId( uint8_t *id );

/*!
 * \brief Gets the board 96 bits unique chip ID 
 *
 * \param [IN] id Pointer to an array that will contain the Unique chip ID
 */
void HW_GetChipId( uint8_t chipId[12] );

/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t HW_GetBatteryLevel( void );

  /*!
 * \brief Initializes the HW and enters stope mode
 */
void HW_EnterStopMode( void);

/*!
 * \brief Exits stop mode and Initializes the HW
 */
void HW_ExitStopMode( void);

/**
  * @brief Enters Low Power Sleep Mode
  * @note ARM exists the function when waking up
  * @param none
  * @retval none
  */
void HW_EnterSleepMode( void);

#ifdef __cplusplus
}
#endif

#endif /* __HW_H__ */

/*****END OF FILE****/
