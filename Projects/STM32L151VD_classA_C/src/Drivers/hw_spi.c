/**============================================================================
* @file      hw_spi.c
* @date      2016-12-05
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     * hw_spi defines and initializes the SPI instance to be used for this project
*            * defines SPI_InOut function used in SX1272.c
*            
* VERSION:   
* 
* V0.01      2016-12-05-Ra      Create File
* V0.02      2016-12-09-Ra      hspi1 defined for external use 			
*
*============================================================================*/

/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "bsp.h"
#include "utilities.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;
/* Private function prototypes -----------------------------------------------*/

/*!
 * @brief Calculates Spi Divisor based on Spi Frequency and Mcu Frequency
 *
 * @param [IN] Spi Frequency
 * @retval Spi divisor
 */
static uint32_t SpiFrequency( uint32_t hz );

/* Exported functions ---------------------------------------------------------*/

/*!
 * @brief Initializes the SPI object and MCU peripheral
 *
 * @param [IN] none
 */
HAL_StatusTypeDef HW_SPI2_Init( void )
{
  /*##-1- Configure the SPI peripheral */
  /* Set the SPI parameters */
  hspi2.Instance = SPI2;
  hspi2.Init.BaudRatePrescaler = SpiFrequency( 10000000 );
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.Mode = SPI_MODE_MASTER;	
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;	
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;	
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial     = 10;	//cubeMX random value (it doesnt matter anyway, CRC is disabled)	
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
	
  return HAL_SPI_Init( &hspi2);
}

/*!
 * @brief De-initializes the SPI object and MCU peripheral
 *
 * @param [IN] none
 */
HAL_StatusTypeDef HW_SPI2_DeInit( void )
{
  HAL_StatusTypeDef ret = HAL_SPI_DeInit( &hspi2);
	//maybe uneeded
    /*##-1- Reset peripherals ####*/
  __HAL_RCC_SPI2_FORCE_RESET();
  __HAL_RCC_SPI2_RELEASE_RESET();
	return ret;
}

/*!
 * @brief Sends outData and receives inData
 *
 * @param [IN] outData Byte to be sent
 * @retval inData      Received byte.
 */
uint8_t HW_SPI2_InOut( uint8_t txData )
{
  uint8_t rxData ;

  HAL_SPI_TransmitReceive( &hspi2, &txData, &rxData, 1, HAL_MAX_DELAY);	

  return rxData;
}

/*!
 * @brief Initializes the SPI object and MCU peripheral
 *
 * @param [IN] none
 */
HAL_StatusTypeDef HW_SPI3_Init( void )
{
  /*##-1- Configure the SPI peripheral */
  /* Set the SPI parameters */
  hspi3.Instance = SPI3;
  hspi3.Init.BaudRatePrescaler = SpiFrequency( 2000000 );
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.Mode = SPI_MODE_MASTER;	
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;	
  hspi3.Init.CLKPhase = SPI_PHASE_2EDGE;	
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
	
  return HAL_SPI_Init( &hspi3);
}

/*!
 * @brief De-initializes the SPI object and MCU peripheral
 *
 * @param [IN] none
 */
HAL_StatusTypeDef HW_SPI3_DeInit( void )
{
  HAL_StatusTypeDef ret = HAL_SPI_DeInit( &hspi3);
	//maybe uneeded
    /*##-1- Reset peripherals ####*/
  __HAL_RCC_SPI3_FORCE_RESET();
  __HAL_RCC_SPI3_RELEASE_RESET();
	return ret;
}

/*!
 * @brief Sends outData and receives inData
 *
 * @param [IN] outData Byte to be sent
 * @retval inData      Received byte.
 */
uint8_t HW_SPI3_InOut( uint8_t txData )
{
  uint8_t rxData ;

  HAL_SPI_TransmitReceive( &hspi3, &txData, &rxData, 1, HAL_MAX_DELAY);	

  return rxData;
}

/* Private functions ---------------------------------------------------------*/
static uint32_t SpiFrequency( uint32_t hz )
{
  uint32_t divisor = 0;
  uint32_t SysClkTmp = SystemCoreClock / 2;
  uint32_t baudRate;
  
  while( SysClkTmp > hz)
  {
    divisor++;
    SysClkTmp= ( SysClkTmp >> 1);
    
    if (divisor >= 7)
      break;
  }
  
  baudRate =((( divisor & 0x4 ) == 0 )? 0x0 : SPI_CR1_BR_2  )| 
            ((( divisor & 0x2 ) == 0 )? 0x0 : SPI_CR1_BR_1  )| 
            ((( divisor & 0x1 ) == 0 )? 0x0 : SPI_CR1_BR_0  );
  
  return baudRate;
}


