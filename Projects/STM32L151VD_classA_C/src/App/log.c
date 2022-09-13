/**============================================================================
* @file      Log.c
* @date      2018-06-08
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     * logs data to the SD Card
*            
* VERSION:   
* 
* V0.01      2018-06-08-Kd      Create File			
*
*============================================================================*/

/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "log.h"
#include "fatfs.h"
#include "bsp.h"
#include "rtcApp.h"
#include "app.h"
#include "meas.h"

#include <stdbool.h>

/* Private Defines ---------------------------------------------------------*/

#define DEF_LOG_ONE_FILE  // When defined -> only one file for all datas     When not defined -> each day a new file

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
  
#define  C_CSV_SEPARATOR_CHAR ';'

/* Private variables ---------------------------------------------------------*/
extern Disk_drvTypeDef disk;
extern SD_HandleTypeDef hsd;

uint32_t gu32LogNr;

typedef struct {
  char*     strHeader;
  int8_t    i8ExtSensMesDataIdx;
  uint8_t   ui8DecLen;
  } tsLogData;

#define  STR_HEADER_SamplingTime    "SamplingTime [YYYY-MM-DD hh:mm:ss]" 
#define  STR_HEADER_LogNr           "LogNr"
  
//#define  STR_HEADER_DATE            "Date [YYYY-MM-DD]" 
//#define  STR_HEADER_TIME            "Time [hh:mm:ss]"

   
const tsLogData asLogData[] = { 
  {"Battery [V]", -1, 3},
  {"Board Temperature [degC]", -1, 2},  
  {"Temperature %d [degC]", 0, 2},  
  {"Voltage %d [V]", 8, 4},         // 2019-01-07-Kd ab V1.03 4 Stellen
  {"Current %d [mA]", 16, 2}, 
  {"Corr. Potential %d [mV]", 24, 1},
  {"Corr. Current %d [uA]", 32, 1},
  {"Impedance %d [Ohm]", 40, 1},    
  {"Imp. Active %d [Ohm]", 48, 1},   
};

#define LOG_DATA_ARY_CNT (sizeof(asLogData) / sizeof(tsLogData))
   
FRESULT resLogFile;

/* External function prototypes -----------------------------------------------*/
extern void App_TerminalSendMeasData(uint8_t *pu8TxBuf, uint16_t ui16Len, bool bCheckBT_CTS);

/* Private functions ---------------------------------------------------------*/	
char* Log_GetFullFilename(void)
{
  static char str_fullfilepath[10+8+1+3+1];  // x:/LgYYMMDD.csv
#ifdef  DEF_LOG_ONE_FILE
  sprintf((char*)str_fullfilepath, "%sLg.csv", SDPath);
#else  
  RTC_DateTypeDef sDate;
  RTC_TimeTypeDef sTime;
    
  App_RTC_GetDateTime(&sDate, &sTime);  
  sprintf((char*)str_fullfilepath, "%sLg%02u%02u%02u.csv", SDPath, sDate.Year%100, sDate.Month, sDate.Date);
#endif  
  
  return str_fullfilepath;
}

/* Private functions ---------------------------------------------------------*/	
static void Log_ShutDown(bool bUnmount, bool bUnlink, bool bSysPowerOff)
{
  if (bUnmount)
    f_mount(0, SDPath, 1); // Unmount  
  if (bUnmount || bUnlink)
    retSD = FATFS_UnLinkDriver(SDPath); // 2. Volume    
  HAL_SD_DeInit(&hsd);
  BSP_SleepDelayMs(100);
  if (bSysPowerOff)
    BSP_SystemVcc(false);  // VCC System off   
}

/* Exported functions --------------------------------------------------------------*/
bool Log_Init(bool bBackupPowerOn)
{
  bool bSuccess = false;     
  bool bUnmount = false;
  bool bUnlink = false;
  bool bSysPowerOff = !BSP_IsSystemVccON();
  
  if (bBackupPowerOn)
  {
    App_RTC_SaveBackupRegister( RtcBackupRamReg_LogNr, 0);
    gu32LogNr = 0;
  }
  else
    gu32LogNr = App_RTC_ReadBackupRegister( RtcBackupRamReg_LogNr);

  if (bSysPowerOff)
    BSP_SystemVcc(true);  // VCC System on 
  
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);
  if (!retSD) // 2. Volume  
  {
    bUnlink = true;
    //    disk.is_initialized[(SDPath[0]-0x30) % _VOLUMES] = 0; // bei f_open() wieder ein disk_initialize() durchführen
    if(f_mount(&SDFatFS, SDPath, 0) == FR_OK)
    {   
       bUnmount = true;
       resLogFile = f_open(&SDFile, Log_GetFullFilename(), FA_OPEN_ALWAYS | FA_WRITE);
       if (resLogFile == FR_OK)
       {
         bSuccess = true;
         f_close(&SDFile);
       }  
    } 
  }  
  
  Log_ShutDown( bUnmount, bUnlink, bSysPowerOff);
  
  return bSuccess;  
}

