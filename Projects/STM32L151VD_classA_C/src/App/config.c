/**============================================================================
* @file      config.c
* @date      2018-06-11
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     main file for configuration
*            
* VERSION:   
* 
* V0.01            2018-06-11-Kd      Create File 		
*
*============================================================================*/  

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "eeprom.h"
#include "fatfs.h"
#include "bsp.h"
#include "config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/*!
 * Defines the application data transmission duty cycle [ms].
 */
#define APP_TX_DUTYCYCLE                (60*1000) // 1 Minute

/* Private macro -------------------------------------------------------------*/
#define __CONFIG_IS_CFG(__CFG__, __STR__)                        \
  (!strncmp(__CFG__, __STR__, sizeof(__CFG__)-1))
    
/* Private function prototypes -----------------------------------------------*/   
/* Private variables ---------------------------------------------------------*/
// Comm Cmds
const char strCom_CfgGet[]              = "CfgGet_";
const char strCom_CfgGetDef[]           = "CfgGetDef_";   // only LoRa Config has a seperate factory default (in EEPROM), specially for the DevEUI
const char strCom_CfgSet[]              = "CfgSet_";      // Edit + Save
const char strCom_CfgSetAsDef[]         = "CfgSetAsDef_"; // Edit + Save (as factory default)
const char strCom_CfgEdit[]             = "CfgEdit_";     // only edit
const char strCom_CfgSave[]             = "CfgSave_";     // save
const char strCom_CfgSaveAsDef[]        = "CfgSaveAsDef_";// save as factory default (only LoRa Config)
const char strCom_CfgDelDef[]           = "CfgDelDef_";   // delete factory default (only LoRa Config) and set to hard coded defaults
const char strCom_CfgUseDef[]           = "CfgUseDef_";   // use factory default (only LoRa Config)

// Config Strings
//LoRa Parameters
// const char strLoRaWANcfg_SendInterval[]   = "SendInterval";

const char strLoRaWANcfg_PrivateNetwork[] = "PrivateNetwork";
const char strLoRaWANcfg_ADR[]            = "ADR"; 
const char strLoRaWANcfg_OTAA[]           = "OTAA";
const char strLoRaWANcfg_DevEUI[]         = "DevEUI";
const char strLoRaWANcfg_AppEUI[]         = "AppEUI";
const char strLoRaWANcfg_AppKey[]         = "AppKey";
const char strLoRaWANcfg_NetwID[]         = "NetwID";
const char strLoRaWANcfg_DevAddr[]        = "DevAddr";
const char strLoRaWANcfg_NetwSesKey[]     = "NetwSesKey";
const char strLoRaWANcfg_AppSesKey[]      = "AppSesKey";

const char strLoRaWANcfg_BC_Addr[]        = "BroadcastAddr";
const char strLoRaWANcfg_BC_NetwSesKey[]  = "BroadcastNetwSesKey";
const char strLoRaWANcfg_BC_AppSesKey[]   = "BroadcastAppSesKey";

const char strLoRaWANcfg_MinDataRate[]        = "MinDatarate";
const char strLoRaWANcfg_MaxDataRate[]        = "MaxDatarate";
const char strLoRaWANcfg_DefaultDataRate[]    = "DefDatarate";
const char strLoRaWANcfg_Rx2DefaultDataRate[] = "Rx2DefDatarate";

//App Parameters
const char strAppCfg_SendInterval[]           = "SendInterval";
const char strAppCfg_ExtSensorsStartupTime[]  = "ExtSensorsStartupTime";
const char strAppCfg_Ch_[]                    = "Ch_";


//const char strAppCfg_ConfirmedTx[]         = "ConfirmedTx";
//const char strAppCfg_LivesignConfirmedTx[] = "LivesignConfirmedTx";
//const char strAppCfg_ConfirmedTxTimeout[]  = "ConfirmedTxTimeout";
//const char strAppCfg_RxConfirmTimeout[]    = "RxConfirmTimeout";

//const char strAppCfg_SendOnChange[]            = "SendOnChange";
//const char strAppCfg_MinSendOnChangeInterval[] = "MinSendOnChangeInterval";

const char aryHex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

/*!
 * Configuration
 */  
