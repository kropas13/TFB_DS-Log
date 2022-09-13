/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
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
//#include "hw.h" 
#include "rtcApp.h" 
#include "fatfs.h"
#include "LoRaMac-board.h"
#include "eeprom.h"
#include "loraModem.h"
#include "config.h"
#include "app.h"
#include "version.h"

// #define DEF_USE_IMST_STUDIO
#ifdef DEF_USE_IMST_STUDIO
  #define NON_LORAWAN_COMPLIANT_DEVICE    1 // Falls nicht LoRaWAN compliant (EUIs big endian) -> auf 1 setzen
#else
  #define NON_LORAWAN_COMPLIANT_DEVICE    0 // Falls LoRaWAN compliant (EUIs little endian) -> auf 0 setzen
#endif

// 1. Volume
const char str_CFG_FILE_INKL_PATH[]       = "0:/Cfg.TXT";     
const char str_DEF_CFG_FILE_INKL_PATH[]   = "0:/Def_Cfg.TXT";
const char str_DEL_CFG_FILE_INKL_PATH[]   = "0:/Del_Cfg.TXT";




uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */

uint8_t retUSER;    /* Return value for USER */
char USER_Path[4];  /* USER logical drive path */
FATFS UserDiscFatFs;  /* File system object for UserDiscFatFs logical drive */
FIL MyFile;     /* File object */
/* USER CODE BEGIN Variables */


bool ReadConfigFileData(uint8_t *pui8MajorVersion, uint8_t *pui8MinorVersion, uint8_t *pu8BandType, bool *pbParamError)
{
  bool        bOk = false;
  bool        bParamError;
  
  f_rewind (&MyFile);	
  *pui8MajorVersion = 0;
  *pui8MinorVersion = 0;
  *pu8BandType = 0; // 0=EU868; 1=US915
  *pbParamError = false;
  while (f_gets(MyStr, sizeof(MyStr)-1, &MyFile) > 0)
  {    
    if (!bOk)
    {
      if (MyStr[0] != 'A')
        break;
      bOk = true;
      
      // Get Version ".... :XX.YY\r"
      if ((strlen(MyStr) > 7) && (MyStr[strlen(MyStr)-7] == ':') && (MyStr[strlen(MyStr)-4] == '.'))
      {
        *pui8MajorVersion = (uint8_t)CfgGetDecValueMinMax(&MyStr[strlen(MyStr)-6], 0, 99, &bParamError);
        *pui8MinorVersion = (uint8_t)CfgGetDecValueMinMax(&MyStr[strlen(MyStr)-3], 0, 99, &bParamError);
        if (strlen(MyStr) > 12)
        {
          if (!strncmp(&MyStr[strlen(MyStr)-12], "US915", sizeof("US915")))
              *pu8BandType = 1;
        }
      }        
    }    

    CfgSetFromString(MyStr, pbParamError);
  }
  
  return bOk;
}

