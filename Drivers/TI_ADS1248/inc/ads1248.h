/**
  ******************************************************************************
  * @file    ads1248.h 
  * @author  MMY Application Team
  * @version V0.0.0
  * @date    02/06/2016
  * @brief   This file provides set of firmware functions to manage communication
	* @brief   between MCU and the ADS1248 device
  ****************************************************************************** 
  */ 

/* Define to prevent recursive inclusion ------------------------------------------------ */
#ifndef __ADS1248_H
#define __ADS1248_H

/* Includes ----------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include "ads1248_board.h"

typedef struct {
	uint8_t	OFC0;
	uint8_t	OFC1;
	uint8_t	OFC2;
} tsADS1248_ChSysOffset;  

void ADS1248_Init( void );
bool ADS1148_CalibSysOffset( t_eADS1248_Board_MeasureTyp eADS1248_MeasureTyp,  tsADS1248_ChSysOffset* psADS1248_ChSysOffset);
bool ADS1148_SetupMeasure( t_eADS1248_Board_MeasureTyp eADS1248_MeasureTyp, int16_t i16ColdJunctionT_GC_100);
bool ADS1248_Measure( int32_t* pi32ADCConvData, float* pfScaledValue, bool bScaling);
void ADS1248_DeInit( void );
#endif /* __ADS1248_H */

/*****END OF FILE****/
