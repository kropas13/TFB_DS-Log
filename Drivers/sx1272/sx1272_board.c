/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SX1272 driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
/**============================================================================
* @file      sx1272_board.c
* @date      2016-12-05
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     all SX1272 config. initializations and functions used for project defined here
*            
* VERSION:   
* 
* V0.10      2016-12-05-Ra      Create File 			
*
*============================================================================*/
  
/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "radio.h"
#include "sx1272.h"
#include "sx1272_board.h"

#if defined (DEF_TCXO)
#define TCXO_ON()   HAL_GPIO_WritePin( RADIO_TCXO_VCC_PORT, RADIO_TCXO_VCC_PIN, GPIO_PIN_SET)  

#define TCXO_OFF()  HAL_GPIO_WritePin( RADIO_TCXO_VCC_PORT, RADIO_TCXO_VCC_PIN, GPIO_PIN_RESET)
#endif

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    SX1272IoInit,
    SX1272IoDeInit,
    SX1272Init,
    SX1272GetStatus,
    SX1272SetModem,
    SX1272SetChannel,
    SX1272IsChannelFree,
    SX1272Random,
    SX1272SetRxConfig,
    SX1272SetTxConfig,
    SX1272CheckRfFrequency,
    SX1272GetTimeOnAir,
    SX1272Send,
    SX1272SetSleep,
    SX1272SetStby, 
    SX1272SetRx,
    SX1272StartCad,
    SX1272ReadRssi,
    SX1272Write,
    SX1272Read,
    SX1272WriteBuffer,
    SX1272ReadBuffer,
    SX1272SetSyncWord,
    SX1272SetMaxPayloadLength
};

#if defined (DEF_TCXO)
void SX1272SetXO( uint8_t state )
{
  if (state == SET )
  {
    TCXO_ON(); 
    HAL_Delay( BOARD_WAKEUP_TIME ); //start up time of TCXO
  }
  else
  {
    TCXO_OFF(); 
  }
}
#endif

void SX1272IoInit( void )
{
  GPIO_InitTypeDef initStruct={0};
  
  initStruct.Mode = GPIO_MODE_IT_RISING;
  initStruct.Pull = GPIO_PULLUP;
  initStruct.Speed = GPIO_SPEED_HIGH;
  /*adapt the pinning to the project! 2016-12-05-Ra  */
  HW_GPIO_Init( RADIO_DIO_0_PORT, RADIO_DIO_0_PIN, &initStruct );
  HW_GPIO_Init( RADIO_DIO_1_PORT, RADIO_DIO_1_PIN, &initStruct );
  HW_GPIO_Init( RADIO_DIO_2_PORT, RADIO_DIO_2_PIN, &initStruct );
  //HW_GPIO_Init( RADIO_DIO_3_PORT, RADIO_DIO_3_PIN, &initStruct );
  //HW_GPIO_Init( RADIO_DIO_4_PORT, RADIO_DIO_4_PIN, &initStruct );
  //HW_GPIO_Init( RADIO_DIO_5_PORT, RADIO_DIO_5_PIN, &initStruct );  
  
}

void SX1272IoIrqInit( DioIrqHandler **irqHandlers )
{
  /*adapt the INT definition to the project!*/
  HW_GPIO_SetIrq( RADIO_DIO_0_PORT, RADIO_DIO_0_PIN, IRQ_HIGH_PRIORITY, irqHandlers[0] );
  HW_GPIO_SetIrq( RADIO_DIO_1_PORT, RADIO_DIO_1_PIN, IRQ_HIGH_PRIORITY, irqHandlers[1] );
  HW_GPIO_SetIrq( RADIO_DIO_2_PORT, RADIO_DIO_2_PIN, IRQ_HIGH_PRIORITY, irqHandlers[2] );
  //HW_GPIO_SetIrq( RADIO_DIO_3_PORT, RADIO_DIO_3_PIN, IRQ_HIGH_PRIORITY, irqHandlers[3] );
  //HW_GPIO_SetIrq( RADIO_DIO_4_PORT, RADIO_DIO_4_PIN, IRQ_HIGH_PRIORITY, irqHandlers[4] );
  //HW_GPIO_SetIrq( RADIO_DIO_5_PORT, RADIO_DIO_5_PIN, IRQ_HIGH_PRIORITY, irqHandlers[5] );  
}

void SX1272IoDeInit( void )
{
  GPIO_InitTypeDef initStruct={0};

  initStruct.Mode = GPIO_MODE_IT_RISING ; //GPIO_MODE_ANALOG;
  initStruct.Pull = GPIO_PULLDOWN;
  
  HW_GPIO_Init( RADIO_DIO_0_PORT, RADIO_DIO_0_PIN, &initStruct );
  HW_GPIO_Init( RADIO_DIO_1_PORT, RADIO_DIO_1_PIN, &initStruct );
  HW_GPIO_Init( RADIO_DIO_2_PORT, RADIO_DIO_2_PIN, &initStruct );	
  //HW_GPIO_Init( RADIO_DIO_3_PORT, RADIO_DIO_3_PIN, &initStruct );
  //HW_GPIO_Init( RADIO_DIO_4_PORT, RADIO_DIO_4_PIN, &initStruct );
  //HW_GPIO_Init( RADIO_DIO_5_PORT, RADIO_DIO_5_PIN, &initStruct );    
}

uint8_t SX1272GetPaSelect( uint32_t channel )
{
	/* set this value according to the HW connection to the SX1272 Chip!*/
    return RF_PACONFIG_PASELECT_PABOOST;
}

void SX1272SetAntSwLowPower( bool status )
{
  //Ant Switch Controlled by SX1272 IC
}

void SX1272AntSwInit( void )
{
  //Ant Switch Controlled by SX1272 IC
}

void SX1272AntSwDeInit( void )
{
  //Ant Switch Controlled by SX1272 IC
}

void SX1272SetAntSw( uint8_t rxTx )
{
  SX1272.RxTx = rxTx;
}

bool SX1272CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}
/*****END OF FILE****/
