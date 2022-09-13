/**============================================================================
* @file      BT.c
* @date      2018-06-11
*
* @author    D.Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     main file of bluetooth
*            
* VERSION:   
* 
* V0.01            2018-06-11-Kd     Create File 		
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "bsp.h"
#include "config.h"
#include "app.h"
#include "com.h"
#include "timeServer.h"
#include "low_power.h"
#include "bt.h"

/* Private define ------------------------------------------------------------*/

#define BT_OP_MODE                     e_BT_OpMode_AutoRunMode // To start the low power vsp BASIC app

#define BT_RX_DATA_SIZE                COM_TX_DATA_SIZE
#if (USE_COM_ECHO_ON == 1)
  #define BT_TX_DATA_SIZE              (BT_RX_DATA_SIZE + COM_TX_DATA_SIZE)
#else
  #define BT_TX_DATA_SIZE              COM_TX_DATA_SIZE
#endif

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
#define BT_HUART              huart3

#define BT_COM_TIME_ms        3000 // do not enter stop mode for min. 3s and is also the max send waiting time   

#define BT_RESTART_TIME_AFTER_COM_ms        (10*60000L) // Restart the BT-Modul after 10 minutes after communication (Fail safe)
#define BT_RESTART_TIME_AFTER_DAY_ms        (60*60000L) // Restart the BT-Modul after 60 minutes without communication (Fail safe)

/* Private variables ---------------------------------------------------------*/

uint8_t  u8RX_data_BT;
uint16_t i16Rx_Counter_BT = 0;
static uint16_t i16Tx_Counter_BT = 0;
volatile bool bRxDataReady_BT = false;
static volatile bool bRestart_BT = false;
#if USE_USART3_DMA == 1
static volatile bool bTxOngoing_BT = false;
static volatile bool bTxStopped_BT = false;
#endif
//static volatile bool bRxEnable = false;
static uint8_t  au8TxBufferBT[BT_TX_DATA_SIZE];
uint8_t  au8RxBufferBT[BT_RX_DATA_SIZE];

/* External variables -------------------------------------------------------*/

/* Private functions --------------------------------------------------------*/

/* Timer to handle SWResetTimer */
TimerEvent_t BTcomTimer;
TimerEvent_t BTrestartTimer;


/* Private function callback functions -------------------------------------------------------*/

// Aufwecken der UART aus dem StopMode mittels ExtI
static void BT_irqHandler_Rx( void )
{
  TimerStop(&BTcomTimer);
  TimerStart(&BTcomTimer);
  LowPower_Disable(e_LOW_POWER_UART_BT); 
  
  HW_GPIO_ClearIrq(  TP408_GPIO_Port, TP408_Pin); // Disable the IRQ, now the UART Rx is working (next char will be received)
  HAL_UART_Receive_IT(&BT_HUART,&u8RX_data_BT,1);  
  
  // ACHTUNG: Es dauert ca. 1ms bis HSE Quarz bereit ist ! Das heisst es wurden alle Zeichen innerhalb 1ms verschluckt !!!
  // Variante 1 : ":" verschicken > 2ms warten und danach das Cmd verschicken z.B. ":RtcGet_"
  // Variante 2 : "            :RtcGet_" >= 12 Leerzeichen voranstellen (12*86us=1.03ms)
}

static void OnBTcomTimerEvent( void )
{
  TimerStop(&BTcomTimer);
  if (bTxOngoing_BT)
  {
    bTxStopped_BT = true;
    HAL_UART_DMAStop(&BT_HUART);
    bTxOngoing_BT = false;
  }
  LowPower_Enable(e_LOW_POWER_UART_BT);  
  HW_GPIO_SetIrq( TP408_GPIO_Port, TP408_Pin, IRQ_HIGH_PRIORITY, BT_irqHandler_Rx );  
}

static void OnBTrestartTimerEvent( void )
{
  TimerStop(&BTrestartTimer);

  bRestart_BT = true;
}


/* Private function prototypes -----------------------------------------------*/
static void BT_Rx_IrqInit( void )
{
  GPIO_InitTypeDef initStruct={0};  	
//Initialize buttons (and ext. con) as interrupt sources
  initStruct.Pull = GPIO_NOPULL;
  initStruct.Speed = GPIO_SPEED_FREQ_HIGH;  
  initStruct.Mode = GPIO_MODE_IT_FALLING;		
  HW_GPIO_Init( TP408_GPIO_Port,TP408_Pin,&initStruct ); 
  HW_GPIO_SetIrq( TP408_GPIO_Port, TP408_Pin, IRQ_HIGH_PRIORITY, BT_irqHandler_Rx );
  
	TimerInit( &BTcomTimer, OnBTcomTimerEvent ); 
	TimerSetValue( &BTcomTimer, BT_COM_TIME_ms);	  
}

