/**
  ******************************************************************************
  * @file   disk.h
  * @brief  Header for disk applications
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __disk_H
#define __disk_H
#ifdef __cplusplus
 extern "C" {
#endif
   
#include "stm32l1xx_hal.h"

   
void   ZeroDisk(void);
int8_t WriteDisk(const uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t ReadDisk (uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);

#ifdef __cplusplus
}
#endif
#endif /*__disk_H */

/*****END OF FILE****/
