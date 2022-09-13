/**============================================================================
* @file      hw_dac.c
* @date      2018-06-21
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     initialization and definition of dac instance for this project
*            
* VERSION:   
* 
* V0.01      2018-06-21-Kd      Create File 			
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "hw.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
TIM_HandleTypeDef htim4; // Capture the Impedance U + I Phase
TIM_HandleTypeDef htim6; // DAC - DMA Feeding

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

/** 
* @brief  Init the TIM4.
* @param  None
* @return None
*/
void HW_TIM4_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_IC_InitTypeDef sConfigIC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 0;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // 32MHz
  if (HAL_TIM_IC_Init(&htim4) != HAL_OK)
  {
    Error_Handler(); // _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler(); // _Error_Handler(__FILE__, __LINE__);
  }

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_3) != HAL_OK) // IMP_U_COMP
  {
    Error_Handler(); // _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_4) != HAL_OK) // IMP_I_COMP
  {
    Error_Handler(); // _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
* @brief  Init the TIM6.
* @param  None
* @return None
*/
void HW_TIM6_Init(uint32_t u32Freq_Hz)
{
  TIM_MasterConfigTypeDef sMasterConfig;

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0; // 32MHz
  htim6.Init.Period = SystemCoreClock / u32Freq_Hz - 1;  // 32MHz / (319+1) = 100kHz 
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP; 
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler(); // _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler(); // _Error_Handler(__FILE__, __LINE__);
  }
}

void HW_TIM6_DeInit(void)
{  
  HAL_TIM_Base_DeInit(&htim6);
}


void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* tim_icHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(tim_icHandle->Instance==TIM4)
  {
    /* TIM4 clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();
  
    /**TIM4 GPIO Configuration    
    PB8     ------> TIM4_CH3
    PB9     ------> TIM4_CH4 
    */
    GPIO_InitStruct.Pin = IMP_U_COMP_Pin|IMP_I_COMP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
}

void HAL_TIM_IC_MspDeInit(TIM_HandleTypeDef* tim_icHandle)
{

  if(tim_icHandle->Instance==TIM4)
  {
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();
  
    /**TIM4 GPIO Configuration    
    PB8     ------> TIM4_CH3
    PB9     ------> TIM4_CH4 
    */
    HAL_GPIO_DeInit(GPIOB, IMP_U_COMP_Pin|IMP_I_COMP_Pin);
  }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{
  if(tim_baseHandle->Instance==TIM6)
  {
    /* TIM6 clock enable */
    __HAL_RCC_TIM6_CLK_ENABLE();
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM6)
  {
    /* Peripheral clock disable */
    __HAL_RCC_TIM6_CLK_DISABLE();
  }
} 

/*****END OF FILE****/