/*!
 * \brief Initializes the BT I/Os pins interface

static void BT_IoInit( void )
{
  GPIO_InitTypeDef GPIO_InitStruct={0};
  
  GPIO_InitStruct.Mode   = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed  = GPIO_SPEED_MEDIUM;  
  GPIO_InitStruct.Pull   = GPIO_NOPULL;
  
  // Bluetooth Control
  // HW_GPIO_Init(BT_nAutoRUN_GPIO_Port,BT_nAutoRUN_Pin,&GPIO_InitStruct);
  HW_GPIO_Init(BT_OTA_APPDL_GPIO_Port,BT_OTA_APPDL_Pin,&GPIO_InitStruct);
  // HW_GPIO_Init(BT_SIO_03_GPIO_Port,BT_SIO_03_Pin,&GPIO_InitStruct);	  
}
*/

/*!
 * \brief De-initializes the BT I/Os pins interface. 
 *
 * \remark Useful when going in MCU lowpower modes
 */
static void BT_IoDeInit( void )
{
  GPIO_InitTypeDef GPIO_InitStruct={0};
  
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;  
  
  // Bluetooth Control
  HW_GPIO_Init(BT_nAutoRUN_GPIO_Port,BT_nAutoRUN_Pin,&GPIO_InitStruct);    // BL652 hat einen PullDown am SIO_13/nAutoRUN Eingang 
  HW_GPIO_Init(BT_nRESET_GPIO_Port,BT_nRESET_Pin,&GPIO_InitStruct);        // BL652 hat einen PullUp am nRESET Eingang 
  HW_GPIO_Init(BT_OTA_APPDL_GPIO_Port,BT_OTA_APPDL_Pin,&GPIO_InitStruct);  // BL652 hat einen PullDown am SIO_02 Eingang 
  HW_GPIO_Init(BT_SIO_03_GPIO_Port,BT_SIO_03_Pin,&GPIO_InitStruct);	       // BL652 hat einen PullUp am SIO_03 Eingang 
  
  // After PowerUp we have the e_BT_OpMode_AutoRunMode active
}

/*!
 * \brief Resets BT device
 */
static void BT_Reset(void)
{
  GPIO_InitTypeDef GPIO_InitStruct={0};
  
  GPIO_InitStruct.Mode   = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed  = GPIO_SPEED_MEDIUM;  
  GPIO_InitStruct.Pull   = GPIO_NOPULL;
  
  // Bluetooth Control
  HW_GPIO_Init(BT_nRESET_GPIO_Port,BT_nRESET_Pin,&GPIO_InitStruct);  
  HAL_GPIO_WritePin(BT_nRESET_GPIO_Port, BT_nRESET_Pin, GPIO_PIN_RESET);    
  BSP_SleepDelayMs(120); // min. 100ms
  
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HW_GPIO_Init(BT_nRESET_GPIO_Port,BT_nRESET_Pin,&GPIO_InitStruct);   
}

/*!
 * \brief Set BT operating mode 
 */
static void BT_SetOpMode(e_BT_OpMode_t e_BT_OpMode)
{
  GPIO_InitTypeDef GPIO_InitStruct={0};
  
  GPIO_InitStruct.Mode   = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed  = GPIO_SPEED_MEDIUM;  
  GPIO_InitStruct.Pull   = GPIO_NOPULL;  
  
  BT_IoDeInit();
  
  if ((e_BT_OpMode == e_BT_OpMode_VSP_BridgeToUart) || (e_BT_OpMode == e_BT_OpMode_VSP_CmdMode))
  {
    HW_GPIO_Init(BT_OTA_APPDL_GPIO_Port,BT_OTA_APPDL_Pin,&GPIO_InitStruct);
    HAL_GPIO_WritePin(BT_OTA_APPDL_GPIO_Port, BT_OTA_APPDL_Pin, GPIO_PIN_SET);     
  }
  
  if ((e_BT_OpMode == e_BT_OpMode_VSP_BridgeToUart) || (e_BT_OpMode == e_BT_OpMode_InteractiveMode))
  {
    HW_GPIO_Init(BT_nAutoRUN_GPIO_Port,BT_nAutoRUN_Pin,&GPIO_InitStruct); 
    HAL_GPIO_WritePin(BT_nAutoRUN_GPIO_Port, BT_nAutoRUN_Pin, GPIO_PIN_SET);       
  }
  
  BT_Reset();     
  
  BSP_SleepDelayMs(200); // wait until BT has started
  
  // The BT has 13k Pulldowns @BT_nAutoRUN_Pin and @BT_OTA_APPDL_Pin
  GPIO_InitStruct.Mode   = GPIO_MODE_ANALOG;   
  HW_GPIO_Init(BT_OTA_APPDL_GPIO_Port,BT_OTA_APPDL_Pin,&GPIO_InitStruct);       
  HW_GPIO_Init(BT_nAutoRUN_GPIO_Port,BT_nAutoRUN_Pin,&GPIO_InitStruct);  
}


