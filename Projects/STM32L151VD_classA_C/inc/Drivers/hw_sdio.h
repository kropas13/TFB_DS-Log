/**============================================================================
* @file      hw_sdio.h
* @date      2018-06-08
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of hw_sdio.c

*            
* VERSION:   
* 
* V0.01      2018-06-08-Kd     Create File 
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_sdio_H
#define __HW_sdio_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
//#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SD_HandleTypeDef hsd;
extern HAL_SD_CardInfoTypedef SDCardInfo;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void HW_SDIO_SD_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__HW_sdio_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
