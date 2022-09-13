/**============================================================================
* @file      ads1248_board.c
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

/* Includes ------------------------------------------------------------------------------ */
#include <math.h>

#include "hw.h"
#include "ads1248_board.h"
#include "RTD.h"
#include "Thermocouple.h"

/* Private define --------------------------------------------------------------*/
#define ADS1248_INTREF_mV           2048 // +-10mV Tolerance over -40..+105°C

/* ADS1248 Register Map Definitions */
#define ADS1248_REG_0_MUX0   				((uint8_t)0x00)
#define ADS1248_REG_1_VBIAS     		((uint8_t)0x01)
#define ADS1248_REG_2_MUX1	     		((uint8_t)0x02)
#define ADS1248_REG_3_SYS0	    		((uint8_t)0x03)
#define ADS1248_REG_4_OFC0	    		((uint8_t)0x04)
#define ADS1248_REG_5_OFC1	    		((uint8_t)0x05)
#define ADS1248_REG_6_OFC2	    		((uint8_t)0x06)
#define ADS1248_REG_7_FSC0	    		((uint8_t)0x07)
#define ADS1248_REG_8_FSC1	    		((uint8_t)0x08)
#define ADS1248_REG_9_FSC2	    		((uint8_t)0x09)
#define ADS1248_REG_10_IDAC0	   		((uint8_t)0x0A)
#define ADS1248_REG_11_IDAC1	   		((uint8_t)0x0B)
#define ADS1248_REG_12_GPIOCFG    	((uint8_t)0x0C)
#define ADS1248_REG_13_GPIODIR    	((uint8_t)0x0D)
#define ADS1248_REG_14_GPIODAT    	((uint8_t)0x0E)