/* Global functions ---------------------------------------------------------*/

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void BT_UART_RxCpltCallback(void)
{	  
   if (i16Rx_Counter_BT == 0) {  
     if (u8RX_data_BT == ':')
         au8RxBufferBT[i16Rx_Counter_BT++] = u8RX_data_BT;      
   } 
   else if (i16Rx_Counter_BT < BT_RX_DATA_SIZE-1) {
     if (!((i16Rx_Counter_BT == 1) && (u8RX_data_BT == ':'))) // discard double start char 
       au8RxBufferBT[i16Rx_Counter_BT++] = u8RX_data_BT;
   }
   else
      i16Rx_Counter_BT = 0;      		      

   if ((u8RX_data_BT == '\n') && i16Rx_Counter_BT) {
     au8RxBufferBT[i16Rx_Counter_BT]= 0;
     bRxDataReady_BT = true;  // Stop Rx -> Handle message and restart Rx
   }
	 else
      HAL_UART_Receive_IT(&BT_HUART,&u8RX_data_BT,1);                
}

#if USE_USART3_DMA == 1
void BT_UART_TxCpltCallback(void)
{	  
  bTxOngoing_BT = false;  
}
#endif
/**
  * @brief  Rx Error callbacks.
  * @param  huart: Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void BT_UART_ErrorCallback(void)
{	  
  HAL_UART_Receive_IT(&BT_HUART,&u8RX_data_BT,1);  
}

bool BT_GetUART_Cts( void )
{
  return (HAL_GPIO_ReadPin(BT_USART3_CTS_GPIO_Port,BT_USART3_CTS_Pin) == GPIO_PIN_RESET ? true : false);
}

void BT_Init( void )
{
  BT_SetOpMode(BT_OP_MODE);
  HW_UART3_Init();
  BT_Rx_IrqInit();
  HAL_UART_Receive_IT(&BT_HUART,&u8RX_data_BT,1); 
	TimerInit( &BTrestartTimer, OnBTrestartTimerEvent ); 
	TimerSetValue( &BTrestartTimer, BT_RESTART_TIME_AFTER_DAY_ms);
  TimerStart(&BTrestartTimer);  
}
/*
bool BT_IsConnected(void)
{
 // Is in vSP not working -> RTS is also active when not connected ->  return ((HAL_GPIO_ReadPin( BT_USART3_RTS_GPIO_Port,BT_USART3_RTS_Pin) == GPIO_PIN_SET) ? true : false);
 
  return ?; // Müsste über IO mit eigenem BASIC Programm implementiert werden
}
*/

bool BT_HasWork( void )
{
  return (bRxDataReady_BT || bRestart_BT);
}

bool BT_SendData( uint8_t *pu8TxBuf, uint16_t ui16Len)
{  
#if USE_USART3_DMA == 1  
static   uint8_t aui8DmaTxBuf[COM_TX_DATA_SIZE];
  
  bool              bOk = false;
  HAL_StatusTypeDef halStatusType;
  // uint32_t  tickstart = 0;
  
  if (!ui16Len || !pu8TxBuf)
    return false;
  
  while (bTxOngoing_BT); // Max. 3s (ausser es werden wieder Zeichen empfangen)
  
  if (!bTxStopped_BT)
  {
    if (ui16Len > COM_TX_DATA_SIZE)
      ui16Len = COM_TX_DATA_SIZE;      
    memcpy(aui8DmaTxBuf, pu8TxBuf, ui16Len);
    halStatusType = HAL_UART_Transmit_DMA(&BT_HUART, aui8DmaTxBuf, ui16Len);
  // halStatusType = HAL_UART_Transmit_IT(&BT_HUART, pu8TxBuf, ui16Len); 
    if (halStatusType == HAL_OK)
    {        
      TimerStop(&BTcomTimer);
      LowPower_Disable(e_LOW_POWER_UART_BT);
      bTxOngoing_BT = true;
      TimerStart(&BTcomTimer);  // 2018-08-22-Kd V0.04 used to enabled the low power after max. 3s for shure               
      bOk = true;
    }
  }
  else
    bTxStopped_BT = false; 
/*  
  tickstart = HAL_GetTick();  
  do {
    if (!bTxOngoing_BT)
    {
      if (bTxStopped_BT)
      {
        bTxStopped_BT = false;
        break;
      }
      if (ui16Len > COM_TX_DATA_SIZE)
        ui16Len = COM_TX_DATA_SIZE;      
      memcpy(aui8DmaTxBuf, pu8TxBuf, ui16Len);
      halStatusType = HAL_UART_Transmit_DMA(&BT_HUART, aui8DmaTxBuf, ui16Len);
    // halStatusType = HAL_UART_Transmit_IT(&BT_HUART, pu8TxBuf, ui16Len); 
      if (halStatusType == HAL_OK)
      {        
        TimerStop(&BTcomTimer);
        LowPower_Disable(e_LOW_POWER_UART_BT);
        bTxOngoing_BT = true;
        TimerStart(&BTcomTimer);  // 2018-08-22-Kd V0.04 used to enabled the low power after max. 3s for shure               
        bOk = true;
      }
      break;
    }          
  } while ((HAL_GetTick() - tickstart) < BT_COM_TIME_ms); // die 128 Bytes müssen spätestens nach 3s weg sein  
 */ 
  return bOk;
#else  
  if (HAL_UART_Transmit(&BT_HUART, pu8TxBuf, ui16Len, BT_COM_TIME_ms) == HAL_OK)    
    return true;
  else
    return false;
#endif  
}

