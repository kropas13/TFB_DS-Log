/**============================================================================
* @file      com.c
* @date      2018-06-11
*
* @author    D.Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     main file of communication
*            
* VERSION:   
* 
* V0.01            2018-06-11-Kd     Create File 		
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "bsp.h"
#include "loraModem.h"
#include "usbd_cdc_if.h"
#include "config.h"
#include "app.h"
#include "rtcApp.h"
#include "log.h"
#include "fatfs.h"
#include "com.h"

/* Private define ------------------------------------------------------------*/
#define COM_TX_DATA_SIZE                128

/* Private macro -------------------------------------------------------------*/
#define __COM_IS_CMD(__CMD__, __RX_PTR__)                        \
  (!strncmp(__CMD__, (char*)__RX_PTR__, sizeof(__CMD__)-1))

/* Private function prototypes -----------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/
typedef enum {
  eComTxProlongType_LogFileRead = 0,
  eComTxProlongType_CfgFileRead  
} teComTxProlongType;

/* Private variables ---------------------------------------------------------*/
const char strCom_Restart[]             = "Restart_";     // restart
const char strCom_GetInfo[]             = "GetInfo_";
const char strCom_RtcGet[]              = "RtcGet_";
const char strCom_RtcSet[]              = "RtcSet_";
const char strCom_ShowMeasData[]        = "ShowMeasData_";
const char strCom_GetMeasData[]         = "GetMeasData_";
const char strCom_PressButton[]         = "PressButton_";
const char strCom_GetLoRaInfo[]         = "GetLoRaInfo_";
const char strCom_LogFileGetSize[]      = "LogFileGetSize_"; 
const char strCom_LogFileRead[]         = "LogFileRead_"; 
const char strCom_LogFileDelete[]       = "LogFileDelete_";
const char strCom_CfgFileRead[]         = "CfgFileRead_"; 
//const char strCom_CfgFileDelete[]       = "CfgFileDelete_";  
  
  
/* Global variables -------------------------------------------------------*/
bool    bCom_SendMeasData = false;
bool    bCom_GetMeasData = false;

