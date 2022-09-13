/**============================================================================
* @file      hw.c
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     grouping of all hw_XX inits and other hw definitions
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 		
* V0.02      2016-12-28-Ra      htim10 and sw_timer added					
*
*============================================================================*/
#include "hw.h"
#include "bsp.h"
#include "radio.h"
#include "debug.h"
#include "meas.h"
#include "app.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

/* Private function prototypes -----------------------------------------------*/
/* Exported functions ---------------------------------------------------------*/

/**
  * @brief This function initializes the hardware
  * @param None
  * @retval None
  */
void HW_Init( bool bBackupPowerOn  )
{
  if( McuInitialized == false )
  {
    HW_IOs_Init();

    // if Module OFF, put SPI and IOs to analog
    if (!GetLoraPower()){
		  HW_RadioIoInit_Analog();
		} else {
		  Radio.IoInit( );  
      HW_SPI2_Init( );
		}
    
    HW_RTC_Init(bBackupPowerOn );					             
    
    HW_SDIO_SD_Init(); // SDCard (only variable init)
  
		// HW_UART3_Init();  //BT    
   
    McuInitialized = true;
  }
}

/**
  * @brief This function Deinitializes the hardware
  * @param None
  * @retval None
  */
void HW_DeInit( void )
{	
  HW_IOs_DeInit();
  
  // if Module OFF, put SPI and IOs to analog
  if (!GetLoraPower()){
    HW_RadioIoInit_Analog();
	} else {
    HW_SPI2_DeInit( );		
    Radio.IoDeInit( );
	}		    
  McuInitialized = false;
}

/**
  * @brief This function return a random seed
  * @note based on the device unique ID
  * @param None
  * @retval see
  */
uint32_t HW_GetRandomSeed( void )
{
  return ( ( *( uint32_t* )ID1 ) ^ ( *( uint32_t* )ID2 ) ^ ( *( uint32_t* )ID3 ) );
}

/**
  * @brief This function return a unique ID
  * @param unique ID
  * @retval none
  */
void HW_GetUniqueId( uint8_t *id )
{
    id[7] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
    id[6] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
    id[5] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
    id[4] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
    id[3] = ( ( *( uint32_t* )ID2 ) ) >> 24;
    id[2] = ( ( *( uint32_t* )ID2 ) ) >> 16;
    id[1] = ( ( *( uint32_t* )ID2 ) ) >> 8;
    id[0] = ( ( *( uint32_t* )ID2 ) );
}

/**
  * @brief This function return a unique chip ID
  * @param unique chip ID
  * @retval none
  */
void HW_GetChipId( uint8_t chipId[12] )
{
    chipId[11] = ( ( *( uint32_t* )ID1 ) ) >> 24;
    chipId[10] = ( ( *( uint32_t* )ID1 ) ) >> 16;
    chipId[9]  = ( ( *( uint32_t* )ID1 ) ) >> 8;
    chipId[8]  = ( ( *( uint32_t* )ID1 ) );  
    chipId[7]  = ( ( *( uint32_t* )ID2 ) ) >> 24;
    chipId[6]  = ( ( *( uint32_t* )ID2 ) ) >> 16;
    chipId[5]  = ( ( *( uint32_t* )ID2 ) ) >> 8;
    chipId[4]  = ( ( *( uint32_t* )ID2 ) );
    chipId[3]  = ( ( *( uint32_t* )ID3 ) ) >> 24;
    chipId[2]  = ( ( *( uint32_t* )ID3 ) ) >> 16;
    chipId[1]  = ( ( *( uint32_t* )ID3 ) ) >> 8;
    chipId[0]  = ( ( *( uint32_t* )ID3 ) );
}

/**
  * @brief This function return the battery level
  * @param none
  * @retval the battery level  1 (very low) to 254 (fully charged)
  */
uint8_t HW_GetBatteryLevel( void ) 
{   
  return MEASGetBatteryLevel();
}


/**
  * @brief Enters Low Power Stop Mode
  * @note ARM exists the function when waking up
  * @param none
  * @retval none
  */
void HW_EnterStopMode( void)
{
  BACKUP_PRIMASK();

  DISABLE_IRQ( );
    
  HW_DeInit( );
  
#ifdef USE_HSE_PINS_AI_IN_STOP    
	/* Switch from HSE to HSI to save energy while sleeping (HSE Pins to analog)*/
	SystemClockHSI_Config();   
#endif
  
  /*clear wake up flag*/
  SET_BIT(PWR->CR, PWR_CR_CWUF);
    
  RESTORE_PRIMASK( );

  /* Enter Stop Mode */
  HAL_PWR_EnterSTOPMode ( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
}

/**
  * @brief Exists Low Power Stop Mode
  * @note Enable the pll at 32MHz
  * @param none
  * @retval none
  */
void HW_ExitStopMode( void)
{
  /* Disable IRQ while the MCU is not running on HSI */

  BACKUP_PRIMASK();
  
  DISABLE_IRQ( );

  /* After wake-up from STOP reconfigure the system clock */
#ifdef USE_HSE  
  #ifdef USE_HSE_PINS_AI_IN_STOP
  /* Enable HSI */
  __HAL_RCC_HSI_ENABLE();
  /* Wait till HSI is ready */
  while( __HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET ) {}  // Takes about 3.7us (max. 6us)    
  #else
    /* Enable HSE */
    __HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
    /* Wait till HSE is ready */
    while( __HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET ) {} // Takes about 1ms
   #endif
#else  
  /* Enable HSI */
  __HAL_RCC_HSI_ENABLE();
  /* Wait till HSI is ready */
  while( __HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET ) {}  // Takes about 3.7us (max. 6us)
#endif
  /* Enable PLL */
  __HAL_RCC_PLL_ENABLE();
  /* Wait till PLL is ready */
  while( __HAL_RCC_GET_FLAG( RCC_FLAG_PLLRDY ) == RESET ) {}
  
  /* Select PLL as system clock source */
  __HAL_RCC_SYSCLK_CONFIG ( RCC_SYSCLKSOURCE_PLLCLK );
  
  /* Wait till PLL is used as system clock source */ 
  while( __HAL_RCC_GET_SYSCLK_SOURCE( ) != RCC_SYSCLKSOURCE_STATUS_PLLCLK ) {}
    
#ifdef USE_HSE_PINS_AI_IN_STOP
  /* Reconfigure HSE as PLL source */
  SystemClockHSE_Config();
#endif
    
  /*initilizes the peripherals*/
  HW_Init( false );
  RESTORE_PRIMASK( );
	
}

/**
  * @brief Enters Low Power Sleep Mode
  * @note ARM exits the function when waking up
  * @param none
  * @retval none
  */
void HW_EnterSleepMode( void)
{
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

/*****END OF FILE****/

