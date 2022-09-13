/**============================================================================
* @file      BT.h
* @date      2018-06-11
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header of bt.c file (Bluetooth impl.)

*            
* VERSION:   
* 
* V0.01      2016-11-10-Ra      Create File
*============================================================================*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BT_H__
#define __BT_H__

#ifdef __cplusplus
 extern "C" {
#endif
      
/* Includes ------------------------------------------------------------------*/   
#include <stdint.h>
#include <stdbool.h>     
   
/* Exported types ------------------------------------------------------------*/
typedef enum
{
  e_BT_OpMode_AutoRunMode = 0,  // runs $autorun$ if it exists
  e_BT_OpMode_InteractiveMode,  // wait for interaction over internal UART
  e_BT_OpMode_VSP_BridgeToUart, // BT emulates a BT-UART (VSP VirtualSerialPort)
  e_BT_OpMode_VSP_CmdMode       // BT be configured over the emulated BT-UART (VSP VirtualSerialPort)
} e_BT_OpMode_t;
/* Exported constants --------------------------------------------------------*/   
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

/**
  * @brief  Rx Transfer completed callbacks.
  */
void BT_UART_RxCpltCallback( void );

/**
  * @brief  Tx Transfer completed callbacks.
  */
void BT_UART_TxCpltCallback( void );
   
/**
  * @brief  Rx Error callbacks.
  */   
void BT_UART_ErrorCallback( void );	

bool BT_GetUART_Cts( void );
/*!
 * \brief Init BT device
 */
void BT_Init( void );

// bool BT_IsConnected(void);

bool BT_HasWork( void );

bool BT_SendData( uint8_t *pu8TxBuf, uint16_t ui16Len);

void BT_Do( void );

#ifdef __cplusplus
}
#endif

#endif /* __BT_H__ */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