void Log_WriteDataLine(bool b_WriteUARTs)
{
  // bool            bOk = true;
  int             iLogIdx, iChIdx;  
  float           *pfVal;  
  uint32_t        bytesWritten;  
  char            strData[50];
  RTC_TimeTypeDef sTime; 
  RTC_DateTypeDef sDate;    
  char            strFormat[10];
       
  App_RTC_GetDateTime( &sDate, &sTime);
  sprintf(strData, "%04u-%02u-%02u %02u:%02u:%02u%c%09u", 
    2000+(uint16_t)sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes, sTime.Seconds, 
  C_CSV_SEPARATOR_CHAR, b_WriteUARTs ? 0 : gu32LogNr); 
  if (b_WriteUARTs)
    App_TerminalSendMeasData((uint8_t*)strData, strlen(strData), true);
  else
    resLogFile = f_write(&SDFile, strData, strlen(strData), (void *)&bytesWritten);         
        
  for (iLogIdx=0; (iLogIdx<LOG_DATA_ARY_CNT) && (b_WriteUARTs || (resLogFile == FR_OK)); iLogIdx++)  
  {      
    if (iLogIdx == 0)
    {
      sprintf(strFormat,"%c%%.%uf", C_CSV_SEPARATOR_CHAR, asLogData[iLogIdx].ui8DecLen);
      sprintf(strData, strFormat, (float)u16BatteryVDDmV/1000);   
      if (b_WriteUARTs)
        App_TerminalSendMeasData((uint8_t*)strData, strlen(strData), false);
      else
        resLogFile = f_write(&SDFile, strData, strlen(strData), (void *)&bytesWritten);
    }
    else if (iLogIdx == 1)
    {
      sprintf(strFormat,"%c%%.%uf", C_CSV_SEPARATOR_CHAR, asLogData[iLogIdx].ui8DecLen);
      sprintf(strData, strFormat, (float)i16Temperature_GC_100 / 100); 
      if (b_WriteUARTs)
        App_TerminalSendMeasData((uint8_t*)strData, strlen(strData), false);
      else      
        resLogFile = f_write(&SDFile, strData, strlen(strData), (void *)&bytesWritten);
    }
    else
    {
      sprintf(strFormat,"%c%%.%uf", C_CSV_SEPARATOR_CHAR, asLogData[iLogIdx].ui8DecLen);
      pfVal = (float*)&sMEASExtSenseData;
      pfVal += asLogData[iLogIdx].i8ExtSensMesDataIdx;
      for (iChIdx=0;iChIdx<EXT_SENSE_CHANNELS;iChIdx++,pfVal++)                            
      {
        sprintf(strData, strFormat, *pfVal);                                
        if (b_WriteUARTs)
          App_TerminalSendMeasData((uint8_t*)strData, strlen(strData), false);
        else
          resLogFile = f_write(&SDFile, strData, strlen(strData), (void *)&bytesWritten);
      }
    }                                       
  }  
  if (b_WriteUARTs)
    App_TerminalSendMeasData((uint8_t*)"\r\n", 2, false);
  else if (resLogFile == FR_OK)  
    f_putc('\n', &SDFile);   
  
}

// Benötigt ca. 20ms um die Daten zu speichern
bool Log_Data()
{
  bool bSuccess = false; 
  bool bUnmount = false;
  bool bUnlink  = false;
  int  iLogIdx, iChIdx;
  uint32_t bytesWritten;
//   float    *pfVal;
  char     strData[50];
//  char     strFormat[10];
//  RTC_TimeTypeDef sTime; 
//  RTC_DateTypeDef sDate;  
  bool bSysPowerOff = !BSP_IsSystemVccON();
  
  SetLEDRed(true);  
  
  App_RTC_SaveBackupRegister( RtcBackupRamReg_LogNr, ++gu32LogNr);
  
  if (bSysPowerOff)
    BSP_SystemVcc(true);  // VCC System on  
  
    /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);
  if (!retSD) // 2. Volume 
  {   
    bUnlink = true;
    if(f_mount(&SDFatFS, SDPath, 0) == FR_OK)
    {   
       bUnmount = true;
       resLogFile = f_open(&SDFile, Log_GetFullFilename(), FA_OPEN_ALWAYS | FA_WRITE);   
       if (resLogFile == FR_OK) 
       {
         // New -> Make Header
         if (f_size(&SDFile) < 30)
         {
           f_rewind(&SDFile);
           f_truncate(&SDFile);
           resLogFile = f_write(&SDFile, STR_HEADER_SamplingTime, strlen(STR_HEADER_SamplingTime), (void *)&bytesWritten); 
           f_putc ( C_CSV_SEPARATOR_CHAR, &SDFile);
           resLogFile = f_write(&SDFile, STR_HEADER_LogNr, strlen(STR_HEADER_LogNr), (void *)&bytesWritten);           
           for (iLogIdx=0; (iLogIdx<LOG_DATA_ARY_CNT) && (resLogFile == FR_OK); iLogIdx++)
           {              
              if (asLogData[iLogIdx].i8ExtSensMesDataIdx >= 0)
              {  
                for (iChIdx=0;iChIdx<EXT_SENSE_CHANNELS;iChIdx++)
                {
                  f_putc ( C_CSV_SEPARATOR_CHAR, &SDFile);	
                  sprintf(strData, asLogData[iLogIdx].strHeader, iChIdx+1);
                  resLogFile = f_write(&SDFile, strData, strlen(strData), (void *)&bytesWritten); 
                }
              }
              else
              {
                f_putc ( C_CSV_SEPARATOR_CHAR, &SDFile);	
                resLogFile = f_write(&SDFile, asLogData[iLogIdx].strHeader, strlen(asLogData[iLogIdx].strHeader), (void *)&bytesWritten); 
              }
           } 
           if (resLogFile == FR_OK)
             f_putc('\n', &SDFile);
         } 
         else                
           resLogFile = f_lseek(&SDFile, f_size(&SDFile)-1); // Go to the end
        
         if (resLogFile == FR_OK)  
           Log_WriteDataLine(false);
       }
         
       if (resLogFile == FR_OK)
         bSuccess = true;
       f_close(&SDFile);      
    }    
  }
  
  Log_ShutDown( bUnmount, bUnlink, bSysPowerOff);
  
  SetLEDRed(false);
  
  return bSuccess;  
}