/* ADS1248 Register 0 - Multiplexer Control Register 0 (MUX0) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//        BCS[1:0]       |             MUX_SP[2:0]           |            MUX_SN[2:0]
//
// Define BCS (burnout current source)
#define ADS1248_MUX0_BCS_OFF			((uint8_t)0x00)	// default
#define ADS1248_MUX0_BCS_500nA		((uint8_t)0x40)
#define ADS1248_MUX0_BCS_2uA			((uint8_t)0x80)
#define ADS1248_MUX0_BCS_10uA			((uint8_t)0xC0)
// Define Positive MUX Input Channels
#define ADS1248_MUX0_SP_AIN0			((uint8_t)0x00)
#define ADS1248_MUX0_SP_AIN1			((uint8_t)0x08)
#define ADS1248_MUX0_SP_AIN2			((uint8_t)0x10)
#define ADS1248_MUX0_SP_AIN3			((uint8_t)0x18)
#define ADS1248_MUX0_SP_AIN4			((uint8_t)0x20)
#define ADS1248_MUX0_SP_AIN5			((uint8_t)0x28)
#define ADS1248_MUX0_SP_AIN6			((uint8_t)0x30)
#define ADS1248_MUX0_SP_AIN7			((uint8_t)0x38)
// Define Negative Mux Input Channels
#define ADS1248_MUX0_SN_AIN0			((uint8_t)0x00)
#define ADS1248_MUX0_SN_AIN1			((uint8_t)0x01)
#define ADS1248_MUX0_SN_AIN2			((uint8_t)0x02)
#define ADS1248_MUX0_SN_AIN3			((uint8_t)0x03)
#define ADS1248_MUX0_SN_AIN4			((uint8_t)0x04)
#define ADS1248_MUX0_SN_AIN5			((uint8_t)0x05)
#define ADS1248_MUX0_SN_AIN6			((uint8_t)0x06)
#define ADS1248_MUX0_SN_AIN7			((uint8_t)0x07)

/* ADS1248 Register 1 - Bias Voltage Register (VBIAS) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                                         VBIAS[7:0]
//
#define ADS1248_VBIAS_OFF					((uint8_t)0x00)
#define ADS1248_VBIAS_AIN0				((uint8_t)0x01)
#define ADS1248_VBIAS_AIN1				((uint8_t)0x02)
#define ADS1248_VBIAS_AIN2				((uint8_t)0x04)
#define ADS1248_VBIAS_AIN3				((uint8_t)0x08)
#define ADS1248_VBIAS_AIN4				((uint8_t)0x10)
#define ADS1248_VBIAS_AIN5				((uint8_t)0x20)
#define ADS1248_VBIAS_AIN6				((uint8_t)0x40)
#define ADS1248_VBIAS_AIN7				((uint8_t)0x80)


/* ADS1248 Register 2 (MUX1) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//  CLKSTAT  |       VREFCON[1:0]    |      REFSELT[1:0]     |            MUXCAL[2:0]
//
// Degine clock source
#define ADS1248_MUX1_CLK_INT					((uint8_t)0x00)
#define ADS1248_MUX1_CLK_EXT					((uint8_t)0x01)
// Define Internal Reference
#define ADS1248_MUX1_INT_VREF_OFF			((uint8_t)0x00)
#define ADS1248_MUX1_INT_VREF_ON			((uint8_t)0x20)
#define ADS1248_MUX1_INT_VREF_CONV		((uint8_t)0x40)
// Define Reference Select
#define ADS1248_MUX1_REF0							((uint8_t)0x00)
#define ADS1248_MUX1_REF1							((uint8_t)0x08)
#define ADS1248_MUX1_INT							((uint8_t)0x10)
#define ADS1248_MUX1_INT_REF0					((uint8_t)0x18)
// Define System Monitor
#define ADS1248_MUX1_MEAS_NORM				((uint8_t)0x00)
#define ADS1248_MUX1_MEAS_OFFSET			((uint8_t)0x01)
#define ADS1248_MUX1_MEAS_GAIN				((uint8_t)0x02)
#define ADS1248_MUX1_MEAS_TEMP				((uint8_t)0x03)
#define ADS1248_MUX1_MEAS_REF1				((uint8_t)0x04)
#define ADS1248_MUX1_MEAS_REF0				((uint8_t)0x05)
#define ADS1248_MUX1_MEAS_AVDD				((uint8_t)0x06)
#define ADS1248_MUX1_MEAS_DVDD				((uint8_t)0x07)

/* ADS1248 Register 3 (SYS0) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//     0     |              PGA[2:0]             |                   DOR[3:0]
//
// Define Gain
#define ADS1248_SYS0_GAIN_1			((uint8_t)0x00)
#define ADS1248_SYS0_GAIN_2			((uint8_t)0x10)
#define ADS1248_SYS0_GAIN_4			((uint8_t)0x20)
#define ADS1248_SYS0_GAIN_8			((uint8_t)0x30)
#define ADS1248_SYS0_GAIN_16		((uint8_t)0x40)
#define ADS1248_SYS0_GAIN_32		((uint8_t)0x50)
#define ADS1248_SYS0_GAIN_64		((uint8_t)0x60)
#define ADS1248_SYS0_GAIN_128		((uint8_t)0x70)
//Define data rate
#define ADS1248_SYS0_DR_5				((uint8_t)0x00)
#define ADS1248_SYS0_DR_10			((uint8_t)0x01)
#define ADS1248_SYS0_DR_20			((uint8_t)0x02)
#define ADS1248_SYS0_DR_40			((uint8_t)0x03)
#define ADS1248_SYS0_DR_80			((uint8_t)0x04)
#define ADS1248_SYS0_DR_160			((uint8_t)0x05)
#define ADS1248_SYS0_DR_320			((uint8_t)0x06)
#define ADS1248_SYS0_DR_640			((uint8_t)0x07)
#define ADS1248_SYS0_DR_1000		((uint8_t)0x08)
#define ADS1248_SYS0_DR_2000		((uint8_t)0x09)

/* ADS1248 Register 4 (OFC0) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                                         OFC0[7:0]
//

/* ADS1248 Register 5 (OFC1) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                                         OFC1[7:0]
//

/* ADS1248 Register 6 (OFC2) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                                         OFC2[7:0]
//

/* ADS1248 Register 7 (FSC0) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                                         FSC0[7:0]
//

/* ADS1248 Register 8 (FSC1) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                                         FSC1[7:0]
//

/* ADS1248 Register 9 (FSC2) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                                         FSC2[7:0]
//

/* ADS1248 Register 10 - IDAC Control Register 0 (IDAC0) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                     ID[3:0]                   | DRDY_MODE |              IMAG[2:0]
//
// Define DRDY mode on DOUT
// 0 = DOUT/#DRDY pin functions only as Data Out (default)
// 1 = DOUT/#DRDY pin functions both as Data Out and Data Ready, active low

#define ADS1248_IDAC0_DRDY_OFF			((uint8_t)0x00)
#define ADS1248_IDAC0_DRDY_ON				((uint8_t)0x08)
//Define IDAC Magnitude
#define ADS1248_IDAC0_IMAG_OFF			((uint8_t)0x00)		// default
#define ADS1248_IDAC0_IMAG_50				((uint8_t)0x01)		// 50uA
#define ADS1248_IDAC0_IMAG_100			((uint8_t)0x02)		// 100uA	
#define ADS1248_IDAC0_IMAG_250			((uint8_t)0x03)		// 250uA
#define ADS1248_IDAC0_IMAG_500			((uint8_t)0x04)		// 500uA
#define ADS1248_IDAC0_IMAG_750			((uint8_t)0x05)		// 750uA
#define ADS1248_IDAC0_IMAG_1000			((uint8_t)0x06)		// 1000uA
#define ADS1248_IDAC0_IMAG_1500			((uint8_t)0x07)		// 1500uA

/* ADS1248 Register 11 - IDAC Control Register (IDAC1) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                   I1DIR[3:0]                  |                   I2DIR[3:0]
//
// Define IDAC1 Output
#define ADS1248_IDAC1_I1DIR_AIN0		((uint8_t)0x00)
#define ADS1248_IDAC1_I1DIR_AIN1		((uint8_t)0x10)
#define ADS1248_IDAC1_I1DIR_AIN2		((uint8_t)0x20)
#define ADS1248_IDAC1_I1DIR_AIN3		((uint8_t)0x30)
#define ADS1248_IDAC1_I1DIR_AIN4		((uint8_t)0x40)
#define ADS1248_IDAC1_I1DIR_AIN5		((uint8_t)0x50)
#define ADS1248_IDAC1_I1DIR_AIN6		((uint8_t)0x60)
#define ADS1248_IDAC1_I1DIR_AIN7		((uint8_t)0x70)
#define ADS1248_IDAC1_I1DIR_EXT1		((uint8_t)0x80)
#define ADS1248_IDAC1_I1DIR_EXT2		((uint8_t)0x90)
#define ADS1248_IDAC1_I1DIR_OFF			((uint8_t)0xF0)
// Define IDAC2 Output
#define ADS1248_IDAC1_I2DIR_AIN0		((uint8_t)0x00)
#define ADS1248_IDAC1_I2DIR_AIN1		((uint8_t)0x01)
#define ADS1248_IDAC1_I2DIR_AIN2		((uint8_t)0x02)
#define ADS1248_IDAC1_I2DIR_AIN3		((uint8_t)0x03)
#define ADS1248_IDAC1_I2DIR_AIN4		((uint8_t)0x04)
#define ADS1248_IDAC1_I2DIR_AIN5		((uint8_t)0x05)
#define ADS1248_IDAC1_I2DIR_AIN6		((uint8_t)0x06)
#define ADS1248_IDAC1_I2DIR_AIN7		((uint8_t)0x07)
#define ADS1248_IDAC1_I2DIR_EXT1		((uint8_t)0x08)
#define ADS1248_IDAC1_I2DIR_EXT2		((uint8_t)0x09)
#define ADS1248_IDAC1_I2DIR_OFF			((uint8_t)0x0F)

/* ADS1248 Register 12 - GPIO Configuration Register (GPIOCFG) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                                         IOCFG[7:0]
//
// 0 = the pin is used as an analog input (default)
// 1 = the pin is used as a GPIO pin

#define ADS1248_GPIOCFG_IOCFG_NO_GPIO   ((uint8_t)0x00)
#define ADS1248_GPIOCFG_IOCFG_GPIO0			((uint8_t)0x01)  // GPIO0 shared with REFP0
#define ADS1248_GPIOCFG_IOCFG_GPIO1			((uint8_t)0x02)	 // GPIO1 shared with REFN0
#define ADS1248_GPIOCFG_IOCFG_GPIO2			((uint8_t)0x04)  // GPIO2 shared with AIN2
#define ADS1248_GPIOCFG_IOCFG_GPIO3			((uint8_t)0x08)  // GPIO3 shared with AIN3
#define ADS1248_GPIOCFG_IOCFG_GPIO4			((uint8_t)0x10)  // GPIO4 shared with AIN4
#define ADS1248_GPIOCFG_IOCFG_GPIO5			((uint8_t)0x20)  // GPIO5 shared with AIN5
#define ADS1248_GPIOCFG_IOCFG_GPIO6			((uint8_t)0x40)  // GPIO6 shared with AIN6
#define ADS1248_GPIOCFG_IOCFG_GPIO7			((uint8_t)0x80)  // GPIO7 shared with AIN7

/* ADS1248 Register 13 - GPIO Direction Register (GPIODIR) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                                         IODIR[7:0]
//
// 0 = the pin is an output (default)
// 1 = the pin is an input 

#define AFS1248_GPIODIR_ALL_OUT			((uint8_t)0x00) // all pin as output
#define AFS1248_GPIODIR_ALL_IN			((uint8_t)0xFF) // all pin as input
#define ADS1248_GPIODIR_IODIR0			((uint8_t)0x01)	// GPIO0
#define ADS1248_GPIODIR_IODIR1			((uint8_t)0x02)	// GPIO1
#define ADS1248_GPIODIR_IODIR2			((uint8_t)0x04)	// GPIO2
#define ADS1248_GPIODIR_IODIR3			((uint8_t)0x08)	// GPIO3
#define ADS1248_GPIODIR_IODIR4			((uint8_t)0x10)	// GPIO4
#define ADS1248_GPIODIR_IODIR5			((uint8_t)0x20)	// GPIO5
#define ADS1248_GPIODIR_IODIR6			((uint8_t)0x40)	// GPIO6
#define ADS1248_GPIODIR_IODIR7			((uint8_t)0x80)	// GPIO7

/* ADS1248 Register 14 - GPIO Data Register (GPIODAT) Definition */
//   Bit 7   |   Bit 6   |   Bit 5   |   Bit 4   |   Bit 3   |   Bit 2   |   Bit 1   |   Bit 0
//--------------------------------------------------------------------------------------------
//                                         IOIDAT[7:0]
//
#define ADS1248_GPIODAT_ALL_OFF					((uint8_t)0x00)	// ALL
#define ADS1248_GPIODAT_IODAT_GPIO0			((uint8_t)0x01)	// GPIO0
#define ADS1248_GPIODAT_IODAT_GPIO1			((uint8_t)0x02)	// GPIO1
#define ADS1248_GPIODAT_IODAT_GPIO2			((uint8_t)0x04)	// GPIO2
#define ADS1248_GPIODAT_IODAT_GPIO3			((uint8_t)0x08)	// GPIO3
#define ADS1248_GPIODAT_IODAT_GPIO4			((uint8_t)0x10)	// GPIO4
#define ADS1248_GPIODAT_IODAT_GPIO5			((uint8_t)0x20)	// GPIO5
#define ADS1248_GPIODAT_IODAT_GPIO6			((uint8_t)0x40)	// GPIO6
#define ADS1248_GPIODAT_IODAT_GPIO7			((uint8_t)0x80)	// GPIO7