char MyStr[200];
tCfg sCfg, sCfg_tmp; 

                                                                      
/* Private functions ---------------------------------------------------------*/
uint8_t GetHexValueAry(char* str, uint8_t ary[], uint8_t cnt, uint8_t nonCompliantDevice)
{
  uint8_t pos = 0;
  uint8_t nibble_cnt = 0;
  uint8_t idx;
  char    chr;
  uint8_t val = 0;
  uint8_t ret_cnt = 0;
  
  if (nonCompliantDevice)
    idx = cnt-1;
  else
    idx = 0;      
  
  do 
  {
    chr = str[pos++];
    if (nibble_cnt == 1)
      val *= 0x10;      
    
    if ((chr >= '0') && (chr <= '9'))
    {
      val += chr - '0';
      nibble_cnt++;
    }
    else if ((chr >= 'A') && (chr <= 'F'))
    {
      val += chr - 'A' + 10; 
      nibble_cnt++;
    }
    else if ((chr >= 'a') && (chr <= 'f'))
    {
      val += chr - 'a' + 10; 
      nibble_cnt++;
    }

    if (nibble_cnt == 2)
    {         
      if (ret_cnt++ < cnt)
      {
        ary[idx] = val;
        if (nonCompliantDevice)
          idx--;
        else
          idx++;                  
      }
      val = 0;
      nibble_cnt = 0;
    }
  } while (str[pos]);
  
  return ret_cnt;
}

/*
uint32_t GetUnsignedDecValueMinMax(char *pStr, uint32_t u32MinVal, uint32_t u32MaxVal)
{
  uint32_t u32Val;
  
  u32Val = strtoul(pStr, NULL, 10);
  if (u32Val < u32MinVal)
    u32Val = u32MinVal;
  else if (u32Val > u32MaxVal)
    u32Val = u32MaxVal;
  
  return u32Val;
}
*/

bool GoNextSemikolon(char **ppStr)
{
  bool bSemikolonFound = false;
  
  while ((**ppStr != '\n') && (**ppStr != 0))
  {
    if (**ppStr == ';')
    {
      (*ppStr)++;
      bSemikolonFound = true;
      break;
    }
    else
      (*ppStr)++;
  }
  
  return bSemikolonFound;
}


/* Public functions ---------------------------------------------------------*/
int32_t CfgGetDecValueMinMax(char *pStr, int32_t i32MinVal, int32_t i32MaxVal, bool *pbParamError)
{
  int32_t i32Val;
  
  
  i32Val = atol(pStr);
  if (i32Val < i32MinVal)
  {
    i32Val = i32MinVal;
    *pbParamError = true;
  }
  else if (i32Val > i32MaxVal)
  {
    i32Val = i32MaxVal;
    *pbParamError = true;
  }
  
  return i32Val;
}

uint32_t GetUnsignedDecValueMinMax(char *pStr, uint32_t u32MinVal, uint32_t u32MaxVal, bool *pbParamError)
{
  uint32_t u32Val;
  
  u32Val = strtoul(pStr, NULL, 10);
  if (u32Val < u32MinVal)
  {
    u32Val = u32MinVal;
    *pbParamError = true;
  }
  else if (u32Val > u32MaxVal)
  {
    u32Val = u32MaxVal;
    *pbParamError = true;
  }
  
  return u32Val;
}

char* CfgMakeHexStr(char* str, uint8_t ary[], uint8_t cnt , uint8_t nonCompliantDevice)
{
  uint8_t pos = 0;
  uint8_t idx;
  
  if (nonCompliantDevice)
    idx = cnt-1;
  else
    idx = 0;  
    
  while (1)
  {
    str[pos++] = aryHex[(ary[idx] >> 4) & 0x0f];
    str[pos++] = aryHex[ary[idx] & 0x0f];
    if (!nonCompliantDevice)
    {
      if (++idx >= cnt)
        break;
    }
    else if (!idx)
      break;
    else
      idx--;

    #ifdef DEF_USE_IMST_STUDIO
      str[pos++] = '-';
    #endif
  }
  str[pos++] = 0;
  
  return str;
}

