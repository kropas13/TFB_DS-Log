/**============================================================================
* @file      bsp.c
* @date      2016-11-10
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     Definition and implementation of MCU functions used accross the application

*            
* VERSION:   
* 
* V0.01      2016-11-10-Ra      Create File REV00
* V1.02      2018-10-29-Kd      REV01 the Shiftregister now got pull down at th output (so use OE pin)
*============================================================================*/
  
  /* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "timeServer.h"
#include "low_power.h"
#include "loraModem.h"
#include "bsp.h"
#include "vcom.h"
#include "app.h"
#include "radio.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RESET_TIME                3000 //3 seconds

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static uint8_t u8TxType           = TX_NONE;
static bool    bShowLED           = false;
static void (* BtnResetCB)(void);

/* Timer to handle DelayTimer */
static TimerEvent_t DelayTimer;
volatile static bool bDelayOver = true;
static bool bDelayTimerInit = false;

/* Timer to handle SWResetTimer */
static TimerEvent_t SWResetTimer;

/* Private function prototypes -----------------------------------------------*/

/* Timer callback functions -------------------------------------------------------*/
static void OnDelayTimerEvent( void )
{
    TimerStop( &DelayTimer );
    bDelayOver = true;
}

static void OnSWResetTimerEvent( void )
{
    TimerStop(&SWResetTimer);
		if (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_PIN)){
      if (BtnResetCB)
        BtnResetCB();
		  
      // NVIC_SystemReset();  //SW Reset done here
    }	
}

/* IRQ callback functions ---------------------------------------------------------*/
/*!
 * @brief Callback - Left button
 */
void irqHandlerButton(void)
{
  SetTxType(TX_ON_EVENT);
	if (LoraTxRxStatus()){
		SetShowLED(true);
	}	
  TimerStop(&SWResetTimer);	
  TimerStart(&SWResetTimer);	  
}

/* Exported functions ---------------------------------------------------------*/

/**
  * @brief  Returns the HW rev
  * @retval value of the HW rev((0..15) (uint8_t)	
  */
uint8_t GetHWRev (void)
{
  static bool     bInitReadRevDone = false;  
  static uint8_t  u8Rev = 0;
  GPIO_InitTypeDef initStruct={0};
  
  if (!bInitReadRevDone)
  {
    bInitReadRevDone = true;
    
    //reconfigure pins to inputs with pull up
    initStruct.Mode   = GPIO_MODE_INPUT;
    initStruct.Speed  = GPIO_SPEED_MEDIUM;  
    initStruct.Pull   = GPIO_PULLUP;     
    
    initStruct.Pin    = REV_BIT0_Pin;
    HAL_GPIO_Init(REV_BIT0_GPIO_Port, &initStruct);   
    initStruct.Pin    = REV_BIT1_Pin;
    HAL_GPIO_Init(REV_BIT1_GPIO_Port, &initStruct);
    initStruct.Pin    = REV_BIT2_Pin;
    HAL_GPIO_Init(REV_BIT2_GPIO_Port, &initStruct); 
    initStruct.Pin    = REV_BIT3_Pin;
    HAL_GPIO_Init(REV_BIT3_GPIO_Port, &initStruct);  
    
    //read the DIP SW values (invers)
    u8Rev |= HAL_GPIO_ReadPin( REV_BIT0_GPIO_Port, REV_BIT0_Pin) ? 0x00 : 0x01;
    u8Rev |= HAL_GPIO_ReadPin( REV_BIT1_GPIO_Port, REV_BIT1_Pin) ? 0x00 : 0x02;  
    u8Rev |= HAL_GPIO_ReadPin( REV_BIT2_GPIO_Port, REV_BIT2_Pin) ? 0x00 : 0x04;
    u8Rev |= HAL_GPIO_ReadPin( REV_BIT3_GPIO_Port, REV_BIT3_Pin) ? 0x00 : 0x08;
    
    //reconfigure pins to analogical
    initStruct.Mode   = GPIO_MODE_ANALOG;
    initStruct.Pull   = GPIO_NOPULL;
    
    initStruct.Pin    = REV_BIT0_Pin;
    HAL_GPIO_Init(REV_BIT0_GPIO_Port, &initStruct);   
    initStruct.Pin    = REV_BIT1_Pin;
    HAL_GPIO_Init(REV_BIT1_GPIO_Port, &initStruct);
    initStruct.Pin    = REV_BIT2_Pin;
    HAL_GPIO_Init(REV_BIT2_GPIO_Port, &initStruct); 
    initStruct.Pin    = REV_BIT3_Pin;
    HAL_GPIO_Init(REV_BIT3_GPIO_Port, &initStruct); 
  }
  
  return u8Rev;
} 