/* Private macro ---------------------------------------------------------------*/
const double PT_A  = 3.9083E-3;
const double PT_B  = -5.775E-7;
//const double PT_C  = -4.183E-12;
const double PT100_R0  = 100.0;          // Widerstand bei 0°C
const double PT1000_R0 = 1000.0;         // Widerstand bei 0°C

/* Private typedef -------------------------------------------------------------*/

/* Private variables -----------------------------------------------------------*/
extern ts_ADS1248_RegMap sADS_Config_Write;
extern ts_ADS1248_RegMap sADS_Config_Read;

/* IRQ callback functions ------------------------------------------------------*/

/* Private function ------------------------------------------------------------*/

//*****************************************************************************************
//  Funktionsname   : LinearizePT1000
//  Aufgabe         : Berechnet Temperatur aufgrund des gemessenen Widerstanders eines PT1000
//                    Sehr genau im Bereich 0..850°C (-200..0°C benötigt eigentlich noch den PT_C Koeffizient) 
//  Input Parameter : Widerstandswert
//  Output Parameter: Temperatur in °C	
//  Laufzeit        : 
//
// Datum-Visum  Vers 	Modifikation
//-----------------------------------------------------------------------------------------
// 2009-12-02-Ph V0.00	Erstes Funktionsmuster
//
//*****************************************************************************************
/*
float LinearizePT1000(float R)
{
  float T=0;

  // Formel ist se
  T = ((-PT_A*PT1000_R0)+(sqrt(((PT_A*PT1000_R0)*(PT_A*PT1000_R0)) - (4*PT_B*PT1000_R0*(PT1000_R0-R)))))/(2*PT_B*PT1000_R0);

  return (T);
} 
*/