void CfgSetFromString(char* pMyStr, bool* pbParamError)
{
  uint8_t     tempAry[16];
  uint8_t     ui8Val, ui8Msk; 
  char        *pStrValue;  
  
  // Get the '='
  for (pStrValue = pMyStr; *pStrValue; pStrValue++)
  {
    if (*pStrValue == '=')
    {
      pStrValue++;
      
      if ( __CONFIG_IS_CFG(strLoRaWANcfg_PrivateNetwork, pMyStr) )   
      {
        sCfg.sLoRaWANCfg.u1PrivateNetwork = (uint8_t)CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError);                         
      }
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_OTAA, pMyStr) )      
      {
        sCfg.sLoRaWANCfg.u1OTA = (uint8_t)CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError);                         
      }
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_ADR, pMyStr) )      
      {
        sCfg.sLoRaWANCfg.u1ADR = (uint8_t)CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError);                         
      }
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_DevEUI, pMyStr) )
      {
        if (GetHexValueAry(pStrValue, tempAry, sizeof(sCfg.sLoRaWANCfg.OTA_DevEui), NON_LORAWAN_COMPLIANT_DEVICE) == sizeof(sCfg.sLoRaWANCfg.OTA_DevEui))
          memcpy( sCfg.sLoRaWANCfg.OTA_DevEui, tempAry, sizeof(sCfg.sLoRaWANCfg.OTA_DevEui));
      }        
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_AppEUI, pMyStr) )
      {
        if (GetHexValueAry(pStrValue, tempAry, sizeof(sCfg.sLoRaWANCfg.OTA_AppEui), NON_LORAWAN_COMPLIANT_DEVICE) == sizeof(sCfg.sLoRaWANCfg.OTA_AppEui))
          memcpy( sCfg.sLoRaWANCfg.OTA_AppEui, tempAry, sizeof(sCfg.sLoRaWANCfg.OTA_AppEui));
      }                
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_AppKey, pMyStr) )
      {
        if (GetHexValueAry(pStrValue, tempAry, sizeof(sCfg.sLoRaWANCfg.OTA_AppKey), 0) == sizeof(sCfg.sLoRaWANCfg.OTA_AppKey))
          memcpy( sCfg.sLoRaWANCfg.OTA_AppKey, tempAry, sizeof(sCfg.sLoRaWANCfg.OTA_AppKey));
      }  
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_NetwID, pMyStr) )
      {
        sCfg.sLoRaWANCfg.ABP_NetworkID = strtol(pStrValue,(char**)0, 0);      
      }
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_DevAddr, pMyStr) )      
      {
        sCfg.sLoRaWANCfg.ABP_DeviceAddr = strtol(pStrValue,(char**)0, 16);      
      }
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_NetwSesKey, pMyStr) )
      {
        if (GetHexValueAry(pStrValue, tempAry, sizeof(sCfg.sLoRaWANCfg.ABP_NetworkSessionKey), 0) == sizeof(sCfg.sLoRaWANCfg.ABP_NetworkSessionKey))
          memcpy( sCfg.sLoRaWANCfg.ABP_NetworkSessionKey, tempAry, sizeof(sCfg.sLoRaWANCfg.ABP_NetworkSessionKey));
        else
          *pbParamError = true;
      }   
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_AppSesKey, pMyStr) )
      {
        if (GetHexValueAry(pStrValue, tempAry, sizeof(sCfg.sLoRaWANCfg.ABP_ApplicationSessionKey), 0) == sizeof(sCfg.sLoRaWANCfg.ABP_ApplicationSessionKey))
          memcpy( sCfg.sLoRaWANCfg.ABP_ApplicationSessionKey, tempAry, sizeof(sCfg.sLoRaWANCfg.ABP_ApplicationSessionKey));
        else
          *pbParamError = true;
      }  
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_BC_Addr, pMyStr) )
      {
        sCfg.sLoRaWANCfg.BC_Addr = strtol(pStrValue,(char**)0, 0);
      }            
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_BC_NetwSesKey, pMyStr) )
      {
        if (GetHexValueAry(pStrValue, tempAry, sizeof(sCfg.sLoRaWANCfg.BC_NetworkSessionKey), 0) == sizeof(sCfg.sLoRaWANCfg.BC_NetworkSessionKey))
          memcpy( sCfg.sLoRaWANCfg.BC_NetworkSessionKey, tempAry, sizeof(sCfg.sLoRaWANCfg.BC_NetworkSessionKey));
        else
          *pbParamError = true;
      }
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_BC_AppSesKey, pMyStr) )
      {
        if (GetHexValueAry(pStrValue, tempAry, sizeof(sCfg.sLoRaWANCfg.BC_ApplicationSessionKey), 0) == sizeof(sCfg.sLoRaWANCfg.BC_ApplicationSessionKey))
          memcpy( sCfg.sLoRaWANCfg.BC_ApplicationSessionKey, tempAry, sizeof(sCfg.sLoRaWANCfg.BC_ApplicationSessionKey));
        else
          *pbParamError = true;
      }
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_MinDataRate, pMyStr) )
      {
        ui8Val = (uint8_t)atol(pStrValue);     
        if (ui8Val <= LORAMAC_TX_MAX_DATARATE)
          sCfg.sLoRaWANCfg.ui8MinDatarate = ui8Val;
        else
          *pbParamError = true;
      }         
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_MaxDataRate, pMyStr) )
      {
        ui8Val = (uint8_t)atol(pStrValue);     
        if (ui8Val <= LORAMAC_TX_MAX_DATARATE)
          sCfg.sLoRaWANCfg.ui8MaxDatarate = ui8Val;
        else
          *pbParamError = true;
      } 
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_DefaultDataRate, pMyStr) )
      {
        ui8Val = (uint8_t)atol(pStrValue);   
        if (ui8Val <= LORAMAC_TX_MAX_DATARATE)
          sCfg.sLoRaWANCfg.ui8DefaultDatarate = ui8Val;
        else
          *pbParamError = true;
      }
      else if ( __CONFIG_IS_CFG(strLoRaWANcfg_Rx2DefaultDataRate, pMyStr) )
      {
        ui8Val = (uint8_t)atol(pStrValue);
        if ((ui8Val >= LORAMAC_RX_MIN_DATARATE) && (ui8Val <= LORAMAC_RX_MAX_DATARATE))
          sCfg.sLoRaWANCfg.ui8Rx2DefaultDatarate = ui8Val;
        else
          *pbParamError = true;
      } 
