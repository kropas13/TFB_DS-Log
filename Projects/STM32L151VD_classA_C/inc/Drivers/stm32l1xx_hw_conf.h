/**============================================================================
* @file      stm32l1xx_hw_conf.h
* @date      2016-12-05
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     contains HW configuration Macros and Constants for Sense 2
*            
* VERSION:   
* 
* V0.01      2016-12-05-Ra      Create File + adapted defines to HW				
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L1xx_HW_CONF_H__
#define __STM32L1xx_HW_CONF_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
 
/* SW Update Input Pin */
#define BOOT_CUSTOM_Pin                           GPIO_PIN_15
#define BOOT_CUSTOM_GPIO_Port                     GPIOE
   
/*  HSE OSC */
#define HSE_IN_PORT                               GPIOH
#define HSE_IN_PIN                                GPIO_PIN_0
#define HSE_OUT_PORT                              GPIOH
#define HSE_OUT_PIN                               GPIO_PIN_1
   
/*  LED Constants */
#define LED_GREEN_PIN                             GPIO_PIN_5 //LED green
#define LED_GREEN_GPIO_PORT                       GPIOD
#define LED_RED_PIN                               GPIO_PIN_6 //LED red
#define LED_RED_GPIO_PORT                         GPIOD 
 
   
/* Digital sense pins */	 
#define EXT_5V_USB_DETECT_Pin                     GPIO_PIN_5
#define EXT_5V_USB_DETECT_GPIO_Port               GPIOC

/* BUTTON & EXT. CONNECT Constants */
#define BUTTON_PIN                                GPIO_PIN_0  //BUTTON with WakeUp function
#define BUTTON_GPIO_PORT                          GPIOA

/* AI Constants */  
#define AI_IMP_U_Pin                              GPIO_PIN_6 //ADC_IN6
#define AI_IMP_U_GPIO_Port                        GPIOA
#define AI_IMP_I1_Pin                             GPIO_PIN_7 //ADC_IN7
#define AI_IMP_I1_GPIO_Port                       GPIOA
#define AI_IMP_I2_Pin                             GPIO_PIN_4 //ADC_IN14
#define AI_IMP_I2_GPIO_Port                       GPIOC
#define AI_Vref_2048mV_Pin                        GPIO_PIN_1  //ADC_IN9
#define AI_Vref_2048mV_GPIO_Port                  GPIOB
#define AI_VBAT_PIN                               GPIO_PIN_0  //ADC_IN10
#define AI_VBAT_GPIO_PORT                         GPIOC

#define MEAS_VBAT_ACTIVATE_Pin                    GPIO_PIN_2
#define MEAS_VBAT_ACTIVATE_GPIO_Port              GPIOC

/* DAC Constants */
#define IMP_DAC_SIGNAL_Pin                        GPIO_PIN_5
#define IMP_DAC_SIGNAL_GPIO_Port                  GPIOA    

/* Digital Inputs Constants */
#define IMP_U_COMP_Pin                            GPIO_PIN_8
#define IMP_U_COMP_GPIO_Port                      GPIOB
#define IMP_I_COMP_Pin                            GPIO_PIN_9
#define IMP_I_COMP_GPIO_Port                      GPIOB

/* Ext. SPI3 ADC Constants */
#define ADC_SPI3_SCLK_Pin                         GPIO_PIN_3
#define ADC_SPI3_SCLK_GPIO_Port                   GPIOB
#define ADC_SPI3_MISO_Pin                         GPIO_PIN_4
#define ADC_SPI3_MISO_GPIO_Port                   GPIOB
#define ADC_SPI3_MOSI_Pin                         GPIO_PIN_5
#define ADC_SPI3_MOSI_GPIO_Port                   GPIOB 
#define ADC_SPI3_CS_Pin                           GPIO_PIN_4
#define ADC_SPI3_CS_GPIO_Port                     GPIOA
#define _ADC_DATA_READY_DI_Pin                    GPIO_PIN_1
#define _ADC_DATA_READY_DI_GPIO_Port              GPIOA
#define ADC_START_DO_Pin                          GPIO_PIN_2
#define ADC_START_DO_GPIO_Port                    GPIOA
#define _ADC_RESET_DO_Pin                         GPIO_PIN_3
#define _ADC_RESET_DO_GPIO_Port                   GPIOA

/* Ext. I2C1 Cold Junction Temp. Sensor  */
#define I2C1_TEMP_SCL_Pin                         GPIO_PIN_6
#define I2C1_TEMP_SCL_GPIO_Port                   GPIOB
#define I2C1_TEMP_SDA_Pin                         GPIO_PIN_7
#define I2C1_TEMP_SDA_GPIO_Port                   GPIOB

