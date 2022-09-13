/**============================================================================
* @file      RS485.c
* @date      2018-06-14
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     * Driver for Si705x Sensor from silicon silabs
*            
* VERSION:   
* 
* V0.01      2018-06-14-Kd      Create File			
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "bsp.h"
#include "RS485.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Exported functions ---------------------------------------------------------*/

bool RS485_TxDataPolled(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  bool bOk = false;
  bool bSysPowerOff = !BSP_IsSystemVccON();
  
  if (bSysPowerOff)
    BSP_SystemVcc(true);  // VCC System on
  
  HAL_GPIO_WritePin( RS485_USART1_DE_GPIO_Port,RS485_USART1_DE_Pin, GPIO_PIN_SET);   
  if (HAL_UART_Transmit(&huart1,pData, Size, Timeout) == HAL_OK)
    bOk = true;
    
  HAL_GPIO_WritePin( RS485_USART1_DE_GPIO_Port,RS485_USART1_DE_Pin, GPIO_PIN_RESET); 
  if (bSysPowerOff)
    BSP_SystemVcc(false);  // VCC System off

  return bOk;
}

/*****END OF FILE****/