//      else if ( __CONFIG_IS_CFG(strCfg_ConfirmedTx, pMyStr) )
//        sCfg.sLoRaWANCfg.IsConfirmedTx = ((uint8_t)CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError) == 0) ? DISABLE : ENABLE;                   
//      else if ( __CONFIG_IS_CFG(strCfg_LivesignConfirmedTx, pMyStr) )
//        sCfg.sLoRaWANCfg.ui16LivesignConfirmedTx_m = (uint16_t)CfgGetDecValueMinMax(pStrValue, 0, 9999, pbParamError);
//      else if ( __CONFIG_IS_CFG(strCfg_ConfirmedTxTimeout, pMyStr) )
//        sCfg.sLoRaWANCfg.ui16ConfirmedTxTimeout_s = (uint16_t)CfgGetDecValueMinMax(pStrValue, 0, 9999, pbParamError);         
//      else if ( __CONFIG_IS_CFG(strCfg_RxConfirmTimeout, pMyStr) )
//        sCfg.sLoRaWANCfg.ui16RxConfirmTimeout_s = (uint16_t)CfgGetDecValueMinMax(pStrValue, 0, 9999, pbParamError);           
      else if ( __CONFIG_IS_CFG(strAppCfg_SendInterval, pMyStr) )
      {
        sCfg.sAppCfg.u32AppTxInterval_m = (uint16_t)CfgGetDecValueMinMax(pStrValue, 0, 9999, pbParamError); 
      }
//      else if ( __CONFIG_IS_CFG(strCfg_SendOnChange, pMyStr) )
//        sCfg.sApp.ui2SendOnChange = (uint8_t)CfgGetDecValueMinMax(pStrValue, 0, 3, pbParamError);                       
//      else if ( __CONFIG_IS_CFG(strCfg_MinSendOnChangeInterval, pMyStr) )
//        sCfg.sApp.ui16MinSendOnChangeInterval_m = (uint16_t)CfgGetDecValueMinMax(pStrValue, 0, 9999, pbParamError);
      else if ( __CONFIG_IS_CFG(strAppCfg_ExtSensorsStartupTime, pMyStr) )
      {
        sCfg.sAppCfg.u8ExtSensorsStartupTime_s = (uint8_t)CfgGetDecValueMinMax(pStrValue, 0, 99, pbParamError); 
      }       
      else if ( __CONFIG_IS_CFG(strAppCfg_Ch_, pMyStr) )
      {
        while (1)
        {
          ui8Val = atol(pStrValue-2);
          if ((ui8Val<1) || (ui8Val>8))
            break;
          ui8Val--;
          ui8Msk = 0x01 << ui8Val;
          sCfg.sAppCfg.au8TemperatureSensorType[ui8Val] = (t_eMEASExtSenseTempType)CfgGetDecValueMinMax(pStrValue, eMEAS_EXTSENSETEMPTYPE_TC_First, eMEAS_EXTSENSETEMPTYPE_TC_Last, pbParamError); 
          if (!GoNextSemikolon(&pStrValue))
            break; 
          // Next param          
          if (CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError))
            sCfg.sAppCfg.u8ChMskVoltageRangeHighRes |= ui8Msk;
          else
            sCfg.sAppCfg.u8ChMskVoltageRangeHighRes &= ~ui8Msk;          
          if (!GoNextSemikolon(&pStrValue))
            break;                     
          // Next param
          if (CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError))
            sCfg.sAppCfg.u8ChMskTemperature |= ui8Msk;
          else
            sCfg.sAppCfg.u8ChMskTemperature &= ~ui8Msk;          
          if (!GoNextSemikolon(&pStrValue))
            break; 
          // Next param
          if (CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError))
            sCfg.sAppCfg.u8ChMskVoltage |= ui8Msk;
          else
            sCfg.sAppCfg.u8ChMskVoltage &= ~ui8Msk;                    
          if (!GoNextSemikolon(&pStrValue))
            break; 
          // Next param          
          if (CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError))
            sCfg.sAppCfg.u8ChMskCurrent |= ui8Msk;
          else
            sCfg.sAppCfg.u8ChMskCurrent &= ~ui8Msk;
          if (!GoNextSemikolon(&pStrValue))
            break; 
          // Next param
          if (CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError))
            sCfg.sAppCfg.u8ChMskCorrVoltage |= ui8Msk;
          else
            sCfg.sAppCfg.u8ChMskCorrVoltage &= ~ui8Msk;
          if (!GoNextSemikolon(&pStrValue))
            break; 
          // Next param
          if (CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError))
            sCfg.sAppCfg.u8ChMskCorrCurrent |= ui8Msk;
          else
            sCfg.sAppCfg.u8ChMskCorrCurrent &= ~ui8Msk;
          if (!GoNextSemikolon(&pStrValue))
            break; 
          // Next param
          if (CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError))
            sCfg.sAppCfg.u8ChMskImpedance |= ui8Msk;
          else
            sCfg.sAppCfg.u8ChMskImpedance &= ~ui8Msk;   
          // 2018-10-03-Kd new V1.0
          if (!GoNextSemikolon(&pStrValue))
            break; 
          // Next param
          if (CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError))
            sCfg.sAppCfg.u8ChMskPhaseAngle |= ui8Msk;
          else
            sCfg.sAppCfg.u8ChMskPhaseAngle &= ~ui8Msk;   
          if (!GoNextSemikolon(&pStrValue))
            break; 
          // Next param
          if (CfgGetDecValueMinMax(pStrValue, 0, 1, pbParamError))
            sCfg.sAppCfg.u8ChMskResistance |= ui8Msk;
          else
            sCfg.sAppCfg.u8ChMskResistance &= ~ui8Msk;             
          break;
        }
      }
    }
  }    
}