/**
  * @brief  Returns the state of the TxOnEvent flag
  * @retval true means TxOnEvent 
  */
uint8_t GetTxType(void)
{
  return u8TxType;
}

/**
  * @brief  sets a tx on event flag to be handled by the app
  * @param  TxOnEvent on (true) or off(false)
  */
void SetTxType(uint8_t TX_TYPE){
  if ((u8TxType != TX_NONE) && (TX_TYPE != TX_NONE)){
	  u8TxType |= TX_TYPE;
  } else {
    u8TxType = TX_TYPE;
  }    
}


void SetLEDRed (bool ON_OFF)
{
  if (ON_OFF){  
		HAL_GPIO_WritePin( LED_RED_GPIO_PORT, LED_RED_PIN, GPIO_PIN_SET);  
  }else {
		HAL_GPIO_WritePin( LED_RED_GPIO_PORT, LED_RED_PIN, GPIO_PIN_RESET);  			   
	}	
}

bool GetLEDRed (void)
{
	bool bRetVal = false;
  bRetVal = (bool)(HAL_GPIO_ReadPin( LED_RED_GPIO_PORT, LED_RED_PIN));
  return bRetVal;  
}


void SetLEDGreen (bool ON_OFF)
{
  if (ON_OFF){  
		HAL_GPIO_WritePin( LED_GREEN_GPIO_PORT, LED_GREEN_PIN, GPIO_PIN_SET);  
  }else {
		HAL_GPIO_WritePin( LED_GREEN_GPIO_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);  			 
	}		
}

bool GetLEDGreen (void)
{
   bool bRetVal = false;
   bRetVal = (bool)(HAL_GPIO_ReadPin( LED_GREEN_GPIO_PORT, LED_GREEN_PIN));
   return bRetVal;
}

bool GetShowLED (void)
{
   return bShowLED;	
}

void SetShowLED(bool ON_OFF)
{
   bShowLED = ON_OFF;
}	

/**
  * @brief  turns Lora Chip and its IOs on or off
  * @retval none
  */
void SetLoraPower(bool ON_OFF){
	if(ON_OFF){	    
		HAL_GPIO_WritePin( EN_LORA_GPIO_PORT, EN_LORA_PIN, GPIO_PIN_RESET);         
    HAL_Delay(10);  		
		Radio.IoInit();
		HW_SPI2_Init();
	}else{	     
		HAL_GPIO_WritePin( EN_LORA_GPIO_PORT, EN_LORA_PIN, GPIO_PIN_SET); 		
		HW_SPI2_DeInit();
		HW_RadioIoInit_Analog();
	}	 
}

/**
  * @brief  Returns actual state of Radio enable pin
  * @retval Radio enable state (true if on, false if off)
  */
bool GetLoraPower(void){
   bool bRetVal = false;
   bRetVal = (bool)(!HAL_GPIO_ReadPin( EN_LORA_GPIO_PORT, EN_LORA_PIN));
   return bRetVal;	 
}

// Takes about +10%
void Delay_us(uint16_t u16_us)
{
  uint32_t counter;
  
  /* Compute number of CPU cycles to wait for */
  counter = (u16_us * (SystemCoreClock / 5000000U));
  while(counter != 0U)
  {
    counter--;
  }
}

