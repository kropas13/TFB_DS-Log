/**============================================================================
* @file      ads1248.c
* @date      2018-06-15
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     ADS1248 meassure functions
*            
* VERSION:   
* 
* V0.01      2018-06-15-Kd      Create File
*
*============================================================================*/


/* Includes ------------------------------------------------------------------------------ */
#include "hw.h"
#include "bsp.h"
#include "ads1248_board.h"
#include "ads1248.h"

/* Private define --------------------------------------------------------------*/
#define ADC_SPI_TIMEOUT_MS      100

/* Command Definitions */
// System Control
#define ADS1248_CMD_WAKEUP    	0x00
#define ADS1248_CMD_SLEEP     	0x03
#define ADS1248_CMD_SYNC     		0x05
#define ADS1248_CMD_RESET    		0x07
#define ADS1248_CMD_NOP    			0xFF
// Data Read
#define ADS1248_CMD_RDATA    		0x13
#define ADS1248_CMD_RDATAC    	0x15
#define ADS1248_CMD_SDATAC    	0x17
// Read Register
#define ADS1248_CMD_RREG    		0x20
// Write Register
#define ADS1248_CMD_WREG    		0x40
// Calibration
#define ADS1248_CMD_SYSOCAL    	0x60
#define ADS1248_CMD_SYSGCAL    	0x61
#define ADS1248_CMD_SELFOCAL    0x62
// Restricted
#define ADS1248_CMD_RESTRICT		0xF1


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

/* Private typedef -------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/                                   
extern SPI_HandleTypeDef    ADS1248_HSPIx;
ts_ADS1248_RegMap           sADS_Config_Write;
ts_ADS1248_RegMap           sADS_Config_Read;
t_eADS1248_Board_MeasureTyp g_eADS1248_MeasureTyp;
int16_t                     g_i16ColdJunctionT_GC_100;

/* Private function prototypes -----------------------------------------------*/
/* Timer callback functions ---------------------------------------------------------*/
/* IRQ callback functions ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/	
/*


void ADS1248_DATA_READY_MODE_SET( void )
{
	uint8_t temp;
	
	temp = ADS1248_IDAC0_DRDY_ON | ADS1248_IDAC0_IMAG_OFF;
	
	ADS1248_REGISTER_WRITE(ADS1248_REG_10_IDAC0, 0x01, &temp);
	
}
*/

HAL_StatusTypeDef ADS1248_SPI_WRITE_BYTE( uint8_t u8TxByte ){
	return (HAL_SPI_Transmit(&ADS1248_HSPIx, &u8TxByte, 1, ADC_SPI_TIMEOUT_MS));
}

HAL_StatusTypeDef ADS1248_REGISTER_WRITE(uint8_t StartAddress, uint8_t NumRegs, uint8_t * pData)
{
	uint8_t           i;
  HAL_StatusTypeDef halState;

  while (1)
  {
    // send the command byte
    halState = ADS1248_SPI_WRITE_BYTE(ADS1248_CMD_WREG | (StartAddress & 0x0f));
    if (halState != HAL_OK)
      break;
    halState = ADS1248_SPI_WRITE_BYTE((NumRegs-1) & 0x0f);
    
    // send the data bytes
    for (i=0; (halState == HAL_OK) && (i < NumRegs); i++)
      ADS1248_SPI_WRITE_BYTE(*pData++);	
    break;
  }
  
  return halState;
}

HAL_StatusTypeDef ADS1248_SYSCONTROL_SDATAC( void ){	
	return( ADS1248_SPI_WRITE_BYTE( ADS1248_CMD_SDATAC ) );
}

// SYNC command resets the ADC digital filter and starts a new conversion
HAL_StatusTypeDef ADS1248_SYSCONTROL_SYNC( void ){
	
	uint8_t aui8DataSend[2];
	
	// prepare output data
	aui8DataSend[0] = ADS1248_CMD_SYNC;
	aui8DataSend[1] = ADS1248_CMD_SYNC;

	// send command
	return(HAL_SPI_Transmit(&ADS1248_HSPIx, &aui8DataSend[0], 2, ADC_SPI_TIMEOUT_MS));
}

// Read data continuous mode
HAL_StatusTypeDef ADS1248_SYSCONTROL_RDATAC( void ){	
	return (ADS1248_SPI_WRITE_BYTE( ADS1248_CMD_RDATAC ));
}

HAL_StatusTypeDef ADS1248_SYSCONTROL_WAKEUP( void ){	
	return( ADS1248_SPI_WRITE_BYTE( ADS1248_CMD_WAKEUP ));
}