//*****************************************************************************************
//  Funktionsname   : LinearizePT100
//  Aufgabe         : Berechnet Temperatur aufgrund des gemessenen Widerstanders eines PT100
//                    Sehr genau im Bereich 0..850°C (-200..0°C benötigt eigentlich noch den PT_C Koeffizient) 
//  Input Parameter : Widerstandswert
//  Output Parameter: Temperatur in °C	
//  Laufzeit        : 
//
// Datum-Visum  Vers 	Modifikation
//-----------------------------------------------------------------------------------------
// 2009-12-02-Ph V0.00	Erstes Funktionsmuster
//
//*****************************************************************************************
/*
float LinearizePT100(float R)
{
  float T=0;

  T = ((-PT_A*PT100_R0)+(sqrt(((PT_A*PT100_R0)*(PT_A*PT100_R0)) - (4*PT_B*PT100_R0*(PT100_R0-R)))))/(2*PT_B*PT100_R0);

  return (T);
} 
*/


/* Exported functions ----------------------------------------------------------*/

void ADS1248_Board_IO_Init( void)
{
  GPIO_InitTypeDef initStruct={0};
  
  initStruct.Mode   = GPIO_MODE_INPUT;
  initStruct.Speed  = GPIO_SPEED_MEDIUM;  
  initStruct.Pull   = GPIO_PULLUP;
  
  // Inputs
  HW_GPIO_Init(_ADC_DATA_READY_DI_GPIO_Port, _ADC_DATA_READY_DI_Pin, &initStruct); 
  
  // Outputs  
  initStruct.Pull   = GPIO_NOPULL;           
  initStruct.Mode   = GPIO_MODE_OUTPUT_PP;    
  HAL_GPIO_WritePin(ADC_START_DO_GPIO_Port, ADC_START_DO_Pin, GPIO_PIN_SET);
  HW_GPIO_Init(ADC_START_DO_GPIO_Port, ADC_START_DO_Pin, &initStruct); 
  HAL_GPIO_WritePin(_ADC_RESET_DO_GPIO_Port, _ADC_RESET_DO_Pin, GPIO_PIN_SET);
  HW_GPIO_Init(_ADC_RESET_DO_GPIO_Port, _ADC_RESET_DO_Pin, &initStruct);  

  // SPI
  HW_SPI3_Init( );  
}

void ADS1248_Board_IO_DeInit( void)
{
  GPIO_InitTypeDef initStruct={0};
  
  HW_SPI3_DeInit( );
  
  initStruct.Mode   = GPIO_MODE_ANALOG;
  initStruct.Pull   = GPIO_NOPULL;  
  
  HW_GPIO_Init(_ADC_DATA_READY_DI_GPIO_Port, _ADC_DATA_READY_DI_Pin, &initStruct);    
  HW_GPIO_Init(ADC_START_DO_GPIO_Port, ADC_START_DO_Pin, &initStruct); 
  HW_GPIO_Init(_ADC_RESET_DO_GPIO_Port, _ADC_RESET_DO_Pin, &initStruct);    
}