uint16_t CfgGetFromString(char* pMyStr, char* pTxBuffer, tCfg *pCfg)
{
  uint16_t  ui16BuffLen = 0;
  uint8_t   ui8Val, ui8Msk;
  bool      bParamError;
                  
  if ( __CONFIG_IS_CFG(strLoRaWANcfg_PrivateNetwork, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%c", (pCfg->sLoRaWANCfg.u1PrivateNetwork == 0) ? '0' : '1');                     
  }
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_OTAA, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%c", (pCfg->sLoRaWANCfg.u1OTA == 0) ? '0' : '1');                     
  }  
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_DevEUI, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%s", CfgMakeHexStr(MyStr, pCfg->sLoRaWANCfg.OTA_DevEui, sizeof(pCfg->sLoRaWANCfg.OTA_DevEui), NON_LORAWAN_COMPLIANT_DEVICE));
  }        
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_AppEUI, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%s", CfgMakeHexStr(MyStr, pCfg->sLoRaWANCfg.OTA_AppEui, sizeof(pCfg->sLoRaWANCfg.OTA_AppEui), NON_LORAWAN_COMPLIANT_DEVICE));
  }                
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_AppKey, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%s", CfgMakeHexStr(MyStr, pCfg->sLoRaWANCfg.OTA_AppKey, sizeof(pCfg->sLoRaWANCfg.OTA_AppKey), 0)); 
  }  
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_NetwID, pMyStr) )
  {
    // not used pCfg->sLoRaWANCfg.ABP_NetworkID = strtol(pStrValue,(char**)0, 0);
  }        
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_DevAddr, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%08x", pCfg->sLoRaWANCfg.ABP_DeviceAddr); 
  }           
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_NetwSesKey, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%s", CfgMakeHexStr(MyStr, pCfg->sLoRaWANCfg.ABP_NetworkSessionKey, sizeof(pCfg->sLoRaWANCfg.ABP_NetworkSessionKey), 0)); 
  }   
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_AppSesKey, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%s", CfgMakeHexStr(MyStr, pCfg->sLoRaWANCfg.ABP_ApplicationSessionKey, sizeof(pCfg->sLoRaWANCfg.ABP_ApplicationSessionKey), 0));
  }  
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_BC_Addr, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=0x%08x", pCfg->sLoRaWANCfg.BC_Addr); 
  }            
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_BC_NetwSesKey, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%s", CfgMakeHexStr(MyStr, pCfg->sLoRaWANCfg.BC_NetworkSessionKey, sizeof(pCfg->sLoRaWANCfg.BC_NetworkSessionKey), 0)); 
  }
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_BC_AppSesKey, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%s", CfgMakeHexStr(MyStr, pCfg->sLoRaWANCfg.BC_ApplicationSessionKey, sizeof(pCfg->sLoRaWANCfg.BC_ApplicationSessionKey), 0));
  }
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_MinDataRate, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%d", pCfg->sLoRaWANCfg.ui8MinDatarate); 
  }         
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_MaxDataRate, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%d", pCfg->sLoRaWANCfg.ui8MaxDatarate); 
  } 
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_DefaultDataRate, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%d", pCfg->sLoRaWANCfg.ui8DefaultDatarate); 
  }
  else if ( __CONFIG_IS_CFG(strLoRaWANcfg_Rx2DefaultDataRate, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%d", pCfg->sLoRaWANCfg.ui8Rx2DefaultDatarate);  
  } 
//  else if ( __CONFIG_IS_CFG(strCfg_ConfirmedTx, pMyStr) )
//  {
//    ui16BuffLen = sprintf(pTxBuffer, "=%c", (pCfg->sLoRaWANCfg.IsConfirmedTx == DISABLE) ? '0' : '1');                   
//  }
//  else if ( __CONFIG_IS_CFG(strCfg_LivesignConfirmedTx, pMyStr) )
//  {
//    ui16BuffLen = sprintf(pTxBuffer, "=%04d", pCfg->sLoRaWANCfg.ui16LivesignConfirmedTx_m);
//  } 
//  else if ( __CONFIG_IS_CFG(strCfg_ConfirmedTxTimeout, pMyStr) )
//  {
//    ui16BuffLen = sprintf(pTxBuffer, "=%04d", pCfg->sLoRaWANCfg.ui16ConfirmedTxTimeout_s);
//  }         
//  else if ( __CONFIG_IS_CFG(strCfg_RxConfirmTimeout, pMyStr) )
//  {
//    ui16BuffLen = sprintf(pTxBuffer, "=%04d", pCfg->sLoRaWANCfg.ui16RxConfirmTimeout_s); 
//  }                
  else if ( __CONFIG_IS_CFG(strAppCfg_SendInterval, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%04d", pCfg->sAppCfg.u32AppTxInterval_m);   
  }   
//  else if ( __CONFIG_IS_CFG(strCfg_SendOnChange, pMyStr) )
//  {
//    ui16BuffLen = sprintf(pTxBuffer, "=%d", pCfg->sApp.ui2SendOnChange);                     
//  }  
//  else if ( __CONFIG_IS_CFG(strCfg_MinSendOnChangeInterval, pMyStr) )
//  {
//    ui16BuffLen = sprintf(pTxBuffer, "=%04d", pCfg->sApp.ui16MinSendOnChangeInterval_m);   
//  }  

  else if ( __CONFIG_IS_CFG(strAppCfg_ExtSensorsStartupTime, pMyStr) )
  {
    ui16BuffLen = sprintf(pTxBuffer, "=%03d", pCfg->sAppCfg.u8ExtSensorsStartupTime_s);   
  }  
  else if ( __CONFIG_IS_CFG(strAppCfg_Ch_, pMyStr) )
  {
    ui8Val = CfgGetDecValueMinMax( &pMyStr[sizeof(strAppCfg_Ch_)-1], 1, EXT_SENSE_CHANNELS, &bParamError)-1; 
    ui8Msk = 0x01 << ui8Val;     
    ui16BuffLen = sprintf(pTxBuffer, "=%02d;%d;%d;%d;%d;%d;%d;%d;%d\n", sCfg.sAppCfg.au8TemperatureSensorType[ui8Val]%100,      
      (sCfg.sAppCfg.u8ChMskTemperature & ui8Msk) ? 1 : 0, 
      (sCfg.sAppCfg.u8ChMskVoltage & ui8Msk) ? 1 : 0, 
      (sCfg.sAppCfg.u8ChMskCurrent & ui8Msk) ? 1 : 0, 
      (sCfg.sAppCfg.u8ChMskCorrVoltage & ui8Msk) ? 1 : 0, 
      (sCfg.sAppCfg.u8ChMskCorrCurrent & ui8Msk) ? 1 : 0, 
      (sCfg.sAppCfg.u8ChMskImpedance & ui8Msk) ? 1 : 0,
      (sCfg.sAppCfg.u8ChMskPhaseAngle & ui8Msk) ? 1 : 0,
      (sCfg.sAppCfg.u8ChMskResistance & ui8Msk) ? 1 : 0);
  }  

  return ui16BuffLen;
}

void CfgSetInitialDefault(void)
{  
  memset(&sCfg, 0, sizeof(sCfg));
  
  appSetDefault();
}

void CfgSetFactoryDefault(void)
{  
  memcpy(&sCfg.sLoRaWANCfg, &sEEPROM.sLoRaDef.sLoRaWANCfg, sizeof(tLoRaWAN_Cfg));
}  

void CfgSaveAppConfig( void )
{
  memcpy(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg) );
  EEPROMWriteAppConfig();  // Write the config to the EEPROM  
  ReWriteConfigFileData(); // Write the config to the EEPROM-Disk  
}


