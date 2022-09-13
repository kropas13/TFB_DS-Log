/**============================================================================
* @file      bsp.h
* @date      2016-11-10
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     header of bsp.c file (MCU and APP specific functions)

*            
* VERSION:   
* 
* V0.01      2016-11-10-Ra      Create File
*============================================================================*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_H__
#define __BSP_H__

#ifdef __cplusplus
 extern "C" {
#endif
      
/* Includes ------------------------------------------------------------------*/   
#include <stdint.h>
#include <stdbool.h>     
   
/* Exported types ------------------------------------------------------------*/
typedef struct {
  uint8_t ui8MuxTemp_Msk;
  uint8_t ui8MuxUI_Msk; 
  uint8_t ui8MuxImp_Msk; 
  bool    bMuxTEMP_PT1000; // 0=PT100 1=PT1000
} t_sBSP_ShiftReg_1;

typedef enum {
  eOP_GAIN_1 = 0,
  eOP_GAIN_2,
  eOP_GAIN_4,
  eOP_GAIN_8,
  eOP_GAIN_16,
  eOP_GAIN_32,
  eOP_GAIN_64,
  eOP_GAIN_128
} t_eBSP_OP_Gain;

typedef struct {
  uint8_t         ui8MuxCorrI_Msk;
  uint8_t         ui8MuxCorrU_Msk;    
  uint8_t         ui8MuxImp_Msk;  
  t_eBSP_OP_Gain  eOP_GainSetupImpU;  // 0..7
  t_eBSP_OP_Gain  eOP_GainSetupImpI;  // 0..7
  bool            bImpOP_EN;
} t_sBSP_ShiftReg_2;

/* Exported constants --------------------------------------------------------*/   
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

void BSP_ClearShiftRegister_1( void );
void BSP_ClearShiftRegister_2( void );
void BSP_SetShiftRegister_1( t_sBSP_ShiftReg_1* psShiftReg_1);
void BSP_SetShiftRegister_2( t_sBSP_ShiftReg_2* psShiftReg_2);
  
   /**
  * @brief  Returns the HW rev
  * @retval value of the HW rev((0..15)) (uint8_t)	
  */
uint8_t GetHWRev (void);
/**
  * @brief  Returns the state of the actual TxType
  * @retval either TX_NONE, TX_ON_EVENT, TX_ON_TIMER or both 
  */
uint8_t GetTxType(void);

/**
  * @brief  sets a tx type to be handled by the app
  * @param  TX_ON_EVENT, TX_ON_TIMER, TX_NONE
  */
void SetTxType(uint8_t TX_TYPE);

/**
  * @brief  setter and getter for red LED
  * @param  ON_OFF, true is on - false is off
  * @retval returns LEDs actual state	
  */
void SetLEDRed (bool ON_OFF);

bool GetLEDRed (void);

/**
  * @brief  setter and getter for green LED
  * @param  ON_OFF, true is on - false is off
  * @retval returns LEDs actual state	
  */
void SetLEDGreen (bool ON_OFF);

bool GetLEDGreen (void);

/**
  * @brief  setter and getter to show LED sign
  * @param  ON_OFF, true is on - false is off
  * @retval returns actual showLED state	
  */
void SetShowLED(bool ON_OFF);

bool GetShowLED (void);

/**
  * @brief  Setter and getter to turn lora chip on or off
  * @param  ON_OFF, true is on - false is off	
  * @retval Radio enable - Supply state (true if on, false if off)
  */
void SetLoraPower(bool ON_OFF);

bool GetLoraPower(void);

/**
 * @brief  reconfigures the buttons and the ext. con as interrupts
 *
 * @note
 * @retval None
 */	 
void BSP_ButtonIrqInit (void (* BtnResetCallback)(void) );	 

void irqHandlerButton(void);
  
/**
 * @brief  Turns on/off the System Vcc
 *
 * @note
 * @retval None
 */	
void BSP_SystemVcc(bool ON_OFF);

/**
 * @brief  Get System Vcc state
 *
 * @note
 * @retval bool TRUE System Vcc is ON
 */
bool BSP_IsSystemVccON( void );

/**
  * @brief  Sets a sleep delay
  * @param  time_ms : sleep delay in ms 
  */
void BSP_SleepDelayMs(uint32_t time_ms);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H__ */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