HAL_StatusTypeDef ADS1248_SYSCONTROL_SLEEP( void ){
	return( ADS1248_SPI_WRITE_BYTE( ADS1248_CMD_SLEEP ) );
}

HAL_StatusTypeDef ADS1248_SYSCONTROL_RESET( void ){
	return( ADS1248_SPI_WRITE_BYTE( ADS1248_CMD_RESET ) );
}

HAL_StatusTypeDef ADS1248_SYSCONTROL_DATA_READ_CONT_STOP( void ){
	return( ADS1248_SPI_WRITE_BYTE( ADS1248_CMD_SDATAC ) );
}

static void ADS1248_Init_HW( void)
{
  ADS1248_Board_IO_Init();
  
  ADS1248_Board_RESET ( HIGH );
	
  BSP_SleepDelayMs(18);
  // HAL_Delay(18); // The ADC needs 16ms after power on
}

static void ADS1248_DeInit_HW( void)
{
	ADS1248_Board_SPI_CS( HIGH );
    
  ADS1248_Board_IO_DeInit();
}

/**----------------------------------------------------------------------------
* @brief       Reads the conversion data
* @details     -
*
* @param			void
* @retval			HAL_StatusTypeDef
*/
HAL_StatusTypeDef ADS1248_CONVERSION_READ( int32_t* pi32ConvData)
{
	HAL_StatusTypeDef retVal;
	uint8_t           aui8DataSend[2];
	uint8_t           aui8DataRead[3];
	uint8_t           aui8DataNOP[3];
	
	// prepare output data
	aui8DataSend[0] = ADS1248_CMD_RDATA;
	memset(aui8DataNOP, ADS1248_CMD_NOP, sizeof(aui8DataNOP));
		
	// send command
  retVal = HAL_SPI_Transmit(&ADS1248_HSPIx, aui8DataSend, 1, ADC_SPI_TIMEOUT_MS);
	if(retVal == HAL_OK)
	{	
    // receive registers, send NOP commands
    retVal = HAL_SPI_TransmitReceive(&ADS1248_HSPIx, aui8DataNOP, aui8DataRead, 3, ADC_SPI_TIMEOUT_MS);
    if(retVal == HAL_OK)
    {    
      *pi32ConvData = ((uint32_t)(aui8DataRead[0]) << 16) + ((uint32_t)(aui8DataRead[1]) << 8) + aui8DataRead[2];      
      if( *pi32ConvData >= 0x00800000)
        *pi32ConvData |= 0xFF000000; // Negative             
    }
  }

	return( retVal );
}	