void BT_Do( void )
{ 
  bool bRequestTxProlong = false;
  bool bRxDataReady_BT_tmp;
  bool bRecCmd;
  
  if (bRestart_BT)
  {
    bRestart_BT = false;
    BT_SetOpMode(BT_OP_MODE);
    TimerStop(&BTrestartTimer);
    TimerSetValue( &BTrestartTimer, BT_RESTART_TIME_AFTER_DAY_ms);	
    TimerStart(&BTrestartTimer);
    
    OnBTcomTimerEvent(); // 2018-08-22-Kd ab V0.04 um sicherzustellen, dass LowPower wieder enabled ist
  }

  while (bRxDataReady_BT || bRequestTxProlong)
  { 
    bRxDataReady_BT_tmp = bRxDataReady_BT;
    if ((au8RxBufferBT[0] == ':') || bRequestTxProlong) {              // : is the telegramm start      
      TimerStop(&BTrestartTimer);      
      TimerSetValue( &BTrestartTimer, BT_RESTART_TIME_AFTER_COM_ms);	
      bRestart_BT = false;
      TimerStart(&BTrestartTimer);
//      PRINTF("[BT Rx]");
//      PRINTF((char*)au8RxBufferBT);   
      if (bRxDataReady_BT_tmp && (au8RxBufferBT[0] == ':'))
        bRecCmd = true;
      else
        bRecCmd = false;
#if (USE_COM_ECHO_ON == 1)
      i16Tx_Counter_BT = 0;
      memcpy(au8TxBufferBT, au8RxBufferBT, i16Rx_Counter_BT);
      i16Tx_Counter_BT += Com_HandleTelegramm( &au8RxBufferBT[1], bRecCmd ? (i16Rx_Counter_BT-1) : 0, &au8TxBufferBT[i16Tx_Counter_BT], &bRequestTxProlong);
#else
      i16Tx_Counter_BT = Com_HandleTelegramm( &au8RxBufferBT[1], bRecCmd ? (i16Rx_Counter_BT-1) : 0, au8TxBufferBT, &bRequestTxProlong);      
#endif      
      
      if (i16Tx_Counter_BT) 
        if (!BT_SendData( au8TxBufferBT, i16Tx_Counter_BT) && bRequestTxProlong) // ACHTUNG : in dieser Zeit kann im TxProlong-Betrieb auch wieder empfangen werden
        {
          Com_HandleTelegramm( NULL, 1, NULL, &bRequestTxProlong); // Stop Cmd (RxLen > 0)          
          bRestart_BT = true; // Connenction to USB Dongle lost	
        }
    }
    
    if (bRxDataReady_BT_tmp)
    {      
      TimerStop(&BTcomTimer);
      TimerStart(&BTcomTimer);  // Prolong the Rx communication Timer   (do not go to stop mode)
      u8RX_data_BT = 0;	
      i16Rx_Counter_BT = 0;  
      memset(au8RxBufferBT, 0, sizeof(au8RxBufferBT));
      bRxDataReady_BT = false;         
    }          
    if (!bRxDataReady_BT)
      HAL_UART_Receive_IT(&BT_HUART,&u8RX_data_BT,1); // Falls in BT_SendData() (ohne DMA) empfangen wurde kann im BT_UART_RxCpltCallback() das Empfangen mittels HAL_UART_Receive_IT() nicht aktiviert werden, da geblockt         
  }    
}

/*****END OF FILE****/