/* Shift Register */
#define SHFT_SERIAL_DATA_Pin                      GPIO_PIN_4
#define SHFT_SERIAL_DATA_GPIO_Port                GPIOE    
/* Shift1 Ctrl CNTRL TEMP/UI/IMP MEAS SETUP */
#define SHFT1_STORE_CLR_Pin                       GPIO_PIN_13
#define SHFT1_STORE_CLR_GPIO_Port                 GPIOD
#define SHFT1_REG_CLK_Pin                         GPIO_PIN_14
#define SHFT1_REG_CLK_GPIO_Port                   GPIOD
#define SHFT1_SERIAL_DATA_Pin                     GPIO_PIN_15
#define SHFT1_SERIAL_DATA_GPIO_Port               GPIOD
#define SHFT1_ENABLE_Pin                          GPIO_PIN_6
#define SHFT1_ENABLE_GPIO_Port                    GPIOC
#define SHFT1_STORE_CLK_Pin                       GPIO_PIN_7
#define SHFT1_STORE_CLK_GPIO_Port                 GPIOC    
/* Shift2 CNTRL CORR/IMP-GAIN MEAS SETUP */
#define SHFT2_STORE_CLR_Pin                       GPIO_PIN_2
#define SHFT2_STORE_CLR_GPIO_Port                 GPIOE
#define SHFT2_REG_CLK_Pin                         GPIO_PIN_3
#define SHFT2_REG_CLK_GPIO_Port                   GPIOE
#define SHFT2_SERIAL_DATA_Pin                     GPIO_PIN_4
#define SHFT2_SERIAL_DATA_GPIO_Port               GPIOE
#define SHFT2_ENABLE_Pin                          GPIO_PIN_0
#define SHFT2_ENABLE_GPIO_Port                    GPIOE
#define SHFT2_STORE_CLK_Pin                       GPIO_PIN_1
#define SHFT2_STORE_CLK_GPIO_Port                 GPIOE

/* SDIO Card */
#define SDIO_CDS_Pin                              GPIO_PIN_9 // Card Detect Input
#define SDIO_CDS_GPIO_Port                        GPIOC
#define SDIO_D0_DIO_Pin                           GPIO_PIN_8
#define SDIO_D0_DIO_GPIO_Port                     GPIOC
#define SDIO_CMD_DO_Pin                           GPIO_PIN_2
#define SDIO_CMD_DO_GPIO_Port                     GPIOD
#define SDIO_CK_DO_Pin                            GPIO_PIN_12
#define SDIO_CK_DO_GPIO_Port                      GPIOC

/* USB Constants */  
#define USB_DM_PIN                                GPIO_PIN_11 //USB_N
#define USB_DM_GPIO_PORT                          GPIOA
#define USB_DP_PIN                                GPIO_PIN_12 //USB_P
#define USB_DP_GPIO_PORT                          GPIOA


/* Power Control Outputs */
#define DCDC_PS_MODE_Pin                          GPIO_PIN_3
#define DCDC_PS_MODE_GPIO_Port                    GPIOC

/* VCC Enable Constants */
#define EN_LORA_PIN                               GPIO_PIN_10   // inverted
#define EN_LORA_GPIO_PORT                         GPIOD
#define EN_SYSTEM_PIN                             GPIO_PIN_1  // inverted
#define EN_SYSTEM_GPIO_PORT                       GPIOC

	 
/* LORA I/O definition */
#define RADIO_RESET_PORT                          GPIOD
#define RADIO_RESET_PIN                           GPIO_PIN_8
#define RADIO_MOSI_PORT                           GPIOD
#define RADIO_MOSI_PIN                            GPIO_PIN_4
#define RADIO_MISO_PORT                           GPIOD
#define RADIO_MISO_PIN                            GPIO_PIN_3
#define RADIO_SCLK_PORT                           GPIOD
#define RADIO_SCLK_PIN                            GPIO_PIN_1
#define RADIO_NSS_PORT                            GPIOD
#define RADIO_NSS_PIN                             GPIO_PIN_0
#define RADIO_TCXO_VCC_PIN                        GPIO_PIN_9                    
#define RADIO_TCXO_VCC_PORT                       GPIOD