/**----------------------------------------------------------------------------
* @brief       Compares 
* @details    
*
* @param[in]	void
*
* @retval			true		ok / valid
* @retval			false 	error
*/
bool ADS1248_REGISTER_CompareAll( bool bCompareCalibRegs )
{

	bool bRetVal = false;

	sADS_Config_Read.IDAC0 &= 0x0F;	
  if (!bCompareCalibRegs)
  {
    if (!memcmp( &sADS_Config_Write, &sADS_Config_Read, ADS1248_REG_3_SYS0+1)) 
      if (!memcmp( &sADS_Config_Write.IDAC0, &sADS_Config_Read.IDAC0, ADS1248_REG_14_GPIODAT-ADS1248_REG_10_IDAC0)) // without GPIODAT
        bRetVal = true;
  }
  else if (!memcmp( &sADS_Config_Write, &sADS_Config_Read, sizeof(ts_ADS1248_RegMap)-1)) // without GPIODAT
    bRetVal = true;
  
  return( bRetVal );	
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

void ADS1248_REGISTER_PrepareAll( t_eADS1248_Board_MeasureTyp eADS1248_MeasureTyp)
{	
  // sADS_Config_Write = sADS_Config_Read;
  
  ADS1248_Board_REGISTER_PrepareAll(eADS1248_MeasureTyp);
}

/**----------------------------------------------------------------------------
* @brief       Set/Resets the chipselect line to the ADS1248
* @details     -
*
* @param[in]	mode can be HIGH or LOW
* @retval			HAL_StatusTypeDef
*/
HAL_StatusTypeDef ADS1248_REGISTER_WRITE_ALL( bool bWriteCalibRegs)
{
	HAL_StatusTypeDef retVal = HAL_OK;
	uint8_t           aui8DataSend[2+sizeof(ts_ADS1248_RegMap)];

  //To prevent an overload (specially for the PGA) check gain changes
  // Change to lower gain ?
  if ((sADS_Config_Write.SYS0 & 0xf0) < (sADS_Config_Read.SYS0 & 0xf0)) 
  {
    // First change the PGA-Gain (SYS0 Reg) and then the channel (MUX0 1.Reg.)
    aui8DataSend[0] = ADS1248_CMD_WREG | ADS1248_REG_3_SYS0;
    aui8DataSend[1] = 1-1;    // number of registers - 1   
    aui8DataSend[2] = sADS_Config_Write.SYS0;
    
    // Send command
    retVal = HAL_SPI_Transmit(&ADS1248_HSPIx, aui8DataSend, 3, ADC_SPI_TIMEOUT_MS);    
  }
  if (retVal == HAL_OK)
  {
    if (bWriteCalibRegs)
    {
      // ATTENTION : PGA change will reload factory settings of the FSC register (FullScale Calib.)
      //             -> they will be overwritten by the given values
      // write command, 15 registers
      aui8DataSend[0] = ADS1248_CMD_WREG | ADS1248_REG_0_MUX0;
      aui8DataSend[1] = sizeof(ts_ADS1248_RegMap)-1;    // number of registers - 1
      
      // data to be send to registers
      memcpy(&aui8DataSend[2], &sADS_Config_Write, sizeof(ts_ADS1248_RegMap));
      
      // Send command
      retVal = HAL_SPI_Transmit(&ADS1248_HSPIx, aui8DataSend, sizeof(aui8DataSend), ADC_SPI_TIMEOUT_MS);
    }
    else
    {
      // write command 1
      aui8DataSend[0] = ADS1248_CMD_WREG | ADS1248_REG_0_MUX0;
      aui8DataSend[1] = ADS1248_REG_3_SYS0-ADS1248_REG_0_MUX0;    // number of registers - 1 ( MUX0 + VBIAS + MUX1 + SYS0)      
      // data to be send to registers
      memcpy(&aui8DataSend[2], &sADS_Config_Write, ADS1248_REG_3_SYS0+1);      
      // Send command 1
      retVal = HAL_SPI_Transmit(&ADS1248_HSPIx, aui8DataSend, ADS1248_REG_3_SYS0+3, ADC_SPI_TIMEOUT_MS);
      if (retVal == HAL_OK)
      {
        // write command 2
        aui8DataSend[0] = ADS1248_CMD_WREG | ADS1248_REG_10_IDAC0;
        aui8DataSend[1] = ADS1248_REG_14_GPIODAT-ADS1248_REG_10_IDAC0;    // number of registers - 1 ( MUX0 + VBIAS + MUX1 + SYS0)      
        // data to be send to registers
        memcpy(&aui8DataSend[2], &sADS_Config_Write.IDAC0, ADS1248_REG_14_GPIODAT-ADS1248_REG_10_IDAC0+1);      
        // Send command 1
        retVal = HAL_SPI_Transmit(&ADS1248_HSPIx, aui8DataSend, ADS1248_REG_14_GPIODAT-ADS1248_REG_10_IDAC0+3, ADC_SPI_TIMEOUT_MS);
      }      
    }
  }
  
  BSP_SleepDelayMs(2);
	// HAL_Delay(2); // Settling time
		
	return( retVal );
}


/**----------------------------------------------------------------------------
* @brief       Reads all registers
* @details     -
*
* @param			void
* @retval			HAL_StatusTypeDef
*/
HAL_StatusTypeDef ADS1248_REGISTER_READ_ALL( void )
{

	HAL_StatusTypeDef retVal;
	uint8_t           aui8DataSend[2];
	ts_ADS1248_RegMap sDataRead;
	uint8_t           aui8DataNOP[sizeof(ts_ADS1248_RegMap)];
	
	// prepare output data
	aui8DataSend[0] = ADS1248_CMD_RREG | ADS1248_REG_0_MUX0;
	aui8DataSend[1] = sizeof(ts_ADS1248_RegMap)-1;    // number of registers - 1
	
	// send command
  retVal = HAL_SPI_Transmit(&ADS1248_HSPIx, aui8DataSend, 2, ADC_SPI_TIMEOUT_MS);
	if(retVal == HAL_OK)
	{
    // receive registers, send NOP commands
    memset(aui8DataNOP, ADS1248_CMD_NOP, sizeof(aui8DataNOP));
    retVal = HAL_SPI_TransmitReceive(&ADS1248_HSPIx, aui8DataNOP, (uint8_t*)&sDataRead, sizeof(ts_ADS1248_RegMap), ADC_SPI_TIMEOUT_MS);
    if(retVal == HAL_OK)
    {    	      
      //fill register structure
      sADS_Config_Read = sDataRead;
    }
  }
	
	return( retVal );
}	


/* Global functions ---------------------------------------------------------*/
void ADS1248_Init( void )
{
  ADS1248_Init_HW();
  
	ADS1248_Board_SPI_CS( LOW );
	ADS1248_SYSCONTROL_RESET();   
  HAL_Delay(2); // must wait 0.6ms    
}

void ADS1248_DeInit( void )
{
  ADS1248_Board_SPI_CS( HIGH ); 
  HAL_Delay(2);          
  ADS1248_DeInit_HW ();
}

bool ADS1248_CalibSysOffset( int32_t* pi32ADCConvData, float* pfScaledValue, bool bScaling)
{
  bool bRetVal = false; // Timeout
  
  
  while (1) {

	
    // ADS1248_TEST_PIN_SET(LOW);
    if (bRetVal)
    {
      ADS1248_CONVERSION_READ( pi32ADCConvData);
    
      if (bScaling && (pfScaledValue != NULL))
        *pfScaledValue = ADS1248_Board_Scale( *pi32ADCConvData, g_eADS1248_MeasureTyp, g_i16ColdJunctionT_GC_100);
    }
    
    break;
  }  
  
  return bRetVal;
}

bool ADS1148_CalibSysOffset( t_eADS1248_Board_MeasureTyp eADS1248_MeasureTyp,  tsADS1248_ChSysOffset* psADS1248_ChSysOffset)
{
  bool bRetVal = false; // Timeout
  
  while (1)
  {
    if (!ADS1148_SetupMeasure(eADS1248_MeasureTyp, 0))
      break;  
    if (ADS1248_SPI_WRITE_BYTE( ADS1248_CMD_SYSOCAL ) != HAL_OK)
      break;
    // Takes about 800ms
    for (int i=0; i<500; i++) {    
      BSP_SleepDelayMs(2);
      if( ADS1248_Board_DATA_READY_STATE_GET() == LOW)
      {
        bRetVal = true;
        break;
      }    
    }
    
    if (ADS1248_REGISTER_READ_ALL() != HAL_OK)
      break;
    psADS1248_ChSysOffset->OFC0 = sADS_Config_Read.OFC0;
    psADS1248_ChSysOffset->OFC1 = sADS_Config_Read.OFC1;
    psADS1248_ChSysOffset->OFC2 = sADS_Config_Read.OFC2;
    
    bRetVal = true;
    break;
  }
  
  return bRetVal;  
}

bool ADS1148_SetupMeasure( t_eADS1248_Board_MeasureTyp eADS1248_MeasureTyp, int16_t i16ColdJunctionT_GC_100)
{
  bool bRetVal = false; // Timeout
  
  g_eADS1248_MeasureTyp = eADS1248_MeasureTyp;
  g_i16ColdJunctionT_GC_100 = i16ColdJunctionT_GC_100;
  while (1) {
    if (ADS1248_REGISTER_READ_ALL() != HAL_OK)
      break;
    ADS1248_REGISTER_PrepareAll(eADS1248_MeasureTyp);
    if (ADS1248_REGISTER_WRITE_ALL(false) != HAL_OK)
      break;
    if (ADS1248_REGISTER_READ_ALL() != HAL_OK)
      break;	
    if( !ADS1248_REGISTER_CompareAll(false))      
      break; // HW error  
    bRetVal = true;
    break;
  }
  
  return bRetVal;
}   

bool ADS1248_Measure( int32_t* pi32ADCConvData, float* pfScaledValue, bool bScaling)
{
  bool bRetVal = false; // Timeout
  
  
  while (1) {
    if( ADS1248_SYSCONTROL_SYNC() != HAL_OK) // starts a new conversion (will take about 50ms@20SPS) 
      break;    
    // ADS1248_TEST_PIN_SET(HIGH);    

    for (int i=0; i<55; i++) {   
      BSP_SleepDelayMs(2);
      if( ADS1248_Board_DATA_READY_STATE_GET() == LOW)
      {
        bRetVal = true;
        break;
      }      
    }
	
    // ADS1248_TEST_PIN_SET(LOW);
    if (bRetVal)
    {
      ADS1248_CONVERSION_READ( pi32ADCConvData);
    
      if (bScaling && (pfScaledValue != NULL))
        *pfScaledValue = ADS1248_Board_Scale( *pi32ADCConvData, g_eADS1248_MeasureTyp, g_i16ColdJunctionT_GC_100);
    }
    
    break;
  }  
  
  return bRetVal;
}
