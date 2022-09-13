/**
  ******************************************************************************
  * @file   eeprom.h
  * @brief  Header for disk applications
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __eeprom_H
#define __eeprom_H
#ifdef __cplusplus
 extern "C" {
#endif
   
#include "lora.h"
#include "app.h"	 
	 
typedef struct {
  uint16_t     crc;   
  uint8_t      cfg_version;
  uint32_t     magic_nr;      // Must by 0x12345678
  tLoRaWAN_Cfg sLoRaWANCfg;   // 127 Bytes  
} tEEPROM_LoRa;               // 134 Bytes

typedef struct {
  uint16_t     crc;   
  uint8_t      cfg_version;
  uint32_t     magic_nr;      // Must by 0x12345678
  tAppCfg      sAppCfg;       // x Bytes  
} tEEPROM_App;                // x Bytes

typedef struct {
  tEEPROM_LoRa   sLoRaDef;      // 134 Bytes
	tEEPROM_App    sApp;          // ca. 24 Bytes
} tEEPROM;                      // ca. 158 Bytes (MaxSize[MSC_BootSectorSize] --> 512 Bytes)

extern tEEPROM sEEPROM;

bool EEPROMReadLoRaDefaultConfig(void);
bool EEPROMReadAppConfig(void);

bool EEPROMWriteLoRaDefaultConfig(void);
bool EEPROMWriteAppConfig(void);

void EEPROMHandleNewSerialNr(bool bFile); 

#ifdef __cplusplus
}
#endif

#endif /*__eeprom_H */

/*****END OF FILE****/
