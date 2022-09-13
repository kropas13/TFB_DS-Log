/**============================================================================
* @file      Si705x.c
* @date      2018-06-14
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     * Driver for Si705x Sensor from silicon silabs
*            
* VERSION:   
* 
* V0.01      2018-06-14-Kd      Create File			
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "Si705x.h"

/* Private typedef -----------------------------------------------------------*/
//  Adress
#define I2C_ADR_Si705x  (0x40 << 1)   //Sensor address
//  CMD
#define Si705x_CMD_MEASTEMP_HOLDMODE      0xE3
#define Si705x_CMD_MEASTEMP_NOHOLDMODE    0xF3
#define Si705x_CMD_RESET                  0xFE
#define Si705x_CMD_WRITE_USERREG1         0xE6
#define Si705x_CMD_READ_USERREG1          0xE7
#define Si705x_CMD_READ_ID_BYTE1          0xFA0F
#define Si705x_CMD_READ_ID_BYTE2          0xFCC9
#define Si705x_CMD_FRIMWARE_REV           0x84B8

/* Private define ------------------------------------------------------------*/
//  CRC
static const uint16_t POLYNOMIAL  = 0x131; //P(x)=x^8+x^5+x^4+1 = 100110001

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t u8I2CAddr = I2C_ADR_Si705x; // I2C Address

/* Private functions ---------------------------------------------------------*/
/*!
 * \brief  calculates CRC
 *
 * \param  *data         : Pointer to data 
 *         nbrOfBytes    : number of bytes to do CRC with
 *	
 * \retval crc           : crc of data
 *
 */
static uint8_t Si705x_CalcCrc(uint8_t data[], uint8_t nbrOfBytes)
{
  uint8_t bit;        // bit mask
  uint8_t crc = 0x00; // calculated checksum
  uint8_t byteCtr;    // byte counter
  
  // calculates 8-Bit checksum with given polynomial
  for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
  {
    crc ^= (data[byteCtr]);
    for(bit = 8; bit > 0; --bit)
    {
      if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
      else           crc = (crc << 1);
    }
  }
  
  return crc;
}

/*!
 * \brief  compares CRC read from chip and calculated CRC
 *
 * \param  *data         : Pointer to data 
 *         nbrOfBytes    : number of bytes to do CRC with
 *         checksum      : CRC read from chip
 *	
 * \retval bool          : false -> checksum not ok;  true -> checksum ok
 *
 */
static bool Si705x_CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
{
  uint8_t crc;     // calculated checksum
  
  // calculates 8-Bit checksum
  crc = Si705x_CalcCrc(data, nbrOfBytes);
  
  // verify checksum
  if(crc != checksum) return false;
  else                return true;
}

/*!
 * \brief  calculates temperature in °C with raw value
 *
 * \param  rawValue    : temperature raw value 
 *	
 * \retval temperature : temperature in float 
 *
 */
static float Si705x_CalcTemperature(uint16_t rawValue)
{
  // calculate temperature [°C]
  // T = 175.72 * rawValue / 65536 - 46.85
  return ((175.72f * (float)rawValue) / 65536.0f) - 46.85f;
}

/* Exported functions ---------------------------------------------------------*/

/*!
 * \brief  gets temperature in °C  ±0.4 °C (max) for the Si7054
 *
 * \param  *temperature  : Pointer to temperature 
 *	
 * \retval result        : false -> error;  true -> ok
 *
 */
bool Si705x_GetTemp(float* fTemperature)
{
  bool      bOk = false;
  uint16_t  rawValueTemp;     // temperature raw value from sensor
  uint8_t   data[3] = {0};  //data array for checksum verification  
  
  if (HAL_I2C_Mem_Read(&hi2c1, u8I2CAddr, Si705x_CMD_MEASTEMP_HOLDMODE, I2C_MEMADD_SIZE_8BIT, data, sizeof(data), 100) == HAL_OK) {
    if (Si705x_CheckCrc(&data[0], 2, data[2])) {
      rawValueTemp = ((uint16_t)data[0] << 8) | data[1];
      *fTemperature = Si705x_CalcTemperature(rawValueTemp);
      bOk = true;
    }
  } 
  
  return bOk;
}

/*****END OF FILE****/