/* Global functions ---------------------------------------------------------*/
uint16_t Com_HandleTelegramm( uint8_t* pui8RxBuffAddr, uint16_t ui16RxBuffLen, uint8_t* pui8TxBuffAddr, bool* pbRequestTxProlong)
{  
static bool bSysPowerOff = false;
static teComTxProlongType eComTxProlongType;  
  RTC_TimeTypeDef sTime = {0}; 
  RTC_DateTypeDef sDate = {0};     
  uint16_t ui16TxBuffLen = 0;
  uint16_t au16DateTime[6];
  uint32_t u32Pos = 0;
  bool     bTxAppendOk = false;
  bool     bTxAppendParamErr = false;
  bool     bAnswer = false;
  
  if (ui16RxBuffLen && !*pbRequestTxProlong)
  {
    bAnswer = true;
    if (__COM_IS_CMD(strCom_Restart, pui8RxBuffAddr))
    {
      pui8RxBuffAddr += sizeof(strCom_Restart)-1;    
      __disable_irq( );
      if (*pui8RxBuffAddr == '1')
        ui8ResetValue |= 0x01; // Start in USB-CDC Mode again
      NVIC_SystemReset();       
    }
    else if (__COM_IS_CMD(strCom_GetInfo, pui8RxBuffAddr))
    {
      uint8_t chipId[12];    
      uint8_t u8HWFlags = (bColdJunctionTempErr ? 0x80 : 0x00) | (bExtAdcErr ? 0x40 : 0x00) | (bUSBinitialized ? 0x20 : 0x00);
      uint16_t u16BatteryVDDmV  = MEASGetUbat();
      
      ui16TxBuffLen = sprintf((char*)pui8TxBuffAddr, "%u;%u;%u;%02u.%02u;", DEVICE_ID, GetHWRev(), APP_ID, APP_MAJOR_VERSION, APP_MINOR_VERSION);    
      
      HW_GetChipId(chipId);    
      CfgMakeHexStr((char*)pui8TxBuffAddr+ui16TxBuffLen, chipId, 12, 0);
      ui16TxBuffLen += (2*12);
      
      ui16TxBuffLen += sprintf((char*)(char*)pui8TxBuffAddr+ui16TxBuffLen, ";%u;%u", u8HWFlags, u16BatteryVDDmV);  
        
      // bTxAppendOk = true;   // 2018-10-09-Kd ab V1.1
    } 
    else if (__COM_IS_CMD(strCom_ShowMeasData, pui8RxBuffAddr))
    {  
      pui8RxBuffAddr += sizeof(strCom_ShowMeasData)-1;    
      if (*pui8RxBuffAddr == '1')    
        bCom_SendMeasData = true;
      else if (*pui8RxBuffAddr == '0') 
        bCom_SendMeasData = false;
      else
        bCom_SendMeasData = !bCom_SendMeasData;
      bTxAppendOk = true;  
    }
    else if (__COM_IS_CMD(strCom_GetMeasData, pui8RxBuffAddr))
    {  
      bCom_GetMeasData = true;
    }        
    else if (__COM_IS_CMD(strCom_PressButton, pui8RxBuffAddr))
    {
      irqHandlerButton();
      bTxAppendOk = true; 
    }
    else if (__COM_IS_CMD(strCom_GetLoRaInfo, pui8RxBuffAddr))
    {    
      uint8_t ui8LastRssi;
      int8_t  i8LastSnr;
      
      LoRaGetAndCalcRssiSnr( &ui8LastRssi, &i8LastSnr);
      ui16TxBuffLen += sprintf((char*)pui8TxBuffAddr, "%u;%u;%u", lora_getMacStatus(), ui8LastRssi, i8LastSnr);
      // bTxAppendOk = true;   // 2018-10-09-Kd ab V1.1
    }  
    else if (__COM_IS_CMD(strCom_RtcGet, pui8RxBuffAddr))
    {   
      App_RTC_GetDateTime( &sDate, &sTime);    
      ui16TxBuffLen += sprintf((char*)pui8TxBuffAddr, "%04u-%02u-%02u %02u:%02u:%02u", 2000+(uint16_t)sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes, sTime.Seconds);  
      // bTxAppendOk = true;   // 2018-10-09-Kd ab V1.1   
    }
    else if (__COM_IS_CMD(strCom_RtcSet, pui8RxBuffAddr))
    {  
      pui8RxBuffAddr += sizeof(strCom_RtcSet)-1;
      if (sscanf( (char*)pui8RxBuffAddr, "%04hu-%02hu-%02hu %02hu:%02hu:%02hu", &au16DateTime[0], &au16DateTime[1], &au16DateTime[2], &au16DateTime[3], &au16DateTime[4], &au16DateTime[5]) == 6)        
      {        
        sDate.Year =  au16DateTime[0] % 100;     
        sDate.Month =  au16DateTime[1];  
        sDate.Date = au16DateTime[2];  
        sTime.Hours = au16DateTime[3];  
        sTime.Minutes = au16DateTime[4];  
        sTime.Seconds = au16DateTime[5];                       
        if (App_RTC_SetDateTime(&sDate, &sTime))                           
          bTxAppendOk = true;
        else
          bTxAppendParamErr = true;
      }
      else
        bTxAppendParamErr = true;
    }      
    else if (__COM_IS_CMD(strCom_LogFileGetSize, pui8RxBuffAddr))
    {      
       u32Pos = Log_FileGetSize();
       ui16TxBuffLen += sprintf((char*)pui8TxBuffAddr, "%u", u32Pos);
       // bTxAppendOk = true;   // 2018-10-09-Kd ab V1.1
    }     
    else if (__COM_IS_CMD(strCom_LogFileRead, pui8RxBuffAddr))
    {
      pui8RxBuffAddr += sizeof(strCom_LogFileRead)-1;    
      if (!sscanf( (char*)pui8RxBuffAddr, "%u", &u32Pos))
        u32Pos = 0;      
      if (Log_FileOpen(&bSysPowerOff, u32Pos))
      {
        eComTxProlongType = eComTxProlongType_LogFileRead;
        ui16RxBuffLen = 0;
        *pbRequestTxProlong = true;
      }
    }
    else if (__COM_IS_CMD(strCom_LogFileDelete, pui8RxBuffAddr))
    {
      bTxAppendOk = Log_FileDelete();
    }  
    else if (__COM_IS_CMD(strCom_CfgFileRead, pui8RxBuffAddr))
    {        
      if (Cfg_FileOpen())
      {
        eComTxProlongType = eComTxProlongType_CfgFileRead;
        ui16RxBuffLen = 0;
        *pbRequestTxProlong = true;
      }
    }
    else
    {
      bAnswer = false;
      ui16TxBuffLen = CfgHandleCommTelegramm( pui8RxBuffAddr, ui16RxBuffLen, pui8TxBuffAddr, &bTxAppendOk, &bTxAppendParamErr, &bAnswer);
    }
  }
  
  if (*pbRequestTxProlong)
  {    
    appHandleIWDG_Refresh(); // The App. should not work longer than 9s without calling this function
    if (eComTxProlongType == eComTxProlongType_LogFileRead)
    {
      ui16TxBuffLen = Log_FileRead((char*)pui8TxBuffAddr, COM_TX_DATA_SIZE);
      if (!ui16TxBuffLen || ui16RxBuffLen)
      {
        Log_FileClose(bSysPowerOff);    
        *pbRequestTxProlong = false;
      }          
    }
    else if (eComTxProlongType == eComTxProlongType_CfgFileRead)
    {
      ui16TxBuffLen = Cfg_FileRead((char*)pui8TxBuffAddr, COM_TX_DATA_SIZE);
      if (!ui16TxBuffLen || ui16RxBuffLen)
      {
        Cfg_FileClose();    
        *pbRequestTxProlong = false;
      }        
    }
  }
  else
  {
    if (bTxAppendOk)
      ui16TxBuffLen += sprintf((char*)pui8TxBuffAddr + ui16TxBuffLen, "Ok");    
    else if (bTxAppendParamErr)
      ui16TxBuffLen += sprintf((char*)pui8TxBuffAddr + ui16TxBuffLen, "ParamErr");    
    else if (!bAnswer)
      ui16TxBuffLen += sprintf((char*)pui8TxBuffAddr + ui16TxBuffLen, "CmdErr");
    
    if (ui16TxBuffLen)
      ui16TxBuffLen += sprintf((char*)pui8TxBuffAddr + ui16TxBuffLen, "\r\n");  
  }
  
  return ui16TxBuffLen;  
}


/*****END OF FILE****/