float ADS1248_Board_Scale( int32_t i32ADCConvData, t_eADS1248_Board_MeasureTyp eADS1248_MeasureTyp, int16_t i16ColdJunctionT_GC_100)
{
  float           fScaledValue;
  uint32_t        ui32Res_mOhm;
  uint32_t        ui32ResScal_mOhm = 2500000L;
  int32_t         i32Temp_10mgC;
  int32_t         i32U_uV;
  teRTD           eRTDTyp = RTD_PT1000;
  teTHERMOCOUPLE  eTCTyp;
 
  // PT1000 (FS = +-5kOhmRbias/Gain4 = +-2.5kOhm = ADC +-0x7FFFFF)
  switch (eADS1248_MeasureTyp)
  {
    case eADS1248_MeasureTyp_PT100_2Wire: // PT100 (FS = +-500 OhmRbias/Gain2 = +-250 Ohm = ADC +-0x7FFFFF)
    case eADS1248_MeasureTyp_PT100_3Wire:       // PT100 (FS = 2*+-500 OhmRbias/Gain4 = +-250 Ohm = ADC +-0x7FFFFF)
    case eADS1248_MeasureTyp_PT100_3Wire_Inv:   // PT100 (FS = 2*+-500 OhmRbias/Gain4 = +-250 Ohm = ADC +-0x7FFFFF)
       eRTDTyp = RTD_PT100; // No break !
       ui32ResScal_mOhm = 250000L;
    case eADS1248_MeasureTyp_PT1000_2Wire: // PT1000 (FS = +-5kOhmRbias/Gain2 = +-2.5kOhm = ADC +-0x7FFFFF)
    case eADS1248_MeasureTyp_PT1000_3Wire:      // PT1000 (FS = 2*+-5kOhmRbias/Gain4 = +-2.5kOhm = ADC +-0x7FFFFF)
    case eADS1248_MeasureTyp_PT1000_3Wire_Inv:  // PT1000 (FS = 2*+-5kOhmRbias/Gain4 = +-2.5kOhm = ADC +-0x7FFFFF)    
        ui32Res_mOhm = (uint32_t)((int64_t)i32ADCConvData * ui32ResScal_mOhm / 0x7FFFFFL); // [mOhm]
        i32Temp_10mgC = RTD_GetTemperature(ui32Res_mOhm, eRTDTyp);
        fScaledValue = (float)i32Temp_10mgC / 100;
        if (fScaledValue > 250.0)
          fScaledValue = 250.0;
        else if (fScaledValue < -50.0)
          fScaledValue = -50.0;    
      break;               
    case eADS1248_MeasureTyp_ThermoCouple_E: // PGA GAIN 32 ±64mV
    case eADS1248_MeasureTyp_ThermoCouple_J:
    case eADS1248_MeasureTyp_ThermoCouple_K:
    case eADS1248_MeasureTyp_ThermoCouple_N:
    case eADS1248_MeasureTyp_ThermoCouple_R:
    case eADS1248_MeasureTyp_ThermoCouple_S:
    case eADS1248_MeasureTyp_ThermoCouple_T:         
      eTCTyp = (teTHERMOCOUPLE)((eADS1248_MeasureTyp - eADS1248_MeasureTyp_ThermoCouple_E) + THERMOCOUPLE_E);
      if (eTCTyp >= THERMOCOUPLE_L)
        eTCTyp -= 1;                 
      i32U_uV = (uint32_t)((int64_t)i32ADCConvData * 64000L / 0x7FFFFFL); // [uV]      
      i32U_uV += Thermocouple_GetVoltage(i16ColdJunctionT_GC_100, eTCTyp);  // Add cold junction compensation voltage 2018-11-20-Kd neu ab V1.02      
      i32Temp_10mgC = Thermocouple_GetTemperature(i32U_uV, eTCTyp);
      fScaledValue = (float)i32Temp_10mgC / 100;      
      if (fScaledValue > 250.0)
        fScaledValue = 250.0;
      else if (fScaledValue < -50.0)
        fScaledValue = -50.0;      
      break;
    case eADS1248_MeasureTyp_U: // R-Teiler 23.5:1 (0..45V) -> 0..1914mV; 1Bit = 2.048V/(2hoch24) = 0.12207uV (max. +-48.15V)
      fScaledValue = (float)i32ADCConvData * (23.5*ADS1248_INTREF_mV/1000) / 0x7FFFFFL; // [V]
      if (fScaledValue > 48.0)
        fScaledValue = 48.0;
      else if (fScaledValue < 0)
        fScaledValue = 0;      
      break;
    case eADS1248_MeasureTyp_U_HighRes: // 2018-12-07-Kd V1.03 R-Teiler 23.5:1 (0..6.5V) -> 0..1914mV; 1Bit = 512mV/(2hoch24) = 0.0305175uV (max. +-12.0375)
      fScaledValue = (float)i32ADCConvData * (23.5*ADS1248_INTREF_mV/4000) / 0x7FFFFFL; // [V]
      if (fScaledValue > 6.5)
        fScaledValue = 6.5;
      else if (fScaledValue < 0.0)
        fScaledValue = 0;      
      break;
      
    case eADS1248_MeasureTyp_I: // Rshunt 1 Ohm (0..100mA) * 19 -> 0..1900mV; 1Bit = 2.048V/(2hoch24) = 0.12207uV (max. 107.8mA)
      fScaledValue = (float)i32ADCConvData * (100.0 * ADS1248_INTREF_mV/1900.0) / 0x7FFFFFL; // [mA]
      if (fScaledValue > 105.0)
        fScaledValue = 105.0;
      else if (fScaledValue < 0)
        fScaledValue = 0;              
      break;
    case eADS1248_MeasureTyp_Corr_U: // R-Teiler 1:1 (-1.0..1.0V); 1Bit = 2.048V/(2hoch24) = 0.12207uV (max. +-2.048V)
      fScaledValue = (float)i32ADCConvData * ADS1248_INTREF_mV / 0x7FFFFFL; // [mV]
      if (fScaledValue > 2000.0)
        fScaledValue = 2000.0;
      else if (fScaledValue < -2000.0)
        fScaledValue = -2000.0;      
      break;      
    case eADS1248_MeasureTyp_Corr_I :// Rshunt 1k Ohm (+-1mA) -> invertiert (-1.0..1.0V); 1Bit = 2.048V/(2hoch24) = 0.12207uV (max. +-2.048V)
      fScaledValue = (float)i32ADCConvData * (ADS1248_INTREF_mV * -1) / 0x7FFFFFL; // [uA]
      if (fScaledValue > 2000.0)
        fScaledValue = 2000.0;
      else if (fScaledValue < -2000.0)
        fScaledValue = -2000.0;      
      break;
    default:
      fScaledValue = i32ADCConvData;
      break;
  }
  
  
  return fScaledValue; 
}

