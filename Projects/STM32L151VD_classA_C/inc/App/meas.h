/**============================================================================
* @file      meas.h
* @date      2016-11-10
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     initialization and definition of sirop I/Os
*            
* VERSION:   
* 
* V0.01      2016-11-10-Ra      Create File 
* V0.02      2016-12-14-Ra      New external definitions added
*
*============================================================================*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MEAS_H
#define __MEAS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/	  
#include <stdint.h>
#include <stdbool.h>   
	 
/* Exported define -----------------------------------------------------------*/   
#define EXT_SENSE_CHANNELS      8   

/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
enum BoardPowerSource {
    EXT_POWER = 0,
    BATTERY_POWER
};

typedef enum {
    eMEAS_EXTSENSETEMPTYPE_TC_First = 0,
    eMEAS_EXTSENSETEMPTYPE_PT1000_2Wire = eMEAS_EXTSENSETEMPTYPE_TC_First,
    eMEAS_EXTSENSETEMPTYPE_PT1000_3Wire, // 2 und 3 Wire funktionieren hier (macht einfach 2 Messungen)
    eMEAS_EXTSENSETEMPTYPE_PT100_2Wire,
    eMEAS_EXTSENSETEMPTYPE_PT100_3Wire,  // 2 und 3 Wire funktionieren hier (macht einfach 2 Messungen)
    eMEAS_EXTSENSETEMPTYPE_TC_E,
    eMEAS_EXTSENSETEMPTYPE_TC_J,
    eMEAS_EXTSENSETEMPTYPE_TC_K,
    eMEAS_EXTSENSETEMPTYPE_TC_N,
    eMEAS_EXTSENSETEMPTYPE_TC_R,
    eMEAS_EXTSENSETEMPTYPE_TC_S,
    eMEAS_EXTSENSETEMPTYPE_TC_T,  
    eMEAS_EXTSENSETEMPTYPE_TC_Last = eMEAS_EXTSENSETEMPTYPE_TC_T
} t_eMEASExtSenseTempType;

typedef struct{
  float fTemp_gC[EXT_SENSE_CHANNELS]; 
  float fU_V[EXT_SENSE_CHANNELS];
  float fI_mA[EXT_SENSE_CHANNELS];
  float fCorrU_mV[EXT_SENSE_CHANNELS];
  float fCorrI_uA[EXT_SENSE_CHANNELS];  
  float fImp_Ohm[EXT_SENSE_CHANNELS];         // Scheinleistung (Apparent)
  float fImpActive_Ohm[EXT_SENSE_CHANNELS];   // Wirkwiderstand
  
  // Hilfswerte
  float fImpReactive_Ohm[EXT_SENSE_CHANNELS]; // Blindwiderstand    
  float fImpU_mv[EXT_SENSE_CHANNELS];  
  float fImpI_uA[EXT_SENSE_CHANNELS];
  int8_t i8ImpPhaseShiftAngle_g[EXT_SENSE_CHANNELS]; // Phasenverschiebung +-90°

} t_sMEASExtSenseData;

/* Exported variables --------------------------------------------------------*/
extern t_sMEASExtSenseData sMEASExtSenseData;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 		 

bool MEASGetExtADCSensors( t_eMEASExtSenseTempType  aeMEASExtSenseTempType[EXT_SENSE_CHANNELS], int16_t i16ColdJunctionT_GC_100, uint32_t u32ExtPowerOnTickstart_ms, 
      uint8_t  u8EnabledTemperatureChMsk, uint8_t  u8EnabledVoltageChMsk, uint8_t  u8EnabledCurrentChMsk, uint8_t  u8EnabledCorrVoltageChMsk, uint8_t  u8EnabledCorrCurrentChMsk);

bool MEASGetIntADCSensors( uint8_t  u8EnabledChMsk );

bool MEASGetColdJunctionTemperature( int16_t * i16ColdJuncTemp_GC_100 );

/*!
 * \brief Measure the analog input UBat
 *
 * \retval value  Voltage in mV (uint16_t)
 */
uint16_t MEASGetUbat( void );

/*!
 * \brief Calculates the battery level out of UBat
 *
 * \retval Battery Level --> 1 is 2400mV 254 is 3000 mV (uint8_t)   
 */
uint8_t MEASGetBatteryLevel( void ); 

/*!
* \brief This function calculates the voltage on a channel from its ADC Value
*
* \param ADCChannel to be measured 
*
* \retval Voltage in mV on the selected channel	
*/
uint16_t MEASGetADCmV (uint32_t ADCChannel, uint16_t u16AverageCnt); 

#ifdef __cplusplus
}
#endif

#endif // __MEAS_BOARD_H

/*****END OF FILE****/