bool Log_FileOpen(bool* pbSysPowerOff, uint32_t u32Pos)
{
  bool bUnmount = false;
  bool bUnlink = false;
  
  *pbSysPowerOff = !BSP_IsSystemVccON();
  
   if (*pbSysPowerOff)
    BSP_SystemVcc(true);  // VCC System on  
   
    /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);
  if (!retSD) // 2. Volume 
  {   
    bUnlink = true;
    if(f_mount(&SDFatFS, SDPath, 0) == FR_OK)
    {   
       bUnmount = true;
       resLogFile = f_open(&SDFile, Log_GetFullFilename(), FA_OPEN_EXISTING | FA_READ);   
       if (resLogFile == FR_OK) 
       {
          if (u32Pos)
            resLogFile = f_lseek(&SDFile, u32Pos);
          if (resLogFile == FR_OK) 
          {
            SetLEDRed(true); 
            return true;
          }
       }
    }
  } 
  
  Log_ShutDown( bUnmount, bUnlink, *pbSysPowerOff);   
  
  return false;
}

uint16_t Log_FileRead(char* pTxBuf, uint16_t ui16TxBuffLen)
{
  uint32_t u32Len = 0;
  
  f_read( &SDFile, pTxBuf, ui16TxBuffLen, &u32Len);
  
  return u32Len;
}

/*
uint16_t Log_FileReadLine(char* pTxBuf, uint16_t ui16TxBuffLen)
{
  uint16_t u16Len = 0;
  
  memset(pTxBuf, 0, ui16TxBuffLen);
  if (f_gets(pTxBuf, ui16TxBuffLen-2, &SDFile))
  {
    u16Len = strlen(pTxBuf);
    
    // Insert CR
    if ((u16Len >= 2) && (pTxBuf[u16Len-2] != '\r') && (pTxBuf[u16Len-1] == '\n'))
    {
      pTxBuf[u16Len-1] = '\r'; // CR
      pTxBuf[u16Len++] = '\n'; // LF
    }
  } 
  return u16Len;
}
*/

void Log_FileClose(bool bSysPowerOff)
{
  f_close(&SDFile);   
  Log_ShutDown( true, true, bSysPowerOff);   
  
  SetLEDRed(false); 
}

uint32_t Log_FileGetSize(void)
{
  bool      bSysPowerOff = false;
  uint32_t  u32Size = 0;
  
  if (Log_FileOpen(&bSysPowerOff,0))
  {
    u32Size = f_size(&SDFile);
    Log_FileClose(bSysPowerOff);
  }
  
  return u32Size;
}

bool Log_FileDelete(void)
{
  bool bSysPowerOff = !BSP_IsSystemVccON();
  bool bOk = false;
  bool bUnmount = false;
  bool bUnlink = false;
  
  SetLEDRed(true);  
  
  if (bSysPowerOff)
    BSP_SystemVcc(true);  // VCC System on  
  
  
    /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);
  if (!retSD) // 2. Volume 
  {   
    bUnlink = true;
    if(f_mount(&SDFatFS, SDPath, 0) == FR_OK)
    {       
      bUnmount = true;
      resLogFile = f_open(&SDFile, Log_GetFullFilename(), FA_OPEN_EXISTING | FA_READ);   
      if (resLogFile == FR_OK) 
      {
         f_close(&SDFile);  
         if (f_unlink(Log_GetFullFilename()) == FR_OK) // Delete File  
            bOk = true;
      }
      else if (resLogFile == FR_NO_FILE)
        bOk = true;
    }   
  }
  
  Log_ShutDown( bUnmount, bUnlink, bSysPowerOff);    
  
  SetLEDRed(false);  

  return bOk;
}

/*****END OF FILE****/