/**----------------------------------------------------------------------------
* @brief       Checks command data for restricted command 
* @details    
*
* @param[in]	void
*
* @retval			true		ok / valid
* @retval			false	  error
*/

void ADS1248_Board_REGISTER_PrepareAll( t_eADS1248_Board_MeasureTyp eADS1248_MeasureTyp)
{	
  // Offsets and FullScales (Default)
  sADS_Config_Write.OFC0 = sADS_Config_Read.OFC0;
  sADS_Config_Write.OFC1 = sADS_Config_Read.OFC1;
  sADS_Config_Write.OFC2 = sADS_Config_Read.OFC2;
  sADS_Config_Write.FSC0 = sADS_Config_Read.FSC0;
  sADS_Config_Write.FSC1 = sADS_Config_Read.FSC1;
  sADS_Config_Write.FSC2 = sADS_Config_Read.FSC2;   
  
  sADS_Config_Write.GPIOCFG = ADS1248_GPIOCFG_IOCFG_NO_GPIO;
  sADS_Config_Write.GPIODIR = AFS1248_GPIODIR_ALL_OUT;
  sADS_Config_Write.GPIODAT = ADS1248_GPIODAT_ALL_OFF;
  sADS_Config_Write.VBIAS = ADS1248_VBIAS_OFF; 
  
  sADS_Config_Write.SYS0 = ADS1248_SYS0_GAIN_1 | ADS1248_SYS0_DR_20; // GAIN 1 ±2.048 V; Datarate 20 SPS (50.825ms)
 
/* old used for < V1.05 (old Temperature Jumper needs to set to 3-Wire -> new Temperature Jumper needs to set to 2-Wire for Thermocouple) 
  if ((eADS1248_MeasureTyp >= eADS1248_MeasureTyp_ThermoCouple_E) && (eADS1248_MeasureTyp <= eADS1248_MeasureTyp_ThermoCouple_T))
    sADS_Config_Write.VBIAS = ADS1248_VBIAS_AIN2;
  else
    sADS_Config_Write.VBIAS = ADS1248_VBIAS_OFF; 
*/

  switch (eADS1248_MeasureTyp)
  {
    case eADS1248_MeasureTyp_PT1000_2Wire:      // Measure with IOut1-Current 100uA * 5k = 0.5V + (100uA * 2k) <= ca. 0.7V (Iout muss < 1.8V sein)
    case eADS1248_MeasureTyp_PT100_2Wire:       // Measure with IOut1-Current 1mA * 0.5k = 0.5V + (1mA * 700 Ohm) <= ca. 1.2V (Iout muss < 1.8V sein)
      sADS_Config_Write.SYS0 = ADS1248_SYS0_GAIN_2 | ADS1248_SYS0_DR_20; // GAIN 2 +-2k5; Datarate 20 SPS (50.825ms)
      sADS_Config_Write.MUX0 = ADS1248_MUX0_BCS_OFF | ADS1248_MUX0_SP_AIN0 | ADS1248_MUX0_SN_AIN1;    // Burn-out current source off; SP AIN0; SN AIN1
      sADS_Config_Write.MUX1 = ADS1248_MUX1_INT_VREF_ON | ADS1248_MUX1_REF0 | ADS1248_MUX1_MEAS_NORM; // Internal Osci used; Internal Ref. ON; REFP0+REFN0 used as reference; Normal measure  
      if (eADS1248_MeasureTyp == eADS1248_MeasureTyp_PT1000_2Wire)   
        sADS_Config_Write.IDAC0 = ADS1248_IDAC0_IMAG_100; // 100uA@IOUT1  
      else
        sADS_Config_Write.IDAC0 = ADS1248_IDAC0_IMAG_1000; // 1000uA@IOUT1      
      sADS_Config_Write.IDAC1 = ADS1248_IDAC1_I1DIR_EXT1 | ADS1248_IDAC1_I2DIR_OFF; // IDAC1 routed to IEXT1 / IOUT1    
      break;
    case eADS1248_MeasureTyp_PT1000_3Wire:      // First Measure with IOut1-Current 100uA + AIN2 100uA * 5k = 1V + (100uA * 2k) = ca. 1.2V (2. Messung mit IDAC1 + IDAC2 an den Stromausgängen tauschen und Messungen mitteln)
    case eADS1248_MeasureTyp_PT1000_3Wire_Inv:  // Second measure with invertet IOuts
    case eADS1248_MeasureTyp_PT100_3Wire:       // First Measure with IOut1-Current 500uA + AIN2 500uA * 0.5k = 0.5V + (1mA * 700 Ohm) <= ca. 1.2V (2. Messung mit IDAC1 + IDAC2 an den Stromausgängen tauschen und Messungen mitteln)
    case eADS1248_MeasureTyp_PT100_3Wire_Inv:   // Second measure with invertet IOuts      
      sADS_Config_Write.SYS0 = ADS1248_SYS0_GAIN_4 | ADS1248_SYS0_DR_20; // GAIN 4 +-2k5; Datarate 20 SPS (50.825ms)
      sADS_Config_Write.MUX0 = ADS1248_MUX0_BCS_OFF | ADS1248_MUX0_SP_AIN0 | ADS1248_MUX0_SN_AIN1;    // Burn-out current source off; SP AIN0; SN AIN1
      sADS_Config_Write.MUX1 = ADS1248_MUX1_INT_VREF_ON | ADS1248_MUX1_REF0 | ADS1248_MUX1_MEAS_NORM; // Internal Osci used; Internal Ref. ON; REFP0+REFN0 used as reference; Normal measure 
      if ((eADS1248_MeasureTyp == eADS1248_MeasureTyp_PT1000_3Wire) || (eADS1248_MeasureTyp == eADS1248_MeasureTyp_PT1000_3Wire_Inv))
        sADS_Config_Write.IDAC0 = ADS1248_IDAC0_IMAG_100; // 100uA@IOUT1 + 100uA@IOUT2 
      else
        sADS_Config_Write.IDAC0 = ADS1248_IDAC0_IMAG_500; // 500uA@IOUT1 + 500uA@IOUT2
      if ((eADS1248_MeasureTyp == eADS1248_MeasureTyp_PT1000_3Wire) || (eADS1248_MeasureTyp == eADS1248_MeasureTyp_PT100_3Wire))
        sADS_Config_Write.IDAC1 = ADS1248_IDAC1_I1DIR_EXT1 | ADS1248_IDAC1_I2DIR_AIN2; // IDAC1 routed to IEXT1/IOUT1 and IDAC2 routed to AIN2
      else       
        sADS_Config_Write.IDAC1 = ADS1248_IDAC1_I1DIR_AIN2 | ADS1248_IDAC1_I2DIR_EXT1; // IDAC1 routed to AIN2 and IDAC2 routed to IEXT1/IOUT1
      break;         
    case eADS1248_MeasureTyp_PT_Discharge_AIN:  
      sADS_Config_Write.SYS0 = ADS1248_SYS0_GAIN_1 | ADS1248_SYS0_DR_20; // PGA GAIN 32 ±64mV; Datarate 20 SPS (50.825ms)
      sADS_Config_Write.MUX0 = ADS1248_MUX0_BCS_OFF | ADS1248_MUX0_SP_AIN0 | ADS1248_MUX0_SN_AIN1;    // Burn-out current source off; SP AIN0; SN AIN1 
      sADS_Config_Write.MUX1 = ADS1248_MUX1_INT_VREF_ON | ADS1248_MUX1_REF0 | ADS1248_MUX1_MEAS_NORM; // Internal Osci used; Internal Ref. ON; REFP0+REFN0 used as reference; Normal measure           
      sADS_Config_Write.IDAC0 = ADS1248_IDAC0_IMAG_OFF;
      sADS_Config_Write.IDAC1 = ADS1248_IDAC1_I1DIR_OFF;     
      break;
    case eADS1248_MeasureTyp_ThermoCouple_E:
    case eADS1248_MeasureTyp_ThermoCouple_J:
    case eADS1248_MeasureTyp_ThermoCouple_K:
    case eADS1248_MeasureTyp_ThermoCouple_N:
    case eADS1248_MeasureTyp_ThermoCouple_R:
    case eADS1248_MeasureTyp_ThermoCouple_S:
    case eADS1248_MeasureTyp_ThermoCouple_T:       
      sADS_Config_Write.SYS0 = ADS1248_SYS0_GAIN_32 | ADS1248_SYS0_DR_20; // PGA GAIN 32 ±64mV; Datarate 20 SPS (50.825ms)
      sADS_Config_Write.MUX0 = ADS1248_MUX0_BCS_OFF | ADS1248_MUX0_SP_AIN0 | ADS1248_MUX0_SN_AIN1;    // Burn-out current source off; SP AIN0; SN AIN1
      sADS_Config_Write.MUX1 = ADS1248_MUX1_INT_VREF_ON | ADS1248_MUX1_INT | ADS1248_MUX1_MEAS_NORM; // Internal Osci used; Internal Ref. ON and used as reference; Normal measure 
      sADS_Config_Write.IDAC0 = ADS1248_IDAC0_IMAG_OFF;
      sADS_Config_Write.IDAC1 = ADS1248_IDAC1_I1DIR_OFF;
      break;    
    case eADS1248_MeasureTyp_U_HighRes: // 2019-01-07-Kd V1.03 R-Teiler 23.5 (0..6.5V) -> 0..276.5mV 
      sADS_Config_Write.SYS0 = ADS1248_SYS0_GAIN_4 | ADS1248_SYS0_DR_20; // GAIN 4 ±512 mV; Datarate 20 SPS (50.825ms)
    case eADS1248_MeasureTyp_U: // R-Teiler 23.5 (0..45V) -> 0..1914mV (GAIN 1)
    case eADS1248_MeasureTyp_I: // Rshunt 1 Ohm (0..100mA) X 19 -> 0..1900mV
    case eADS1248_MeasureTyp_Corr_U: // 1:1 (-1..+1V)
    case eADS1248_MeasureTyp_Corr_I: // -1..+1mA = +1V..-1V (invertiert)      
      if ((eADS1248_MeasureTyp == eADS1248_MeasureTyp_U) || (eADS1248_MeasureTyp == eADS1248_MeasureTyp_U_HighRes))
        sADS_Config_Write.MUX0 = ADS1248_MUX0_BCS_OFF | ADS1248_MUX0_SP_AIN3 | ADS1248_MUX0_SN_AIN7;    // Burn-out current source off; SP AIN3; SN AIN7
      else if (eADS1248_MeasureTyp == eADS1248_MeasureTyp_I)
        sADS_Config_Write.MUX0 = ADS1248_MUX0_BCS_OFF | ADS1248_MUX0_SP_AIN4 | ADS1248_MUX0_SN_AIN7;    // Burn-out current source off; SP AIN4; SN AIN7
      else if (eADS1248_MeasureTyp == eADS1248_MeasureTyp_Corr_I)
        sADS_Config_Write.MUX0 = ADS1248_MUX0_BCS_OFF | ADS1248_MUX0_SP_AIN5 | ADS1248_MUX0_SN_AIN7;    // Burn-out current source off; SP AIN5; SN AIN7
      else
        sADS_Config_Write.MUX0 = ADS1248_MUX0_BCS_OFF | ADS1248_MUX0_SP_AIN6 | ADS1248_MUX0_SN_AIN7;    // Burn-out current source off; SP AIN6; SN AIN7    
      sADS_Config_Write.MUX1 = ADS1248_MUX1_INT_VREF_ON | ADS1248_MUX1_INT | ADS1248_MUX1_MEAS_NORM; // Internal Osci used; Internal Ref. ON and used as reference; Normal measure 
      sADS_Config_Write.IDAC0 = ADS1248_IDAC0_IMAG_OFF;
      sADS_Config_Write.IDAC1 = ADS1248_IDAC1_I1DIR_OFF;    
      break;    
  }
  
  sADS_Config_Write.IDAC0 |= ADS1248_IDAC0_DRDY_ON; // DRDY both as Data Out an Data Ready 
}