#define RADIO_DIO_0_PORT                          GPIOB
#define RADIO_DIO_0_PIN                           GPIO_PIN_10
#define RADIO_DIO_1_PORT                          GPIOB
#define RADIO_DIO_1_PIN                           GPIO_PIN_11
#define RADIO_DIO_2_PORT                          GPIOB
#define RADIO_DIO_2_PIN                           GPIO_PIN_12
#define RADIO_DIO_3_PORT                          GPIOB
#define RADIO_DIO_3_PIN                           GPIO_PIN_13
#define RADIO_DIO_4_PORT                          GPIOB
#define RADIO_DIO_4_PIN                           GPIO_PIN_14
#define RADIO_DIO_5_PORT                          GPIOB
#define RADIO_DIO_5_PIN                           GPIO_PIN_15


/* SWD I/O definition added by 2016-11-10-Ra  */
#define SW_DAT_Pin                                GPIO_PIN_13     
#define SW_DAT_Port                               GPIOA
#define SW_CLK_Pin                                GPIO_PIN_14     
#define SW_CLK_Port                               GPIOA

/* Definition for USART1 Pins - RS485 */
#define RS485_USART1_TX_Pin                       GPIO_PIN_9
#define RS485_USART1_TX_GPIO_Port                 GPIOA
#define RS485_USART1_RX_Pin                       GPIO_PIN_10
#define RS485_USART1_RX_GPIO_Port                 GPIOA
#define RS485_USART1_DE_Pin                       GPIO_PIN_8
#define RS485_USART1_DE_GPIO_Port                 GPIOA


/* Definition for USART3 Pins - Console(vcom) and BT (Bluetooth modul) */
#define BT_USART3_TX_Pin                          GPIO_PIN_10
#define BT_USART3_TX_GPIO_Port                    GPIOC
#define BT_USART3_RX_Pin                          GPIO_PIN_11
#define BT_USART3_RX_GPIO_Port                    GPIOC
#define BT_USART3_CTS_Pin                         GPIO_PIN_11
#define BT_USART3_CTS_GPIO_Port                   GPIOD
#define BT_USART3_RTS_Pin                         GPIO_PIN_12
#define BT_USART3_RTS_GPIO_Port                   GPIOD
/* Bluetooth Control */
#define BT_nAutoRUN_Pin                           GPIO_PIN_7
#define BT_nAutoRUN_GPIO_Port                     GPIOE
#define BT_nRESET_Pin                             GPIO_PIN_8
#define BT_nRESET_GPIO_Port                       GPIOE
#define BT_OTA_APPDL_Pin                          GPIO_PIN_9
#define BT_OTA_APPDL_GPIO_Port                    GPIOE
#define BT_SIO_03_Pin                             GPIO_PIN_10
#define BT_SIO_03_GPIO_Port                       GPIOE

/* Revision Input Pins */
#define REV_BIT0_Pin                              GPIO_PIN_11
#define REV_BIT0_GPIO_Port                        GPIOE
#define REV_BIT1_Pin                              GPIO_PIN_12
#define REV_BIT1_GPIO_Port                        GPIOE
#define REV_BIT2_Pin                              GPIO_PIN_13
#define REV_BIT2_GPIO_Port                        GPIOE
#define REV_BIT3_Pin                              GPIO_PIN_14
#define REV_BIT3_GPIO_Port                        GPIOE

/* Unused Pins */
#define Boot1_TP403_Pin                           GPIO_PIN_2
#define Boot1_TP403_GPIO_Port                     GPIOB
#define TP404_Pin                                 GPIO_PIN_0
#define TP404_GPIO_Port                           GPIOB
#define TP405_Pin                                 GPIO_PIN_15
#define TP405_GPIO_Port                           GPIOA
#define TP408_Pin                                 GPIO_PIN_7
#define TP408_GPIO_Port                           GPIOD
#define TP410_Pin                                 GPIO_PIN_13
#define TP410_GPIO_Port                           GPIOC
#define TP411_Pin                                 GPIO_PIN_5
#define TP411_GPIO_Port                           GPIOE
#define TP412_Pin                                 GPIO_PIN_6
#define TP412_GPIO_Port                           GPIOE

/* --------------------------- RTC HW definition -------------------------------- */
#define RTC_OUTPUT                                DBG_RTC_OUTPUT

/* --------------------------- DEBUG redefinition -------------------------------*/
#define   __HAL_RCC_DBGMCU_CLK_ENABLE()
#define   __HAL_RCC_DBGMCU_CLK_DISABLE()

#ifdef __cplusplus
}
#endif

#endif /* __STM32L1xx_HW_CONF_H__ */

/*****END OF FILE****/