// CNTRL TEMP/UI/IMP MEAS SETUP
void BSP_ClearShiftRegister_1( void )
{
  if (BSP_IsSystemVccON())
  { 
    HAL_GPIO_WritePin(SHFT1_REG_CLK_GPIO_Port, SHFT1_REG_CLK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SHFT1_STORE_CLR_GPIO_Port, SHFT1_STORE_CLR_Pin, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(SHFT1_REG_CLK_GPIO_Port, SHFT1_REG_CLK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SHFT1_STORE_CLR_GPIO_Port, SHFT1_STORE_CLR_Pin, GPIO_PIN_SET);   
  }
}

void BSP_InitIOShiftRegister_1( void )
{
  GPIO_InitTypeDef initStruct={0};    

// Set to PP Ouputs
  initStruct.Mode  = GPIO_MODE_OUTPUT_PP; // or GPIO_MODE_OUTPUT_OD slower
  initStruct.Speed = GPIO_SPEED_MEDIUM; 
  initStruct.Pull  = GPIO_NOPULL; 
  
  HAL_GPIO_WritePin(SHFT1_STORE_CLR_GPIO_Port, SHFT1_STORE_CLR_Pin, GPIO_PIN_RESET); 
  HAL_GPIO_WritePin(SHFT1_REG_CLK_GPIO_Port, SHFT1_REG_CLK_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SHFT1_SERIAL_DATA_GPIO_Port, SHFT1_SERIAL_DATA_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SHFT1_STORE_CLK_GPIO_Port, SHFT1_STORE_CLK_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SHFT1_ENABLE_GPIO_Port, SHFT1_ENABLE_Pin, GPIO_PIN_SET);              // /OE High -> Output Disable (REV00 Output states are undefined after power up)
  
  HW_GPIO_Init(SHFT1_STORE_CLR_GPIO_Port, SHFT1_STORE_CLR_Pin, &initStruct);       
  HW_GPIO_Init(SHFT1_REG_CLK_GPIO_Port, SHFT1_REG_CLK_Pin, &initStruct);  
  HW_GPIO_Init(SHFT1_SERIAL_DATA_GPIO_Port, SHFT1_SERIAL_DATA_Pin, &initStruct);  
  HW_GPIO_Init(SHFT1_STORE_CLK_GPIO_Port, SHFT1_STORE_CLK_Pin, &initStruct);    
  HW_GPIO_Init(SHFT1_ENABLE_GPIO_Port, SHFT1_ENABLE_Pin, &initStruct); 

  BSP_ClearShiftRegister_1(); 

  HAL_GPIO_WritePin(SHFT1_ENABLE_GPIO_Port, SHFT1_ENABLE_Pin, GPIO_PIN_RESET);              // /OE Low -> Output Enable
}

void BSP_DeInitIOShiftRegister_1( void )
{  
  GPIO_InitTypeDef initStruct={0};
  
  BSP_ClearShiftRegister_1();

// Set to Analog inputs
  initStruct.Mode  = GPIO_MODE_ANALOG;
  initStruct.Pull  = GPIO_NOPULL;  
  
  HW_GPIO_Init(SHFT1_STORE_CLR_GPIO_Port, SHFT1_STORE_CLR_Pin, &initStruct);  
  HW_GPIO_Init(SHFT1_REG_CLK_GPIO_Port, SHFT1_REG_CLK_Pin, &initStruct);  
  HW_GPIO_Init(SHFT1_SERIAL_DATA_GPIO_Port, SHFT1_SERIAL_DATA_Pin, &initStruct);    
  HW_GPIO_Init(SHFT1_STORE_CLK_GPIO_Port, SHFT1_STORE_CLK_Pin, &initStruct); 
  HW_GPIO_Init(SHFT1_ENABLE_GPIO_Port, SHFT1_ENABLE_Pin, &initStruct);    
}

void BSP_SerWriteShiftRegister_1( bool bBit)
{
  HAL_GPIO_WritePin(SHFT1_SERIAL_DATA_GPIO_Port, SHFT1_SERIAL_DATA_Pin, bBit ? GPIO_PIN_SET :  GPIO_PIN_RESET); 
  HAL_GPIO_WritePin(SHFT1_STORE_CLK_GPIO_Port, SHFT1_STORE_CLK_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SHFT1_STORE_CLK_GPIO_Port, SHFT1_STORE_CLK_Pin, GPIO_PIN_RESET);    
}

// CNTRL TEMP/UI/IMP MEAS SETUP
void BSP_SetShiftRegister_1( t_sBSP_ShiftReg_1* psShiftReg_1 )
{
  int idx, msk;
  
  if (BSP_IsSystemVccON())
  {   
    HAL_GPIO_WritePin(SHFT1_STORE_CLR_GPIO_Port, SHFT1_STORE_CLR_Pin, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(SHFT1_STORE_CLR_GPIO_Port, SHFT1_STORE_CLR_Pin, GPIO_PIN_SET);   
    
    HAL_GPIO_WritePin(SHFT1_REG_CLK_GPIO_Port, SHFT1_REG_CLK_Pin, GPIO_PIN_RESET);
    
    // Start with last bit
    for (idx=0, msk=0x80; idx<8; idx++, msk >>= 1)
      BSP_SerWriteShiftRegister_1(psShiftReg_1->ui8MuxImp_Msk & msk);
    for (idx=0, msk=0x80; idx<8; idx++, msk >>= 1)
      BSP_SerWriteShiftRegister_1(psShiftReg_1->ui8MuxUI_Msk & msk);    
    BSP_SerWriteShiftRegister_1(psShiftReg_1->bMuxTEMP_PT1000);      
    BSP_SerWriteShiftRegister_1(false);
    for (idx=0, msk=0x80; idx<8; idx++, msk >>= 1)
      BSP_SerWriteShiftRegister_1(psShiftReg_1->ui8MuxTemp_Msk & msk);  

    HAL_GPIO_WritePin(SHFT1_REG_CLK_GPIO_Port, SHFT1_REG_CLK_Pin, GPIO_PIN_SET);  
  }
}

// CNTRL CORR/IMP-GAIN MEAS SETUP
void BSP_ClearShiftRegister_2( void )
{
  if (BSP_IsSystemVccON())
  {   
    HAL_GPIO_WritePin(SHFT2_REG_CLK_GPIO_Port, SHFT2_REG_CLK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SHFT2_STORE_CLR_GPIO_Port, SHFT2_STORE_CLR_Pin, GPIO_PIN_SET); 
    HAL_GPIO_WritePin(SHFT2_REG_CLK_GPIO_Port, SHFT2_REG_CLK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SHFT2_STORE_CLR_GPIO_Port, SHFT2_STORE_CLR_Pin, GPIO_PIN_RESET);   
    Delay_us(1);   
  }
}

void BSP_InitBeforePowerIOShiftRegister_2( void )
{
  GPIO_InitTypeDef initStruct={0};
    
// Set to PP Ouputs
  initStruct.Mode  = GPIO_MODE_OUTPUT_PP; // or GPIO_MODE_OUTPUT_OD slower
  initStruct.Speed = GPIO_SPEED_MEDIUM; 
  initStruct.Pull  = GPIO_NOPULL; 
  
  // REV00 output states are undefined after power up;    >= REV01 shifter outputs are pulled down
  HAL_GPIO_WritePin(SHFT2_ENABLE_GPIO_Port, SHFT2_ENABLE_Pin, GPIO_PIN_RESET);  // /OE High -> Output Disable 

  HW_GPIO_Init(SHFT2_ENABLE_GPIO_Port, SHFT2_ENABLE_Pin, &initStruct);
}
  
void BSP_InitIOShiftRegister_2( void )
{
  GPIO_InitTypeDef initStruct={0};
    
// Set to PP Ouputs
  initStruct.Mode  = GPIO_MODE_OUTPUT_PP; // or GPIO_MODE_OUTPUT_OD slower
  initStruct.Speed = GPIO_SPEED_MEDIUM; 
  initStruct.Pull  = GPIO_NOPULL; 
   
  // Has a inverting level shifter !
  HAL_GPIO_WritePin(SHFT2_STORE_CLR_GPIO_Port, SHFT2_STORE_CLR_Pin, GPIO_PIN_SET); 
  HAL_GPIO_WritePin(SHFT2_REG_CLK_GPIO_Port, SHFT2_REG_CLK_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SHFT2_SERIAL_DATA_GPIO_Port, SHFT2_SERIAL_DATA_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SHFT2_STORE_CLK_GPIO_Port, SHFT2_STORE_CLK_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SHFT2_ENABLE_GPIO_Port, SHFT2_ENABLE_Pin, GPIO_PIN_SET);
    
  HW_GPIO_Init(SHFT2_STORE_CLR_GPIO_Port, SHFT2_STORE_CLR_Pin, &initStruct);       
  HW_GPIO_Init(SHFT2_REG_CLK_GPIO_Port, SHFT2_REG_CLK_Pin, &initStruct);  
  HW_GPIO_Init(SHFT2_SERIAL_DATA_GPIO_Port, SHFT2_SERIAL_DATA_Pin, &initStruct);  
  HW_GPIO_Init(SHFT2_STORE_CLK_GPIO_Port, SHFT2_STORE_CLK_Pin, &initStruct);    
  HW_GPIO_Init(SHFT2_ENABLE_GPIO_Port, SHFT2_ENABLE_Pin, &initStruct);   

  BSP_ClearShiftRegister_2();  
  
  HAL_GPIO_WritePin(SHFT2_ENABLE_GPIO_Port, SHFT2_ENABLE_Pin, GPIO_PIN_SET);              // /OE Low -> Output Enable
}

void BSP_DeInitIOShiftRegister_2( void )
{  
  GPIO_InitTypeDef initStruct={0};
  
  BSP_ClearShiftRegister_2();

// Set to Analog inputs
  initStruct.Mode   = GPIO_MODE_ANALOG;
  initStruct.Pull   = GPIO_NOPULL;  
  
  HW_GPIO_Init(SHFT2_STORE_CLR_GPIO_Port, SHFT2_STORE_CLR_Pin, &initStruct);  
  HW_GPIO_Init(SHFT2_REG_CLK_GPIO_Port, SHFT2_REG_CLK_Pin, &initStruct);  
  HW_GPIO_Init(SHFT2_SERIAL_DATA_GPIO_Port, SHFT2_SERIAL_DATA_Pin, &initStruct);    
  HW_GPIO_Init(SHFT2_STORE_CLK_GPIO_Port, SHFT2_STORE_CLK_Pin, &initStruct); 
  // HW_GPIO_Init(SHFT2_ENABLE_GPIO_Port, SHFT2_ENABLE_Pin, &initStruct); // Output disable
}

void BSP_SerWriteShiftRegister_2( bool bBit)
{   
  HAL_GPIO_WritePin(SHFT2_SERIAL_DATA_GPIO_Port, SHFT2_SERIAL_DATA_Pin, bBit ? GPIO_PIN_RESET :  GPIO_PIN_SET);  
  Delay_us(1);  // the SHFT2_STORE_CLK_Pin, GPIO_PIN_SET ad the end needs also this time
  HAL_GPIO_WritePin(SHFT2_STORE_CLK_GPIO_Port, SHFT2_STORE_CLK_Pin, GPIO_PIN_RESET); // Clock
  HAL_GPIO_WritePin(SHFT2_STORE_CLK_GPIO_Port, SHFT2_STORE_CLK_Pin, GPIO_PIN_SET);
}

// Takes about 2.7ms
void BSP_SetShiftRegister_2( t_sBSP_ShiftReg_2* psShiftReg_2)
{
  int idx, msk;
  
  if (BSP_IsSystemVccON())
  {          
    HAL_GPIO_WritePin(SHFT2_REG_CLK_GPIO_Port, SHFT2_REG_CLK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SHFT2_STORE_CLR_GPIO_Port, SHFT2_STORE_CLR_Pin, GPIO_PIN_SET); 
    HAL_GPIO_WritePin(SHFT2_STORE_CLR_GPIO_Port, SHFT2_STORE_CLR_Pin, GPIO_PIN_RESET);     
    
    // Start with last bit
    BSP_SerWriteShiftRegister_2(psShiftReg_2->bImpOP_EN); 
    for (idx=0, msk=0x04; idx<3; idx++, msk >>= 1)
      BSP_SerWriteShiftRegister_2(psShiftReg_2->eOP_GainSetupImpI & msk);
    for (idx=0, msk=0x04; idx<3; idx++, msk >>= 1)
      BSP_SerWriteShiftRegister_2(psShiftReg_2->eOP_GainSetupImpU & msk);  
    for (idx=0, msk=0x80; idx<8; idx++, msk >>= 1)
      BSP_SerWriteShiftRegister_2(psShiftReg_2->ui8MuxCorrU_Msk & msk);    
    for (idx=0, msk=0x80; idx<8; idx++, msk >>= 1)
      BSP_SerWriteShiftRegister_2(psShiftReg_2->ui8MuxCorrI_Msk & msk);

    HAL_GPIO_WritePin(SHFT2_REG_CLK_GPIO_Port, SHFT2_REG_CLK_Pin, GPIO_PIN_RESET);  
  }
}

void BSP_ButtonIrqInit (void (* BtnResetCallback)(void))
{
  GPIO_InitTypeDef initStruct={0};  
   
  BtnResetCB = BtnResetCallback;
  
//Initialize buttons (and ext. con) as interrupt sources
  initStruct.Pull = GPIO_NOPULL;
  initStruct.Speed = GPIO_SPEED_FREQ_HIGH;  
  initStruct.Mode = GPIO_MODE_IT_RISING;		
  HW_GPIO_Init( BUTTON_GPIO_PORT,BUTTON_PIN,&initStruct ); 
  HW_GPIO_SetIrq( BUTTON_GPIO_PORT, BUTTON_PIN, IRQ_HIGH_PRIORITY, irqHandlerButton );
	TimerInit( &SWResetTimer, OnSWResetTimerEvent ); 
	TimerSetValue( &SWResetTimer, RESET_TIME);		  
}

void BSP_SystemVcc(bool ON_OFF)
{    
  if (ON_OFF) {
    BSP_InitBeforePowerIOShiftRegister_2();
    
    HAL_GPIO_WritePin(DCDC_PS_MODE_GPIO_Port, DCDC_PS_MODE_Pin, GPIO_PIN_SET); // Power Save Mode disabled (benötigt etwa 10mA mehr, ist aber für +3V3_ext nötig!!!)
    HAL_Delay(2);
    HAL_GPIO_WritePin(EN_SYSTEM_GPIO_PORT, EN_SYSTEM_PIN, GPIO_PIN_RESET);  // VCC System on
    LowPower_Disable(e_LOW_POWER_3V3_SYS);
    HAL_Delay(3); // Wait until power is good 
    BSP_InitIOShiftRegister_1();
    BSP_InitIOShiftRegister_2();
#if defined (DEF_RS485)	  
    HW_UART1_Init();  //RS485 (VCOM)
#endif	   
  }
  else
  {
    BSP_DeInitIOShiftRegister_1();
    BSP_DeInitIOShiftRegister_2();
    HAL_GPIO_WritePin(EN_SYSTEM_GPIO_PORT, EN_SYSTEM_PIN, GPIO_PIN_SET);    // VCC System off
    HAL_GPIO_WritePin(DCDC_PS_MODE_GPIO_Port, DCDC_PS_MODE_Pin, GPIO_PIN_RESET); // Power Save Mode enabled
#if defined (DEF_RS485)	  
    HW_UART1_DeInit();  //RS485 (VCOM)
#endif	   
    LowPower_Enable(e_LOW_POWER_3V3_SYS);
    HAL_Delay(3); // Wait until power is down
    

/* V1.10 2021-02-04-Kd eben doch nicht auf analog setzen
    // V1.08 2020-06-18-Kd
    // Set to Analog inputs
    GPIO_InitTypeDef initStruct={0}; 
    initStruct.Mode   = GPIO_MODE_ANALOG;
    initStruct.Pull   = GPIO_NOPULL;      
    HW_GPIO_Init(SHFT2_ENABLE_GPIO_Port, SHFT2_ENABLE_Pin, &initStruct); // do not source anymore (ansonsten benötigt max. 3.3V / 105k6 = 31uA falls über +3V3_sys Strom abfliessen kann)
*/    
  }
    
}

bool BSP_IsSystemVccON(void)
{
  return (HAL_GPIO_ReadPin(EN_SYSTEM_GPIO_PORT,EN_SYSTEM_PIN) == GPIO_PIN_RESET);
}

/**
  * @brief  Sets a sleep delay
  * @param  time_ms : sleep delay in ms 
  */
void BSP_SleepDelayMs(uint32_t time_ms)
{  
#if defined (LOW_POWER_DISABLE)  
   HAL_Delay(time_ms);
#else
  if (!bDelayTimerInit){
    TimerInit( &DelayTimer, OnDelayTimerEvent );  
    bDelayTimerInit = true;
	}
	
  if (!bDelayOver) {
    HAL_Delay(time_ms); // Sleep timer already running, so we stay in here 
  } else {    
    bDelayOver = false;
    TimerSetValue( &DelayTimer, time_ms );    
    TimerStart( &DelayTimer );
    while (!bDelayOver)
    {			
      DISABLE_IRQ( );  
			/* if an interrupt has occurred after DISABLE_IRQ, it is kept pending 
			 * and cortex will not enter low power anyway  */
			if ( !bDelayOver) {
				LowPower_Handler( ); // Sleep or Stop
			}
			ENABLE_IRQ();
    }
  }     
#endif   
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