/*
void ADS1248_Board_TEST_PIN_SET( uint8_t u8Mode )
{
  HAL_GPIO_WritePin(RCC_MCO_DO_GPIO_Port, RCC_MCO_DO_Pin, (u8Mode == HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
*/


/**----------------------------------------------------------------------------
* @brief       Set/Resets the reset line to the ADS1148
* @details     -
*
* @param[in]	mode can be HIGH or LOW
* @retval			none
*/
void ADS1248_Board_RESET( uint8_t u8Mode )
{
	HAL_GPIO_WritePin(_ADC_RESET_DO_GPIO_Port, _ADC_RESET_DO_Pin, ( u8Mode == HIGH ) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**----------------------------------------------------------------------------
* @brief       Set/Resets the start line to the ADS1148
* @details     -
*
* @param[in]	mode can be HIGH or LOW
* @retval			none
*/
void ADS1248_Board_START( uint8_t u8Mode )
{
	HAL_GPIO_WritePin(ADC_START_DO_GPIO_Port, ADC_START_DO_Pin, ( u8Mode == HIGH ) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/*@brief       Gets the state of the Data Ready Line (nDRDY) from the ADS1248
* @details     -
*
* @param			void
* @retval			state of the data ready line [HIGH / LOW]
*/
uint8_t ADS1248_Board_DATA_READY_STATE_GET( void )
{	
  return ( (HAL_GPIO_ReadPin(ADC_SPI3_MISO_GPIO_Port, ADC_SPI3_MISO_Pin) == GPIO_PIN_SET) ? HIGH : LOW); // ADS1148_IDAC0_DRDY_ON must be set !
  // return ( (HAL_GPIO_ReadPin(_ADC_DATA_READY_DI_GPIO_Port, _ADC_DATA_READY_DI_Pin) == GPIO_PIN_SET) ? HIGH : LOW);
}

/**----------------------------------------------------------------------------
* @brief       Set/Resets the chipselect line to the ADS1248
* @details     -
*
* @param[in]	mode can be HIGH or LOW
* @retval			none
*/
void ADS1248_Board_SPI_CS( uint8_t u8Mode )
{
	HAL_GPIO_WritePin(ADC_SPI3_CS_GPIO_Port, ADC_SPI3_CS_Pin, ( u8Mode == HIGH ) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

