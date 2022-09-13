/**============================================================================
* @file      hw_sdio.c
* @date      2018-06-08
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     * hw_sdio defines and initializes the SDIO instance to be used for this project
*            
* VERSION:   
* 
* V0.01      2018-06-08-Kd      Create File			
*
*============================================================================*/

/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "bsp.h"
#include "hw_sdio.h"


SD_HandleTypeDef hsd;
HAL_SD_CardInfoTypedef SDCardInfo;

/* SDIO init function */

void HW_SDIO_SD_Init(void)
{

  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  // MUST: Frequency PCLK2() >= 3/8 × Frequency SDIO_CK() -> 32MHz / 8 * 3 >= 9MHz for PCLK2() -> PCLK2 is 32MHz ok
  // And from errata 2.7.3 No underrun detected and wrong data transmission
  // [3 x period(APBClock) + 3 x period(SDIOCLK)] < (32 / (BusWidth)) x period(SDIO_CK)
  // (3/32MHz + 3/48MHz) < ((32/BusWidth) * 1/24MHz)
  // (0.09375 + 0.0625) < (32 * 0.041667)
  // 0.15625 < 1.333 is ok
  hsd.Init.ClockDiv = 0; // 0:48MHz/(2+0)=24MHz 2:48MHz/(2+2)=12MHz  4:48MHz/(2+4)=8MHz   
}

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(sdHandle->Instance==SDIO)
  {
    /* SDIO clock enable */
    __HAL_RCC_SDIO_CLK_ENABLE();
  
    /**SDIO GPIO Configuration    
    PC8     ------> SDIO_D0
    PC12     ------> SDIO_CK
    PD2     ------> SDIO_CMD 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* sdHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  if(sdHandle->Instance==SDIO)
  {
    /* Peripheral clock disable */
    __HAL_RCC_SDIO_CLK_DISABLE();
  
    /**SDIO GPIO Configuration    
    PC8     ------> SDIO_D0
    PC12     ------> SDIO_CK
    PD2     ------> SDIO_CMD 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);    
  }
} 


/*****END OF FILE****/
