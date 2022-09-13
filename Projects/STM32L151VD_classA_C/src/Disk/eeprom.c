#include "hw.h"
#include "bsp.h"
#include "eeprom.h"

#ifndef FLASH_EEPROM_BASE
  #define FLASH_EEPROM_BASE 0x08080000 
#endif
static bool    bEEPROM_LoRa; // CRC ok ?
static bool    bEEPROM_App;

tEEPROM sEEPROM;
tEEPROM* pEEPROMreadonly = (tEEPROM *) FLASH_EEPROM_BASE;

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

/**
  * @brief          Calulate CRC-16-CCITT
  * @param[in]      pui8Data : data for CRC calulation
  * @param[in]      ui16Len : data lenght
  * @retval         ui16Crc : CRC-16-CCITT
  */
uint16_t CrcCcitt(const uint8_t *pui8Data, uint16_t ui16Len)
{
  uint16_t ui16Crc = 0;
  uint8_t  ui8Mask = 0;

  for (; 0 < ui16Len; --ui16Len, ++pui8Data)   // Loop all bytes
  {
    for (ui8Mask=0x01; ui8Mask; ui8Mask <<=1)  // Loop all bits
    {
      if ((*pui8Data) & ui8Mask)
        ui16Crc ^= 0x001;
      if (ui16Crc & 0x0001)
        ui16Crc = (ui16Crc >> 1) ^ 0x1021;
      else
        ui16Crc = ui16Crc >> 1;
    }
  }

  return ui16Crc;  
}

//READERS
bool EEPROMReadLoRaDefaultConfig(void)
{
	uint16_t ui16Crc;

  BUILD_BUG_ON((sizeof(tEEPROM) > 512 ? 1 : 0));	
  
  memcpy(&sEEPROM.sLoRaDef, pEEPROMreadonly , sizeof(sEEPROM.sLoRaDef));  
  
  ui16Crc = CrcCcitt((uint8_t*)&sEEPROM.sLoRaDef+sizeof(sEEPROM.sLoRaDef.crc), sizeof(sEEPROM.sLoRaDef)-sizeof(sEEPROM.sLoRaDef.crc));
  if ((sEEPROM.sLoRaDef.magic_nr == 0x12345678) && (ui16Crc == sEEPROM.sLoRaDef.crc))
    bEEPROM_LoRa = true;
  else
    bEEPROM_LoRa = false;
  
  return bEEPROM_LoRa;
}

bool EEPROMReadAppConfig(void)
{
	uint16_t ui16Crc;
	uint32_t u32Addr;

  BUILD_BUG_ON(((sizeof(sEEPROM.sLoRaDef)%4) ? 1 : 0));	

	u32Addr  = sizeof(sEEPROM.sLoRaDef);
	u32Addr += FLASH_EEPROM_BASE;	
  
  memcpy(&sEEPROM.sApp, (void*)u32Addr , sizeof(sEEPROM.sApp));  
  
  ui16Crc = CrcCcitt((uint8_t*)&sEEPROM.sApp+sizeof(sEEPROM.sApp.crc), sizeof(sEEPROM.sApp)-sizeof(sEEPROM.sApp.crc));
  if ((sEEPROM.sApp.magic_nr == 0x12345678) && (ui16Crc == sEEPROM.sApp.crc))
    bEEPROM_App = true;
  else
    bEEPROM_App = false;
  
  return bEEPROM_App;
}


//WRITERS
bool EEPROMWriteLoRaDefaultConfig(void)
{
  bool bSuccess = true;
  
  uint32_t *pui32Buf = (uint32_t*)&sEEPROM.sLoRaDef;
  uint32_t ui32Addr;
  
  sEEPROM.sLoRaDef.cfg_version = 0;
  sEEPROM.sLoRaDef.magic_nr = 0x12345678;
  sEEPROM.sLoRaDef.crc = CrcCcitt((uint8_t*)&sEEPROM.sLoRaDef+sizeof(sEEPROM.sLoRaDef.crc), sizeof(sEEPROM.sLoRaDef)-sizeof(sEEPROM.sLoRaDef.crc));
  
  HAL_FLASHEx_DATAEEPROM_DisableFixedTimeProgram(); // Only Erase before Write, when EEPROM_READ != DATA      
  HAL_FLASHEx_DATAEEPROM_Unlock();
  for (ui32Addr=0;ui32Addr<sizeof(sEEPROM.sLoRaDef);ui32Addr+=4,pui32Buf++)
    if ((HAL_FLASHEx_DATAEEPROM_Program( FLASH_TYPEPROGRAMDATA_WORD, FLASH_EEPROM_BASE+ui32Addr, *pui32Buf)) != HAL_OK)  
    {   
      //here some message that it didnt work per vcom?      
      bSuccess = false;
    }
  HAL_FLASHEx_DATAEEPROM_Lock();
    
  return bSuccess;
}

bool EEPROMWriteAppConfig(void)
{
  bool bSuccess = true;
  
  uint32_t *pui32Buf = (uint32_t*)&sEEPROM.sApp;
  uint32_t ui32Addr;
  
  sEEPROM.sApp.cfg_version = 0;
  sEEPROM.sApp.magic_nr = 0x12345678;
  sEEPROM.sApp.crc = CrcCcitt((uint8_t*)&sEEPROM.sApp+sizeof(sEEPROM.sApp.crc), sizeof(sEEPROM.sApp)-sizeof(sEEPROM.sApp.crc));
  
  HAL_FLASHEx_DATAEEPROM_DisableFixedTimeProgram(); // Only Erase befor Write, when EEPROM_READ != DATA      
  HAL_FLASHEx_DATAEEPROM_Unlock();
  for (ui32Addr=0;ui32Addr<sizeof(sEEPROM.sApp);ui32Addr+=4,pui32Buf++)
    if ((HAL_FLASHEx_DATAEEPROM_Program( FLASH_TYPEPROGRAMDATA_WORD, FLASH_EEPROM_BASE+sizeof(tEEPROM_LoRa)+ui32Addr, *pui32Buf)) != HAL_OK)  
    {   
      //here some message that it didnt work per vcom?      
      bSuccess = false;
    }
  HAL_FLASHEx_DATAEEPROM_Lock();
    
  return bSuccess;
}