void CfgSaveAsFactoryDefault(bool b_restart)
{
  memcpy(&sEEPROM.sLoRaDef.sLoRaWANCfg, &sCfg.sLoRaWANCfg, sizeof(tLoRaWAN_Cfg) );
  EEPROMWriteLoRaDefaultConfig(); // Write the default config to the EEPROM
  EEPROMReadLoRaDefaultConfig(); 
  memcpy(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg) );
  EEPROMWriteAppConfig();
  EEPROMReadAppConfig();
  ReWriteConfigFileData(); // Write the config to the EEPROM-Disk    
  
  if (b_restart)
  {
      // Restart to take action
      __disable_irq( );
      NVIC_SystemReset();     
  }  
} 

void CfgDeleteFactoryDefault(bool b_restart)
{
  if (b_restart)
    f_unlink("Cfg.TXT"); // Delete config File   
  
  CfgSetInitialDefault();
  memcpy(&sEEPROM.sLoRaDef.sLoRaWANCfg, &sCfg.sLoRaWANCfg, sizeof(tLoRaWAN_Cfg) );
  EEPROMWriteLoRaDefaultConfig(); // Write the default config to the EEPROM
  memcpy(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg) );
  EEPROMWriteAppConfig();
  
  if (b_restart)
  {
      // Restart to take action
      __disable_irq( );
      NVIC_SystemReset();     
  }
  else
    ReWriteConfigFileData();  
} 

