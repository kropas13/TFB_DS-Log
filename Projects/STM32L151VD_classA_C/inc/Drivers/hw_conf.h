/**============================================================================
* @file      hw_conf.h
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     hw conf definitions and macros (hal and hw conf files are merged here)
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 			
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_CONF_H__
#define __HW_CONF_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* --------Preprocessor compile swicthes------------ */
/* MAIN Switches to be set so everything (comtac side) works, for now here!! */
#define USE_STM32L1XX

/* debug swicthes in debug.h */
//#define DEBUG
//#define TRACE   
   
/* uncomment below line to never enter lowpower modes in main.c*/
//#define LOW_POWER_DISABLE

#define  USE_COM_ECHO_ON              0
#define  USE_VCOM_ON_RS485_USART1     1
#define  USE_VCOM_ON_X403_BT_USART3   0
   
#define   USE_USART3_DMA              1 // falls 0 -> wird während dem Senden max. 1 Byte empfangen !!
   
#define   USE_HSE // use external Quarz and switch off the HSI (Osci needs about 400..700uA, the HSI takes about 100uA)
#define   USE_HSE_PINS_AI_IN_STOP // Switch the HSE Pins to AI during Stop (save power, about 45uA)
   
#if defined(USE_HSE_PINS_AI_IN_STOP) && !defined(USE_HSE)
  #error "Switch on USE_HSE"
#endif
	 
/* SX1272 turned off when not needed */
#define   DEF_RF_ON_OFF	 	

/*Periphery switches */
#define   DEF_USB
#define   DEF_RS485
#define   DEF_TCXO





/* timing test switch */
//#define DEF_TIMINGTEST


/* Includes ------------------------------------------------------------------*/
#ifdef USE_STM32L1XX  
  #include "stm32l1xx_hal.h"
  #include "stm32l1xx_hal_conf.h"
  #include "stm32l1xx_hw_conf.h"
   
  /*!
   *  \brief Unique Devices IDs register set ( STM32L1xxx )
   */   
  #ifndef         UID_BASE
    #define       UID_BASE              ((uint32_t)0x1FF800D0U)        /*!< Unique device ID register base address for Cat.3, Cat.4, Cat.5 and Cat.6 devices */
  #endif
  #define         ID1                   ( UID_BASE + 0x00 )
  #define         ID2                   ( UID_BASE + 0x04 )
  #define         ID3                   ( UID_BASE + 0x14 )   
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

#ifdef __cplusplus
}
#endif

#endif /* __HW_CONF_H__ */

/*****END OF FILE****/
