/**============================================================================
* @file      hw_i2c.c
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     initialization and definition of i2c instance for this project
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 			
*
*============================================================================*/

/* Includes ------------------------------------------------------------------*/
#include "hw.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*!
 * i2c handle variable
 */
I2C_HandleTypeDef hi2c1;

/* Private function prototypes -----------------------------------------------*/
/* Exported functions ---------------------------------------------------------*/
/*!
 * @brief Initializes the I2C object and MCU peripheral
 *
 * @param [IN] none
 */
HAL_StatusTypeDef HW_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  return HAL_I2C_Init(&hi2c1);
}

/*!
 * @brief De-initializes the I2C object and MCU peripheral
 *
 * @param [IN] none
 */
HAL_StatusTypeDef HW_I2C1_DeInit(void)
{
  HAL_StatusTypeDef ret = HAL_I2C_DeInit(&hi2c1);
	
  __HAL_RCC_I2C1_FORCE_RESET();
  __HAL_RCC_I2C1_RELEASE_RESET();
	
  return ret;
}

/* MspInit and DeInit function definitions -----------------------------------*/
/**
  * @brief I2C MSP Initialization 
  *        This function configures the hardware resources of I2C 
  * @param huart: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hi2c->Instance==I2C1)
  {
    /* Peripheral clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
    __NOP( );
    __NOP( );		
  
	  /* I2C TX/RX GPIO pin configuration  */  
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;

    HW_GPIO_Init(I2C1_TEMP_SCL_GPIO_Port,I2C1_TEMP_SCL_Pin,&GPIO_InitStruct);
	  HW_GPIO_Init(I2C1_TEMP_SDA_GPIO_Port,I2C1_TEMP_SDA_Pin,&GPIO_InitStruct);						
  }
}

/**
  * @brief UART MSP DeInit
  * @param huart: uart handle
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hi2c->Instance==I2C1)
  {

    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();
  
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  
    
    HW_GPIO_Init(I2C1_TEMP_SCL_GPIO_Port,I2C1_TEMP_SCL_Pin,&GPIO_InitStruct);
	  HW_GPIO_Init(I2C1_TEMP_SDA_GPIO_Port,I2C1_TEMP_SDA_Pin,&GPIO_InitStruct);	
			
  }
} 

/*****END OF FILE****/