void CfgInit(void)
{    
  // Init config
  CfgSetInitialDefault();    
  if (!EEPROMReadLoRaDefaultConfig())
  {
    memcpy(&sEEPROM.sLoRaDef.sLoRaWANCfg, &sCfg.sLoRaWANCfg, sizeof(tLoRaWAN_Cfg) );
    EEPROMWriteLoRaDefaultConfig(); // Write the default config to the EEPROM
    EEPROMReadLoRaDefaultConfig();        
  }
  else  
  {
    // Check the band type differences (in case of SW update to other band)
    if ( (sEEPROM.sLoRaDef.sLoRaWANCfg.ui8MinDatarate > LORAMAC_TX_MAX_DATARATE) || 
         (sEEPROM.sLoRaDef.sLoRaWANCfg.ui8MaxDatarate > LORAMAC_TX_MAX_DATARATE) || 
         (sEEPROM.sLoRaDef.sLoRaWANCfg.ui8DefaultDatarate > LORAMAC_TX_MAX_DATARATE) || 
         ((sEEPROM.sLoRaDef.sLoRaWANCfg.ui8Rx2DefaultDatarate < LORAMAC_RX_MIN_DATARATE) || (sEEPROM.sLoRaDef.sLoRaWANCfg.ui8Rx2DefaultDatarate > LORAMAC_RX_MAX_DATARATE)) )
   {
      sEEPROM.sLoRaDef.sLoRaWANCfg.ui8MinDatarate = sCfg.sLoRaWANCfg.ui8MinDatarate;
      sEEPROM.sLoRaDef.sLoRaWANCfg.ui8MaxDatarate = sCfg.sLoRaWANCfg.ui8MaxDatarate;  
      sEEPROM.sLoRaDef.sLoRaWANCfg.ui8DefaultDatarate = sCfg.sLoRaWANCfg.ui8DefaultDatarate; 
      sEEPROM.sLoRaDef.sLoRaWANCfg.ui8Rx2DefaultDatarate = sCfg.sLoRaWANCfg.ui8Rx2DefaultDatarate;       
      EEPROMWriteLoRaDefaultConfig(); // Write the changed default config to the EEPROM
    }
    memcpy(&sCfg.sLoRaWANCfg, &sEEPROM.sLoRaDef.sLoRaWANCfg, sizeof(tLoRaWAN_Cfg) );
  }
  
  if (!EEPROMReadAppConfig())
  {
    memcpy(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg) );
    EEPROMWriteAppConfig(); // Write the default config to the EEPROM
    EEPROMReadAppConfig();
  }
  else    
    memcpy(&sCfg.sAppCfg, &sEEPROM.sApp.sAppCfg, sizeof(tAppCfg) );                
}

