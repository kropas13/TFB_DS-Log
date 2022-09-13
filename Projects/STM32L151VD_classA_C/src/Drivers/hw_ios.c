/**============================================================================
* @file      hw_ios.c
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     initialization and definition of sirop I/Os
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 			
*
*============================================================================*/
  /* Includes ------------------------------------------------------------------*/
#include "hw.h"

/* Private typedef -------------------------------------------------------------*/
/* Private define --------------------------------------------------------------*/
/* Private macro ---------------------------------------------------------------*/
/* Private variables -----------------------------------------------------------*/
/*!
 * Flag to indicate if the IOs were initialized
 */
static bool bInitDone = false;

/* Private function prototypes -------------------------------------------------*/
/* IRQ callback functions ------------------------------------------------------*/
/* Exported functions ----------------------------------------------------------*/
/**
  * @brief This function initializes sirop I/Os
  * @param none
  * @retval none
  */
void  HW_IOs_Init( void  )
{
  GPIO_InitTypeDef initStruct={0};
 
  if (!bInitDone)
  {		
 // Init the GPIO pins 
     
// Inputs     
    initStruct.Mode   = GPIO_MODE_INPUT;
    initStruct.Speed  = GPIO_SPEED_MEDIUM;  
    initStruct.Pull   = GPIO_PULLDOWN;
	 // USB
    HW_GPIO_Init(USB_DM_GPIO_PORT, USB_DM_PIN, &initStruct);  // activate PullDown  
    HW_GPIO_Init(USB_DP_GPIO_PORT, USB_DP_PIN, &initStruct);  // activate PullDown    
		
    initStruct.Pull   = GPIO_NOPULL;
       
    HW_GPIO_Init(EXT_5V_USB_DETECT_GPIO_Port, EXT_5V_USB_DETECT_Pin, &initStruct);
				
	 // buttons 
	  HW_GPIO_Init(BUTTON_GPIO_PORT, BUTTON_PIN, &initStruct);          // optional PIN_PULL_DOWN when RXXX is not soldered
   // Custom pin
    HW_GPIO_Init(BOOT_CUSTOM_GPIO_Port, BOOT_CUSTOM_Pin, &initStruct);  
    
// Outputs     
    initStruct.Mode   = GPIO_MODE_OUTPUT_PP;
    
#ifdef DEF_TIMINGTEST
    HW_GPIO_Init(TP404_GPIO_Port,TP404_Pin,&initStruct);
    HW_GPIO_Init(TP405_GPIO_Port,TP405_Pin,&initStruct);
#endif     
    // Power    
    HAL_GPIO_WritePin(DCDC_PS_MODE_GPIO_Port, DCDC_PS_MODE_Pin, GPIO_PIN_RESET); // Power Save Mode enabled at init
    HW_GPIO_Init(DCDC_PS_MODE_GPIO_Port, DCDC_PS_MODE_Pin, &initStruct); 
		
    // VCC System
    HAL_GPIO_WritePin(EN_SYSTEM_GPIO_PORT, EN_SYSTEM_PIN, GPIO_PIN_SET);                   // VCC System off
    HW_GPIO_Init(EN_SYSTEM_GPIO_PORT, EN_SYSTEM_PIN, &initStruct);   
    		        
	  // VCC LORA
    HAL_GPIO_WritePin(EN_LORA_GPIO_PORT, EN_LORA_PIN, GPIO_PIN_SET);                       // VCC LoRa off
    HW_GPIO_Init(EN_LORA_GPIO_PORT, EN_LORA_PIN, &initStruct);     
#if defined (DEF_TCXO)
    HAL_GPIO_WritePin( RADIO_TCXO_VCC_PORT, RADIO_TCXO_VCC_PIN, GPIO_PIN_RESET);                    // LoRa clock off
    HW_GPIO_Init ( RADIO_TCXO_VCC_PORT, RADIO_TCXO_VCC_PIN, &initStruct );   
#endif  	 	
    
	  // LEDs
	  //State LED
    HAL_GPIO_WritePin( LED_GREEN_GPIO_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);                       // LED2 off (State) 
    HW_GPIO_Init(LED_GREEN_GPIO_PORT, LED_GREEN_PIN, &initStruct);      	
    //Link LED	
    HAL_GPIO_WritePin( LED_RED_GPIO_PORT, LED_RED_PIN, GPIO_PIN_RESET);                           // LED1 off (Link)     
    HW_GPIO_Init(LED_RED_GPIO_PORT, LED_RED_PIN, &initStruct);                  
          
    
// Analog inputs
    initStruct.Mode   = GPIO_MODE_ANALOG;
    //initStruct.Pull   = GPIO_NOPULL;
    
	 // Int. ADC
    HW_GPIO_Init(MEAS_VBAT_ACTIVATE_GPIO_Port, MEAS_VBAT_ACTIVATE_Pin, &initStruct);
    HW_GPIO_Init(AI_VBAT_GPIO_PORT, AI_VBAT_PIN, &initStruct);
    HW_GPIO_Init(AI_IMP_U_GPIO_Port, AI_IMP_U_Pin, &initStruct);
    HW_GPIO_Init(AI_IMP_I1_GPIO_Port, AI_IMP_I1_Pin, &initStruct); // 2020-06-18-Kd V1.08 alt war AI_IMP_I2_Pin -> hatte aber keinen Einfluss auf Funktion (Bootloader init. auf AI_IMP_I1_Pin ist auch schon auf analog)
    HW_GPIO_Init(AI_IMP_I2_GPIO_Port, AI_IMP_I2_Pin, &initStruct);
    HW_GPIO_Init(AI_Vref_2048mV_GPIO_Port, AI_Vref_2048mV_Pin, &initStruct);
    // Int. DAC
    HW_GPIO_Init(IMP_DAC_SIGNAL_GPIO_Port, IMP_DAC_SIGNAL_Pin, &initStruct);      
    // Ext. SPI3 ADC
    HW_GPIO_Init(ADC_SPI3_SCLK_GPIO_Port, ADC_SPI3_SCLK_Pin, &initStruct); 
    HW_GPIO_Init(ADC_SPI3_MISO_GPIO_Port, ADC_SPI3_MISO_Pin, &initStruct); 
    HW_GPIO_Init(ADC_SPI3_MOSI_GPIO_Port, ADC_SPI3_MOSI_Pin, &initStruct); 
    HW_GPIO_Init(ADC_SPI3_CS_GPIO_Port, ADC_SPI3_CS_Pin, &initStruct); 
    HW_GPIO_Init(_ADC_DATA_READY_DI_GPIO_Port, _ADC_DATA_READY_DI_Pin, &initStruct); 
    HW_GPIO_Init(ADC_START_DO_GPIO_Port, ADC_START_DO_Pin, &initStruct); 
    HW_GPIO_Init(_ADC_RESET_DO_GPIO_Port, _ADC_RESET_DO_Pin, &initStruct); 
    // Ext. I2C1 Cold Junction Temp. Sensor
    HW_GPIO_Init(I2C1_TEMP_SCL_GPIO_Port, I2C1_TEMP_SCL_Pin, &initStruct);
    HW_GPIO_Init(I2C1_TEMP_SDA_GPIO_Port, I2C1_TEMP_SDA_Pin, &initStruct);
    // Shift Register
    HW_GPIO_Init(SHFT_SERIAL_DATA_GPIO_Port, SHFT_SERIAL_DATA_Pin, &initStruct);  
    // Shift1 Ctrl CNTRL TEMP/UI/IMP MEAS SETUP
    HW_GPIO_Init(SHFT1_STORE_CLR_GPIO_Port, SHFT1_STORE_CLR_Pin, &initStruct);  
    HW_GPIO_Init(SHFT1_REG_CLK_GPIO_Port, SHFT1_REG_CLK_Pin, &initStruct);  
    HW_GPIO_Init(SHFT1_SERIAL_DATA_GPIO_Port, SHFT1_SERIAL_DATA_Pin, &initStruct);  
    HW_GPIO_Init(SHFT1_ENABLE_GPIO_Port, SHFT1_ENABLE_Pin, &initStruct);  
    HW_GPIO_Init(SHFT1_STORE_CLK_GPIO_Port, SHFT1_STORE_CLK_Pin, &initStruct);  
    // Shift2 CNTRL CORR/IMP-GAIN MEAS SETUP
    HW_GPIO_Init(SHFT2_STORE_CLR_GPIO_Port, SHFT2_STORE_CLR_Pin, &initStruct);
    HW_GPIO_Init(SHFT2_REG_CLK_GPIO_Port, SHFT2_REG_CLK_Pin, &initStruct);
    HW_GPIO_Init(SHFT2_SERIAL_DATA_GPIO_Port, SHFT2_SERIAL_DATA_Pin, &initStruct);     
    HW_GPIO_Init(SHFT2_ENABLE_GPIO_Port, SHFT2_ENABLE_Pin, &initStruct);
    HW_GPIO_Init(SHFT2_STORE_CLK_GPIO_Port, SHFT2_STORE_CLK_Pin, &initStruct);
    // SDIO Card
    HW_GPIO_Init(SDIO_CDS_GPIO_Port, SDIO_CDS_Pin, &initStruct);
    HW_GPIO_Init(SDIO_D0_DIO_GPIO_Port, SDIO_D0_DIO_Pin, &initStruct);
    HW_GPIO_Init(SDIO_CMD_DO_GPIO_Port, SDIO_CMD_DO_Pin, &initStruct);
    HW_GPIO_Init(SDIO_CK_DO_GPIO_Port, SDIO_CK_DO_Pin, &initStruct);
    // Digital Inputs Constants
    HW_GPIO_Init(IMP_U_COMP_GPIO_Port, IMP_U_COMP_Pin, &initStruct);
    HW_GPIO_Init(IMP_I_COMP_GPIO_Port, IMP_I_COMP_Pin, &initStruct);
    // Revision Input Pins
    HW_GPIO_Init(REV_BIT0_GPIO_Port, REV_BIT0_Pin, &initStruct);
    HW_GPIO_Init(REV_BIT1_GPIO_Port, REV_BIT1_Pin, &initStruct);
    HW_GPIO_Init(REV_BIT2_GPIO_Port, REV_BIT2_Pin, &initStruct);
    HW_GPIO_Init(REV_BIT3_GPIO_Port, REV_BIT3_Pin, &initStruct);	
    //SPI1 LORA + IOs
    HW_GPIO_Init ( RADIO_MOSI_PORT, RADIO_MOSI_PIN, &initStruct );		
    HW_GPIO_Init ( RADIO_MISO_PORT, RADIO_MISO_PIN, &initStruct ); 
    HW_GPIO_Init ( RADIO_SCLK_PORT, RADIO_SCLK_PIN, &initStruct );
    HW_GPIO_Init ( RADIO_NSS_PORT,  RADIO_NSS_PIN , &initStruct );
    HW_GPIO_Init ( RADIO_RESET_PORT, RADIO_RESET_PIN, &initStruct ); 
    HW_GPIO_Init ( RADIO_DIO_0_PORT, RADIO_DIO_0_PIN, &initStruct );	
    HW_GPIO_Init ( RADIO_DIO_1_PORT, RADIO_DIO_1_PIN, &initStruct );	
    HW_GPIO_Init ( RADIO_DIO_2_PORT, RADIO_DIO_2_PIN, &initStruct );			
    HW_GPIO_Init ( RADIO_DIO_3_PORT, RADIO_DIO_3_PIN, &initStruct );		  	
    HW_GPIO_Init ( RADIO_DIO_4_PORT, RADIO_DIO_4_PIN, &initStruct );	
    HW_GPIO_Init ( RADIO_DIO_5_PORT, RADIO_DIO_5_PIN, &initStruct );		 
    //UART1 RS485 + IOs           
	  HW_GPIO_Init(RS485_USART1_DE_GPIO_Port,RS485_USART1_DE_Pin,&initStruct); 
    HW_GPIO_Init(RS485_USART1_TX_GPIO_Port,RS485_USART1_TX_Pin,&initStruct);
    HW_GPIO_Init(RS485_USART1_RX_GPIO_Port,RS485_USART1_RX_Pin,&initStruct);		      
    // USART3 Pins - Console(vcom) and BT (Bluetooth modul)
    HW_GPIO_Init(BT_USART3_TX_GPIO_Port,BT_USART3_TX_Pin,&initStruct);
    HW_GPIO_Init(BT_USART3_RX_GPIO_Port,BT_USART3_RX_Pin,&initStruct);
    HW_GPIO_Init(BT_USART3_CTS_GPIO_Port,BT_USART3_CTS_Pin,&initStruct);
    HW_GPIO_Init(BT_USART3_RTS_GPIO_Port,BT_USART3_RTS_Pin,&initStruct);
    // Bluetooth Control
    HW_GPIO_Init(BT_nAutoRUN_GPIO_Port,BT_nAutoRUN_Pin,&initStruct);
    HW_GPIO_Init(BT_nRESET_GPIO_Port,BT_nRESET_Pin,&initStruct);
    HW_GPIO_Init(BT_OTA_APPDL_GPIO_Port,BT_OTA_APPDL_Pin,&initStruct);
    HW_GPIO_Init(BT_SIO_03_GPIO_Port,BT_SIO_03_Pin,&initStruct);	
    // MCU unused pins
    HW_GPIO_Init(Boot1_TP403_GPIO_Port,Boot1_TP403_Pin,&initStruct);
#ifndef DEF_TIMINGTEST
    HW_GPIO_Init(TP404_GPIO_Port,TP404_Pin,&initStruct);
    HW_GPIO_Init(TP405_GPIO_Port,TP405_Pin,&initStruct);
#endif    
    HW_GPIO_Init(TP408_GPIO_Port,TP408_Pin,&initStruct);
    HW_GPIO_Init(TP410_GPIO_Port,TP410_Pin,&initStruct);
    HW_GPIO_Init(TP411_GPIO_Port,TP411_Pin,&initStruct);
    HW_GPIO_Init(TP412_GPIO_Port,TP412_Pin,&initStruct);

    bInitDone = true; 		
  } 
}

