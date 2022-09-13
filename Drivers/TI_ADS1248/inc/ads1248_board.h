/**============================================================================
* @file      ads1248_board.h
* @date      2018-06-15
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     ADS1248 Board dependent functions
*            
* VERSION:   
* 
* V0.01      2018-06-15-Kd      Create File
*
*============================================================================*/

/* Define to prevent recursive inclusion ------------------------------------------------ */
#ifndef __ADS1248_BOARD_H
#define __ADS1248_BOARD_H

/* Includes ----------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>

#define ADS1248_HSPIx   hspi3

#ifndef LOW
#define LOW      (0)
#endif

#ifndef HIGH
#define HIGH      (!LOW)
#endif

typedef struct {
	uint8_t MUX0;
	uint8_t	VBIAS;
	uint8_t	MUX1;
	uint8_t	SYS0;     // Enthält PGA gain
	uint8_t	OFC0;
	uint8_t	OFC1;
	uint8_t	OFC2;
	uint8_t	FSC0;     // ACHTUNG: bei PGA-Änderung wird die entsprechende Werkskalibartion geladen !
	uint8_t	FSC1;     // ACHTUNG: bei PGA-Änderung wird die entsprechende Werkskalibartion geladen !
	uint8_t	FSC2;     // ACHTUNG: bei PGA-Änderung wird die entsprechende Werkskalibartion geladen !
	uint8_t	IDAC0;
	uint8_t	IDAC1;
	uint8_t	GPIOCFG;
	uint8_t	GPIODIR;
	uint8_t GPIODAT;
} ts_ADS1248_RegMap;  

typedef enum {
  eADS1248_MeasureTyp_PT1000_2Wire  = 0, // -200 bis +850; Measure with IOut1-Current 100uA * 5k = 0.5V + (100uA * 2k) <= ca. 0.7V (Iout muss < 1.8V sein)
  eADS1248_MeasureTyp_PT1000_3Wire,      // -200 bis +850; First Measure with IOut1-Current 100uA + AIN2 100uA * 5k = 1V + (100uA * 2k) = ca. 1.2V (2. Messung mit IDAC1 + IDAC2 an den Stromausgängen tauschen und Messungen mitteln)
  eADS1248_MeasureTyp_PT1000_3Wire_Inv,  //                Second measure with invertet IOuts
  eADS1248_MeasureTyp_PT100_2Wire,       // -200 bis +850; Measure with IOut1-Current 1mA * 0.5k = 0.5V + (1mA * 700 Ohm) <= ca. 1.2V (Iout muss < 1.8V sein)
  eADS1248_MeasureTyp_PT100_3Wire,       // -200 bis +850; First Measure with IOut1-Current 500uA + AIN2 500uA * 0.5k = 0.5V + (1mA * 700 Ohm) <= ca. 1.2V (2. Messung mit IDAC1 + IDAC2 an den Stromausgängen tauschen und Messungen mitteln)
  eADS1248_MeasureTyp_PT100_3Wire_Inv,   //                Second measure with invertet IOuts
  eADS1248_MeasureTyp_PT_Discharge_AIN,
  eADS1248_MeasureTyp_ThermoCouple_E,    // -270 bis +1000;Bias an AIN2 ausgeben (keine Ströme ausgeben) und eine hoher GAIN verwenden
  eADS1248_MeasureTyp_ThermoCouple_J,    // -210 bis +1200;"
  eADS1248_MeasureTyp_ThermoCouple_K,    // -270 bis +1300;"
  eADS1248_MeasureTyp_ThermoCouple_N,    // -270 bis +1300;"
  eADS1248_MeasureTyp_ThermoCouple_R,    // -50 bis +1768; "
  eADS1248_MeasureTyp_ThermoCouple_S,    // -50 bis +1768; "
  eADS1248_MeasureTyp_ThermoCouple_T,    // -270 bis +400; "    
  eADS1248_MeasureTyp_U,                 // 0..48.15V Gain 1
  eADS1248_MeasureTyp_I,
  eADS1248_MeasureTyp_Corr_U,
  eADS1248_MeasureTyp_Corr_I,
  eADS1248_MeasureTyp_U_HighRes          // 2019-01-07-Kd new V1.03 0..6.5V Gain 4
} t_eADS1248_Board_MeasureTyp;

//void  ADS1248_Board_TEST_PIN_SET( uint8_t u8Mode );
void    ADS1248_Board_RESET( uint8_t u8Mode );
void    ADS1248_Board_START( uint8_t u8Mode );
uint8_t ADS1248_Board_DATA_READY_STATE_GET( void );
void    ADS1248_Board_SPI_CS( uint8_t u8Mode );

void    ADS1248_Board_IO_Init( void);
void    ADS1248_Board_IO_DeInit( void);

float   ADS1248_Board_Scale( int32_t i32ADCConvData, t_eADS1248_Board_MeasureTyp eADS1248_MeasureTyp, int16_t i16ColdJunctionT_GC_100);
void    ADS1248_Board_REGISTER_PrepareAll( t_eADS1248_Board_MeasureTyp eADS1248_MeasureTyp);
#endif /* __ADS1248_BOARD_H */

/*****END OF FILE****/