uint8_t WriteConfigFileData(void)
{
   uint8_t u8Success = 0;
   uint8_t u8Ch, u8Msk; 

   f_rewind (&MyFile);	
 #if defined( USE_BAND_868 )
   f_printf(&MyFile, "App.vers.EU868:%02d.%02d\n", APP_MAJOR_VERSION, APP_MINOR_VERSION); 
 #elif defined( USE_BAND_915 )
   f_printf(&MyFile, "App.vers.US915:%02d.%02d\n", APP_MAJOR_VERSION, APP_MINOR_VERSION); 
 #elif
    #error "No LoRaWAN band defined (e.g. USE_BAND_868) !"  
 #endif  
   f_printf(&MyFile, "\nLoraWAN V1.0.1 Config (LoRaMac version %X):\n", VERSION_LORA);	 
   f_printf(&MyFile, "%s=%c (0: Public Network, 1: Private Network)\n",strLoRaWANcfg_PrivateNetwork,(sCfg.sLoRaWANCfg.u1PrivateNetwork == 0) ? '0' : '1'); 
   f_printf(&MyFile, "%s=%c (0: ADR OFF, 1: ADR ON)\n", strLoRaWANcfg_ADR, (sCfg.sLoRaWANCfg.u1ADR == 0) ?  '0' : '1'); 
   f_printf(&MyFile, "%s=%c (0: ABP, 1: OTAA)\n", strLoRaWANcfg_OTAA, (sCfg.sLoRaWANCfg.u1OTA == 0) ? '0' : '1');   
   f_puts("\nOTAA (OverTheAirActivation):\n", &MyFile);       
   f_printf(&MyFile, "%s=%s\n", strLoRaWANcfg_DevEUI, CfgMakeHexStr(MyStr, sCfg.sLoRaWANCfg.OTA_DevEui, sizeof(sCfg.sLoRaWANCfg.OTA_DevEui), NON_LORAWAN_COMPLIANT_DEVICE));
   f_printf(&MyFile, "%s=%s\n", strLoRaWANcfg_AppEUI, CfgMakeHexStr(MyStr, sCfg.sLoRaWANCfg.OTA_AppEui, sizeof(sCfg.sLoRaWANCfg.OTA_AppEui), NON_LORAWAN_COMPLIANT_DEVICE));
   f_printf(&MyFile, "%s=%s\n", strLoRaWANcfg_AppKey, CfgMakeHexStr(MyStr, sCfg.sLoRaWANCfg.OTA_AppKey, sizeof(sCfg.sLoRaWANCfg.OTA_AppKey), 0)); 
   f_puts("\nABP (ActivationByPersonalization):\n", &MyFile);         
   f_printf(&MyFile, "%s=%08x\n", strLoRaWANcfg_DevAddr, sCfg.sLoRaWANCfg.ABP_DeviceAddr); 
   f_printf(&MyFile, "%s=%s\n", strLoRaWANcfg_NetwSesKey, CfgMakeHexStr(MyStr, sCfg.sLoRaWANCfg.ABP_NetworkSessionKey, sizeof(sCfg.sLoRaWANCfg.ABP_NetworkSessionKey), 0)); 
   f_printf(&MyFile, "%s=%s\n", strLoRaWANcfg_AppSesKey, CfgMakeHexStr(MyStr, sCfg.sLoRaWANCfg.ABP_ApplicationSessionKey, sizeof(sCfg.sLoRaWANCfg.ABP_ApplicationSessionKey), 0));
   f_puts("\n", &MyFile); 
   f_printf(&MyFile, "%s=0x%08x\n", strLoRaWANcfg_BC_Addr, sCfg.sLoRaWANCfg.BC_Addr); 
   f_printf(&MyFile, "%s=%s\n", strLoRaWANcfg_BC_NetwSesKey, CfgMakeHexStr(MyStr, sCfg.sLoRaWANCfg.BC_NetworkSessionKey, sizeof(sCfg.sLoRaWANCfg.BC_NetworkSessionKey), 0)); 
   f_printf(&MyFile, "%s=%s\n", strLoRaWANcfg_BC_AppSesKey, CfgMakeHexStr(MyStr, sCfg.sLoRaWANCfg.BC_ApplicationSessionKey, sizeof(sCfg.sLoRaWANCfg.BC_ApplicationSessionKey), 0));        
   
 #if defined( USE_BAND_868 )
   f_puts("\nLoRaMAC EU868 Datarate (0..5; DR_0..DR_5; SF12..SF7/BW125):\n", &MyFile); 		          
   f_printf(&MyFile, "%s=%d\n", strLoRaWANcfg_MinDataRate, sCfg.sLoRaWANCfg.ui8MinDatarate); 
   f_printf(&MyFile, "%s=%d\n", strLoRaWANcfg_MaxDataRate, sCfg.sLoRaWANCfg.ui8MaxDatarate); 
   f_printf(&MyFile, "%s=%d\n", strLoRaWANcfg_DefaultDataRate, sCfg.sLoRaWANCfg.ui8DefaultDatarate); 
   f_printf(&MyFile, "%s=%d\n", strLoRaWANcfg_Rx2DefaultDataRate, sCfg.sLoRaWANCfg.ui8Rx2DefaultDatarate);	   
 #elif defined( USE_BAND_915 )
   f_puts("\nLoRaMAC US915 Datarate:\n", &MyFile); //0=SF10/BW125; 1=SF9/BW125; 2=SF8/BW125; 3=SF7/BW125; 4=SF8/BW500
   f_printf(&MyFile, "%s=%d (Uplink 0..4; DR_0..DR_4; SF10..7/BW125+SF8/BW500)\n", strLoRaWANcfg_DefaultDataRate, sCfg.sLoRaWANCfg.ui8DefaultDatarate); 
   f_printf(&MyFile, "%s=%d (Downlink 8..13; DR_8..DR_13; SF10..7/BW125+SF8/BW500)\n", strLoRaWANcfg_Rx2DefaultDataRate, sCfg.sLoRaWANCfg.ui8Rx2DefaultDatarate);   
 #endif   
   
   f_puts("\nTFB:\n", &MyFile);
   f_printf(&MyFile, "%s=%04d (0000..9999 minutes, 0000 for no interval)\n", strAppCfg_SendInterval, sCfg.sAppCfg.u32AppTxInterval_m); 
   f_printf(&MyFile, "%s=%02d (00..99 seconds from powering to measure Voltage and Current)\n\n", strAppCfg_ExtSensorsStartupTime, sCfg.sAppCfg.u8ExtSensorsStartupTime_s); 
   f_printf(&MyFile, "     TemperatureSensorType (00:PT1000_2Wire 01:PT1000_3Wire 02:PT100_2Wire 03:PT100_3Wire 04:TC_E 05:TC_J 06:TC_K 07:TC_N 08:TC_R 09:TC_S 10:TC_T)\n");
   f_printf(&MyFile, "     |  Voltage range (0:0..48V 1:0..6,5V)\n");    
   f_printf(&MyFile, "     |	| Sensor data (0: not used; 1: measured and send in LoRa-Uplink)\n"); 
   f_printf(&MyFile, "     |  | Temperature\n");
   f_printf(&MyFile, "     |  | | Voltage\n");
   f_printf(&MyFile, "     |  | | | Current\n");
   f_printf(&MyFile, "     |  | | | | Corr.voltage\n");
   f_printf(&MyFile, "     |  | | | | | Corr.current\n");
   f_printf(&MyFile, "     |  | | | | | | Impedance\n");
   f_printf(&MyFile, "     |  | | | | | | | Phase angle\n");
   f_printf(&MyFile, "     |  | | | | | | | | Resistance (real)\n"); 
   
   for (u8Ch=0, u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++, u8Msk <<= 1)
     f_printf(&MyFile, "%s%d=%02d;%d;%d;%d;%d;%d;%d;%d;%d;%d\n", strAppCfg_Ch_, u8Ch+1, sCfg.sAppCfg.au8TemperatureSensorType[u8Ch]%100, (sCfg.sAppCfg.u8ChMskVoltageRangeHighRes & u8Msk) ? 1 : 0,      
      (sCfg.sAppCfg.u8ChMskTemperature & u8Msk) ? 1 : 0, 
      (sCfg.sAppCfg.u8ChMskVoltage & u8Msk) ? 1 : 0, 
      (sCfg.sAppCfg.u8ChMskCurrent & u8Msk) ? 1 : 0, 
      (sCfg.sAppCfg.u8ChMskCorrVoltage & u8Msk) ? 1 : 0, 
      (sCfg.sAppCfg.u8ChMskCorrCurrent & u8Msk) ? 1 : 0, 
      (sCfg.sAppCfg.u8ChMskImpedance & u8Msk) ? 1 : 0,
      (sCfg.sAppCfg.u8ChMskPhaseAngle & u8Msk) ? 1 : 0,
      (sCfg.sAppCfg.u8ChMskResistance & u8Msk) ? 1 : 0);
   if (f_puts("\n", &MyFile) != EOF){
      f_truncate(&MyFile); 
      f_rewind (&MyFile);	
      if (f_puts("A", &MyFile)!= EOF){ // First char should be a big 'A' to mark as written complete
         u8Success = 1;
		}	
  } else {
    // Error_Handler();
  }  
  
  return u8Success;
}