/* MspInit and DeInit function definitions -----------------------------------*/
/**
  * @brief SPI MSP Initialization
  *        This function configures the hardware resources used for the SPI
  * @param hspi: SPI handle pointer
  * @note  
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hspi->Instance==SPI2)
  {
    /* Peripheral clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();
    __NOP( );
    __NOP( );
    /**SPI2 GPIO Configuration    
    PD0     ------> SPI2_NSS
    PD1     ------> SPI2_SCK
    PD3     ------> SPI2_MISO
    PD4     ------> SPI2_MOSI 
    */
    GPIO_InitStruct.Mode =GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate= GPIO_AF5_SPI2;		
    
    HW_GPIO_Init( RADIO_SCLK_PORT, RADIO_SCLK_PIN, &GPIO_InitStruct); 
    HW_GPIO_Init( RADIO_MISO_PORT, RADIO_MISO_PIN, &GPIO_InitStruct); 
    HW_GPIO_Init( RADIO_MOSI_PORT, RADIO_MOSI_PIN, &GPIO_InitStruct);     

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
		
    HW_GPIO_Init(RADIO_NSS_PORT, RADIO_NSS_PIN, &GPIO_InitStruct);
	  HAL_GPIO_WritePin(RADIO_NSS_PORT, RADIO_NSS_PIN, GPIO_PIN_SET);		
  /* USER CODE END SPI2_MspInit 1 */
  } 
  else if(hspi->Instance==SPI3) {
  /* USER CODE BEGIN SPI3_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_SPI3_CLK_ENABLE();
    __NOP( );
    __NOP( );  
    /**SPI3 GPIO Configuration    
    PA4     ------> SPI3_NSS
    PB3     ------> SPI3_SCK
    PB4     ------> SPI3_MISO
    PB5     ------> SPI3_MOSI 
    */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;	
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;

    HW_GPIO_Init( ADC_SPI3_MISO_GPIO_Port, ADC_SPI3_MISO_Pin, &GPIO_InitStruct); 
    HW_GPIO_Init( ADC_SPI3_MOSI_GPIO_Port, ADC_SPI3_MOSI_Pin, &GPIO_InitStruct);      		
	  HW_GPIO_Init( ADC_SPI3_SCLK_GPIO_Port, ADC_SPI3_SCLK_Pin, &GPIO_InitStruct); 

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;		
	  
    HAL_GPIO_WritePin(ADC_SPI3_CS_GPIO_Port, ADC_SPI3_CS_Pin, GPIO_PIN_SET);
    HW_GPIO_Init(ADC_SPI3_CS_GPIO_Port, ADC_SPI3_CS_Pin, &GPIO_InitStruct);			
  }	
}

/**
  * @brief RTC MSP De-Initialization 
  *        This function freeze the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hspi->Instance==SPI2)
  {
		/* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();
		
    /**SPI2 GPIO Configuration    
    PD0     ------> SPI2_NSS
    PD1     ------> SPI2_SCK
    PD3     ------> SPI2_MISO
    PD4     ------> SPI2_MOSI 
    */
		/* if Module OFF, put SPI to analog */
    if (!GetLoraPower()){
      GPIO_InitStruct.Mode = GPIO_MODE_ANALOG; 
      GPIO_InitStruct.Pull = GPIO_NOPULL; 
		
      HW_GPIO_Init ( RADIO_MOSI_PORT, RADIO_MOSI_PIN, &GPIO_InitStruct );		
      HW_GPIO_Init ( RADIO_MISO_PORT, RADIO_MISO_PIN, &GPIO_InitStruct ); 
      HW_GPIO_Init ( RADIO_SCLK_PORT, RADIO_SCLK_PIN, &GPIO_InitStruct );
      HW_GPIO_Init ( RADIO_NSS_PORT, RADIO_NSS_PIN , &GPIO_InitStruct ); 		
    } else {
		  /* IF CS NOT PULLED DOWN EXTERNALLY (9) */
		  // MISO 0 w/ no pull
      GPIO_InitStruct.Mode =GPIO_MODE_ANALOG;
      GPIO_InitStruct.Pull =GPIO_NOPULL ; 			
      HW_GPIO_Init ( RADIO_MISO_PORT, RADIO_MISO_PIN, &GPIO_InitStruct ); 			
		
      // MOSI and SCLK set to 0 w/ no pull
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;	
      GPIO_InitStruct.Pull = GPIO_NOPULL; 
      HW_GPIO_Init ( RADIO_MOSI_PORT, RADIO_MOSI_PIN, &GPIO_InitStruct );	
      HAL_GPIO_WritePin(RADIO_MOSI_PORT, RADIO_MOSI_PIN, GPIO_PIN_RESET);			
      HW_GPIO_Init ( RADIO_SCLK_PORT, RADIO_SCLK_PIN, &GPIO_InitStruct );
      HAL_GPIO_WritePin(RADIO_SCLK_PORT, RADIO_SCLK_PIN, GPIO_PIN_RESET);			
		
      // CS set to 1 w/ no pull
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;	
      GPIO_InitStruct.Pull = GPIO_NOPULL; 
      HW_GPIO_Init ( RADIO_NSS_PORT, RADIO_NSS_PIN , &GPIO_InitStruct ); 
      HAL_GPIO_WritePin(RADIO_NSS_PORT, RADIO_NSS_PIN, GPIO_PIN_SET);			
    }		
  }
  else if(hspi->Instance==SPI3)
  {
    /* Peripheral clock disable */
    __HAL_RCC_SPI3_CLK_DISABLE();
  
    /**SPI3 GPIO Configuration    
    PA4     ------> SPI3_NSS
    PB3     ------> SPI3_SCK
    PB4     ------> SPI3_MISO
    PB5     ------> SPI3_MOSI 
    */
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG; 
    GPIO_InitStruct.Pull = GPIO_NOPULL;  

    HW_GPIO_Init( ADC_SPI3_MISO_GPIO_Port, ADC_SPI3_MISO_Pin, &GPIO_InitStruct); 
    HW_GPIO_Init( ADC_SPI3_MOSI_GPIO_Port, ADC_SPI3_MOSI_Pin, &GPIO_InitStruct);      		
	  HW_GPIO_Init( ADC_SPI3_SCLK_GPIO_Port, ADC_SPI3_SCLK_Pin, &GPIO_InitStruct); 		
    HW_GPIO_Init(ADC_SPI3_CS_GPIO_Port, ADC_SPI3_CS_Pin, &GPIO_InitStruct);			
  }	
}

/*****END OF FILE****/

