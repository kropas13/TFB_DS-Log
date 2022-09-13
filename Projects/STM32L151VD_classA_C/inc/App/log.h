/**============================================================================
* @file      app.h
* @date      2017-08-30
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header file of app.c
*            
* VERSION:   
* 
* V0.10      2017-08-30-Ra      Create File 			
*
*============================================================================*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOG_H
#define __LOG_H

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/	
#include <stdint.h>
#include <stdbool.h>   
	 
/* Exported types ------------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */ 	

/**
  * @brief  Initializes application 
  */
bool Log_Init(bool bBackupPowerOn);

void Log_WriteDataLine(bool b_WriteUARTs);
/**
  * @brief  Log datas to SD Card
  */
bool Log_Data(void);	 
	 
bool Log_FileOpen(bool* pbSysPowerOff, uint32_t u32Pos);
uint16_t Log_FileRead(char* pTxBuf, uint16_t ui16TxBuffLen);
void Log_FileClose(bool bSysPowerOff);

bool Log_FileDelete(void);

uint32_t Log_FileGetSize(void);
   
#ifdef __cplusplus
}
#endif

#endif /*__LOG_H */

/*****END OF FILE****/