/* USER CODE END Variables */    

void MX_FATFS_Init(void) 
{
  /* variables used to be able to actualize app version and subversion on the go */
  uint8_t u8AppVersion = 0;
	uint8_t u8AppSubVersion = 0;	
  uint8_t u8BandType = 0;
  bool    bParamError = false;
  
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USER_Path);  // 1. Volume  
  
  /*## FatFS: Link the SD driver ###########################*/
  // retSD = FATFS_LinkDriver(&SD_Driver, SDPath);  // 2. Volume will be linked in log.c because of SDCard powering on and off (and removing SD Card)
  
  
  /* USER CODE BEGIN Init */
  /* additional user code for init */ 
  if(f_mount(&UserDiscFatFs, (TCHAR const*)USER_Path, 0) != FR_OK)
  {
     // Error_Handler();
  }
  else 
  {     
    if(f_open(&MyFile, str_CFG_FILE_INKL_PATH, FA_OPEN_EXISTING | FA_WRITE | FA_READ) != FR_OK){   
      // Debug Command
      // SAVE EEPROM.hex 0x08080000, 0x08081FFF
      // Win CMD
      // Win+R
      // cmd
      // e:
      // cd E:\Users\danie_000\Documents\STM32Cube\Projects\TeaserUSB_MSC_FATFS\MDK-ARM\
      // HEX2BIN.EXE EEPROM.hex
      if ( f_mkfs ( (TCHAR const*)USER_Path,	1, 1) != FR_OK){
        // Error_Handler(); 
      }
			
      f_setlabel(VOL_LABEL_NAME);

      if(f_open(&MyFile, str_CFG_FILE_INKL_PATH, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK){
        // Error_Handler();
      } else {
        WriteConfigFileData();
      }
    } else if (ReadConfigFileData(&u8AppVersion, &u8AppSubVersion, &u8BandType, &bParamError)) {
      #if defined (USE_BAND_915 )
        if ((u8AppVersion != APP_MAJOR_VERSION) || (u8AppSubVersion != APP_MINOR_VERSION) || (u8BandType != 1) || bParamError) {      
      #else
        if ((u8AppVersion != APP_MAJOR_VERSION) || (u8AppSubVersion != APP_MINOR_VERSION) || (u8BandType > 0) || bParamError) {
      #endif
			  WriteConfigFileData(); //REWRITE CFG FILE IF VERSION NOT CORRECT   
   		}		
      
      // Check we got other app data than the EEPROM has
      if (memcmp(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg)))
      {
        memcpy(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg) );
        EEPROMWriteAppConfig();
      }            
    } else {
		  WriteConfigFileData();
    }
		
    if (f_close(&MyFile) != FR_OK )
    {
      // Error_Handler();
    }
       
    if(f_open(&MyFile, str_DEF_CFG_FILE_INKL_PATH, FA_OPEN_EXISTING | FA_READ) == FR_OK){                
      ReadConfigFileData(&u8AppVersion, &u8AppSubVersion, &u8BandType, &bParamError);    
      f_close(&MyFile);
      f_unlink(str_DEF_CFG_FILE_INKL_PATH); // Delete File
      f_unlink(str_CFG_FILE_INKL_PATH); // Delete File			
      memcpy(&sEEPROM.sLoRaDef.sLoRaWANCfg, &sCfg.sLoRaWANCfg, sizeof(tLoRaWAN_Cfg));
			memcpy(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg));
			//Write defaults to EEPROM
      EEPROMWriteLoRaDefaultConfig();
			EEPROMWriteAppConfig();	
      // SW-Reset, file will be rewritten with new default values
      __disable_irq( );
      NVIC_SystemReset();      
    }  
    if(f_open(&MyFile, str_DEL_CFG_FILE_INKL_PATH, FA_OPEN_EXISTING | FA_READ) == FR_OK)
    {           
      f_close(&MyFile);
      f_unlink(str_DEL_CFG_FILE_INKL_PATH); // Delete File
      f_unlink(str_CFG_FILE_INKL_PATH); // Delete File
      appSetDefault();
      memcpy(&sEEPROM.sLoRaDef.sLoRaWANCfg, &sCfg.sLoRaWANCfg, sizeof(tLoRaWAN_Cfg));
			memcpy(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg));
			//Write defaults to EEPROM
      EEPROMWriteLoRaDefaultConfig();
			EEPROMWriteAppConfig();				
			// SW-Reset, file will be rewritten with hardcoded default values
      __disable_irq( );
      NVIC_SystemReset(); 
    }
  }
  /* USER CODE END Init */
}

/**
  * @brief  Rewrite file after startup in case some downlink change happened 
  * @param  None
  * @retval None
  */
void ReWriteConfigFileData (void)
{ 
  if (f_open(&MyFile, str_CFG_FILE_INKL_PATH, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK){   
    WriteConfigFileData();
  }
  
	if (f_close(&MyFile) != FR_OK ){
    // Error_Handler();
  }  
}

bool Cfg_FileOpen(void)
{
  if (f_open(&MyFile, str_CFG_FILE_INKL_PATH, FA_OPEN_EXISTING | FA_READ) == FR_OK)
    return true;
  else
    return false;
}

uint16_t Cfg_FileRead(char* pTxBuf, uint16_t ui16TxBuffLen)
{
  uint32_t u32Len = 0;
  
  f_read( &MyFile, pTxBuf, ui16TxBuffLen, &u32Len);
  
  return u32Len;
}

void Cfg_FileClose(void)
{
   f_close(&MyFile);    
}

/**
  * @brief  Gets Time from RTC 
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return App_RTC_GetDateTimeStamp(); // *( uint32_t* )ID1;
  /* USER CODE END get_fattime */  
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