uint16_t CfgHandleCommTelegramm( uint8_t* pui8RxBuffAddr, uint16_t ui16RxBuffLen, uint8_t* pui8TxBuffAddr, bool *pbTxAppendOk, bool *pbTxAppendParamErr, bool *pbAnswer)
{
  uint16_t ui16TxBuffLen = 0;
  bool     bParamError = false;
  
  *pbAnswer = true;
  if (!strncmp(strCom_CfgSet, (char*)pui8RxBuffAddr, sizeof(strCom_CfgSet)-1))
  {
    pui8RxBuffAddr += sizeof(strCom_CfgSet)-1;
    sCfg_tmp = sCfg;
    CfgSetFromString((char*)pui8RxBuffAddr, &bParamError);        
    // Check we got changed data
    if (memcmp(&sCfg_tmp, &sCfg, sizeof(sCfg))) 
    {
      // Check we got other App data than the EEPROM has
      if (memcmp(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg)))
      {
        memcpy(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg) );
        EEPROMWriteAppConfig();
        // ModbusDeInit( );
        // ModbusInit(sCfg.sMB.u32Baudrate, sCfg.sMB.eParity, sCfg.sMB.u8ASCII, sCfg.sMB.u16AnswerTimeout_ms, sCfg.sMB.u8Retries);
      }
      ReWriteConfigFileData();           // Write the config to the EEPROM-Disk 
    } 
    *pbTxAppendOk = true; 
  }
  else if (!strncmp(strCom_CfgGet, (char*)pui8RxBuffAddr, sizeof(strCom_CfgGet)-1))
  {
    pui8RxBuffAddr += sizeof(strCom_CfgGet)-1;
    ui16TxBuffLen = CfgGetFromString((char*)pui8RxBuffAddr, (char*)pui8TxBuffAddr, &sCfg);   
    if (!ui16TxBuffLen)    
      *pbTxAppendParamErr = true;
  }
  else if (!strncmp(strCom_CfgGetDef, (char*)pui8RxBuffAddr, sizeof(strCom_CfgGetDef)-1))
  {
    pui8RxBuffAddr += sizeof(strCom_CfgGetDef)-1;
    sCfg_tmp = sCfg;
    memcpy(&sCfg_tmp.sLoRaWANCfg, &sEEPROM.sLoRaDef.sLoRaWANCfg, sizeof(sCfg_tmp.sLoRaWANCfg));
    ui16TxBuffLen = CfgGetFromString((char*)pui8RxBuffAddr, (char*)pui8TxBuffAddr, &sCfg_tmp);
    if (!ui16TxBuffLen)    
      *pbTxAppendParamErr = true;
  }      
  else if (!strncmp(strCom_CfgEdit, (char*)pui8RxBuffAddr, sizeof(strCom_CfgEdit)-1))
  {
    pui8RxBuffAddr += sizeof(strCom_CfgEdit)-1;
    CfgSetFromString((char*)pui8RxBuffAddr,&bParamError);    
    *pbTxAppendOk = true;    
  }
  else if (!strncmp(strCom_CfgSave, (char*)pui8RxBuffAddr, sizeof(strCom_CfgSave)-1))
  {
    // Check we got other Modbus data than the EEPROM has
    if (memcmp(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg)))
    {
      memcpy(&sEEPROM.sApp.sAppCfg, &sCfg.sAppCfg, sizeof(tAppCfg) );
      EEPROMWriteAppConfig();
      // ModbusDeInit( );
      // ModbusInit(sCfg.sMB.u32Baudrate, sCfg.sMB.eParity, sCfg.sMB.u8ASCII, sCfg.sMB.u16AnswerTimeout_ms, sCfg.sMB.u8Retries);
    }
    ReWriteConfigFileData();           // Write the config to the EEPROM-Disk 
    *pbTxAppendOk = true; 
  }  
  else if (!strncmp(strCom_CfgSetAsDef, (char*)pui8RxBuffAddr, sizeof(strCom_CfgSetAsDef)-1))
  {  
    pui8RxBuffAddr += sizeof(strCom_CfgSetAsDef)-1;
    sCfg_tmp = sCfg;
    CfgSetFromString((char*)pui8RxBuffAddr,&bParamError);        
    // Check we got changed lora data
    if (memcmp(&sCfg_tmp.sLoRaWANCfg, &sCfg.sLoRaWANCfg, sizeof(sCfg.sLoRaWANCfg))) 
      CfgSaveAsFactoryDefault(false);    
    else
      sCfg = sCfg_tmp; // Copy back modbus data
 
    *pbTxAppendOk = true;   
  }
  else if (!strncmp(strCom_CfgSaveAsDef, (char*)pui8RxBuffAddr, sizeof(strCom_CfgSaveAsDef)-1))
  {  
    CfgSaveAsFactoryDefault(false); 
    *pbTxAppendOk = true; 
  } 
  else if (!strncmp(strCom_CfgDelDef, (char*)pui8RxBuffAddr, sizeof(strCom_CfgDelDef)-1))
  {   
    CfgDeleteFactoryDefault(false);
    *pbTxAppendOk = true;   
  }
  else if (!strncmp(strCom_CfgUseDef, (char*)pui8RxBuffAddr, sizeof(strCom_CfgUseDef)-1))
  {   
    CfgSetFactoryDefault();
    *pbTxAppendOk = true;  
  }
  else
    *pbAnswer = false;
  
  return ui16TxBuffLen;  
}


/*****END OF FILE****/
