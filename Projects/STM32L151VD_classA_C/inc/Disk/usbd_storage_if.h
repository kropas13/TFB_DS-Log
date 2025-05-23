/**
  ******************************************************************************
  * @file           : usbd_storage_if.h
  * @brief          : header file for the usbd_storage_if.c file
  ******************************************************************************
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  * 1. Redistributions of source code must retain the above copyright notice,
  * this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  * this list of conditions and the following disclaimer in the documentation
  * and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of its contributors
  * may be used to endorse or promote products derived from this software
  * without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __USBD_STORAGE_IF_H_
#define __USBD_STORAGE_IF_H_
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_msc.h"
/* USER CODE BEGIN INCLUDE */
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_STORAGE
  * @brief header file for the USBD_STORAGE.c file
  * @{
  */ 

/** @defgroup USBD_STORAGE_Exported_Defines
  * @{
  */ 
/* USER CODE BEGIN EXPORTED_DEFINES  */ 
   // MSC_Memory is saved in the EEPROM (max. 8192 Bytes)
   
   #define MSC_MemorySize  (8 * 1024) // for Win7 min. 8192 is needed for others 4096 is ok (for the Bootsector + FAT minimum. + Rootentry 2560 Bytes will be used)

   #define MSC_BlockSize   512
   #define MSC_BlockCount  (MSC_MemorySize / MSC_BlockSize)

/* USER CODE END  EXPORTED_DEFINES */

/**
  * @}
  */ 

/** @defgroup USBD_STORAGE_Exported_Types
  * @{
  */  
/* USER CODE BEGIN EXPORTED_TYPES  */
/* USER CODE END  EXPORTED_TYPES */

/**
  * @}
  */ 

/** @defgroup USBD_STORAGE_Exported_Macros
  * @{
  */ 
/* USER CODE BEGIN EXPORTED_MACRO  */
/* USER CODE END  EXPORTED_MACRO */

/**
  * @}
  */ 

/** @defgroup USBD_STORAGE_Exported_Variables
  * @{
  */ 
  extern USBD_StorageTypeDef  USBD_Storage_Interface_fops_FS;

/* USER CODE BEGIN EXPORTED_VARIABLES  */

/* USER CODE END  EXPORTED_VARIABLES */

/**
  * @}
  */ 

/** @defgroup USBD_STORAGE_Exported_FunctionsPrototype
  * @{
  */ 

/* USER CODE BEGIN EXPORTED_FUNCTIONS  */
/* USER CODE END  EXPORTED_FUNCTIONS */
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 
  
#ifdef __cplusplus
}
#endif

#endif /* __USBD_STORAGE_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