/**
  * @brief This function deinitializes sirop I/Os
  * @param none
  * @retval none
  */
void HW_IOs_DeInit( void )
{  
 
}


/**
  * @brief This function initilizes Radio IOs as Analog
  * @param none
  * @retval none
  */
void HW_RadioIoInit_Analog(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG; 
  GPIO_InitStruct.Pull = GPIO_NOPULL; 
	
	/* RADIO SPI Pins */
  HW_GPIO_Init ( RADIO_MOSI_PORT, RADIO_MOSI_PIN, &GPIO_InitStruct );		
  HW_GPIO_Init ( RADIO_MISO_PORT, RADIO_MISO_PIN, &GPIO_InitStruct ); 
  HW_GPIO_Init ( RADIO_SCLK_PORT, RADIO_SCLK_PIN, &GPIO_InitStruct );
  HW_GPIO_Init ( RADIO_NSS_PORT, RADIO_NSS_PIN , &GPIO_InitStruct );        
   
  /* RADIO DIO Pins (uncomment according to used pins in the project)*/
	HW_GPIO_Init( RADIO_DIO_0_PORT, RADIO_DIO_0_PIN, &GPIO_InitStruct );	
	HW_GPIO_Init( RADIO_DIO_1_PORT, RADIO_DIO_1_PIN, &GPIO_InitStruct );	
	HW_GPIO_Init( RADIO_DIO_2_PORT, RADIO_DIO_2_PIN, &GPIO_InitStruct );	
//	HW_GPIO_Init( RADIO_DIO_3_PORT, RADIO_DIO_3_PIN, &GPIO_InitStruct ); 	
//	HW_GPIO_Init( RADIO_DIO_4_PORT, RADIO_DIO_4_PIN, &GPIO_InitStruct );	
//	HW_GPIO_Init( RADIO_DIO_5_PORT, RADIO_DIO_5_PIN, &GPIO_InitStruct );	
}


/*****END OF FILE****/
