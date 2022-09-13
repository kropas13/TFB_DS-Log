/**
  ******************************************************************************
  * @file   config.h
  * @brief  Header for configuration
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __config_H
#define __config_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>      
#include "stm32l1xx.h"
#include "lora.h"
#include "app.h"   

#ifdef DEF_USE_IMST_STUDIO
  #define NON_LORAWAN_COMPLIANT_DEVICE    1 // Falls nicht LoRaWAN compliant (EUIs big endian) -> auf 1 setzen
#else
  #define NON_LORAWAN_COMPLIANT_DEVICE    0 // Falls LoRaWAN compliant (EUIs little endian) -> auf 0 setzen
#endif
    
//#define C_SENDONCHANGE_NONE         0
//#define C_SENDONCHANGE_BOTH_EDGES   1
//#define C_SENDONCHANGE_RISING_EDGE  2
//#define C_SENDONCHANGE_FALLING_EDGE 3
    
// Config Strings
//LoRa Parameters
extern const char strLoRaWANcfg_PrivateNetwork[];
extern const char strLoRaWANcfg_ADR[]; 
extern const char strLoRaWANcfg_OTAA[];
extern const char strLoRaWANcfg_DevEUI[];
extern const char strLoRaWANcfg_AppEUI[];
extern const char strLoRaWANcfg_AppKey[];
extern const char strLoRaWANcfg_NetwID[];
extern const char strLoRaWANcfg_DevAddr[];
extern const char strLoRaWANcfg_NetwSesKey[];
extern const char strLoRaWANcfg_AppSesKey[];

extern const char strLoRaWANcfg_BC_Addr[];
extern const char strLoRaWANcfg_BC_NetwSesKey[];
extern const char strLoRaWANcfg_BC_AppSesKey[];

extern const char strLoRaWANcfg_MinDataRate[];
extern const char strLoRaWANcfg_MaxDataRate[];
extern const char strLoRaWANcfg_DefaultDataRate[];
extern const char strLoRaWANcfg_Rx2DefaultDataRate[];

//App Parameters
extern const char strAppCfg_SendInterval[];
extern const char strAppCfg_ExtSensorsStartupTime[];
extern const char strAppCfg_Ch_[];

typedef struct{
  tLoRaWAN_Cfg sLoRaWANCfg;  // 132 Bytes
  tAppCfg      sAppCfg;      // xBytes
} tCfg; 

extern tCfg sCfg; 

extern tCfg sCfg; 
extern char MyStr[200];

extern int32_t  CfgGetDecValueMinMax(char *pStr, int32_t i32MinVal, int32_t i32MaxVal, bool *pbParamError);
//extern uint32_t GetUnsignedDecValueMinMax(char *pStr, uint32_t u32MinVal, uint32_t u32MaxVal, bool *pbParamError));
extern char*    CfgMakeHexStr(char* str, uint8_t ary[], uint8_t cnt , uint8_t nonCompliantDevice);
extern void     CfgSetFromString(char* pMyStr, bool* pbParamError);
extern void     CfgSetInitialDefault(void);
extern void     CfgSaveAppConfig(void);
extern void     CfgSaveAsFactoryDefault(bool b_restart);
extern void     CfgDeleteFactoryDefault(bool b_restart);
extern void     CfgInit(void);

extern uint16_t CfgHandleCommTelegramm( uint8_t* pui8RxBuffAddr, uint16_t ui16RxBuffLen, uint8_t* pui8TxBuffAddr, bool *pbTxAppendOk, bool *pbTxAppendParamErr, bool *pbAnswer);

#ifdef __cplusplus
}
#endif

#endif /*__config_H */

/*****END OF FILE****/
