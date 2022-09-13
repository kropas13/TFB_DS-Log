/**============================================================================
* @file      hw_uart.c
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     hw_uart defines and initializes the USART instance to be used for this project
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 			
*
*============================================================================*/
#include "hw.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*!
 * uart handle variable
 */
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_tx;

/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/**
* @brief initialize the UART object and MCU peripheral
*
* @details
*
* @param None 
*
* @retval None	
*/
void HW_UART1_Init(void)    //RS485
{
  /*## Configure the UART peripheral ######################################*/
  /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
  /* UART1 configured as follow:
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - Parity = no parity
      - BaudRate = 115200 baud
      - Hardware flow control disabled (RTS and CTS signals) */
  huart1.Instance        = USART1;
  huart1.Init.BaudRate   = 115200/*115200*/;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits   = UART_STOPBITS_1;
  huart1.Init.Parity     = UART_PARITY_NONE;
  huart1.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  huart1.Init.Mode       = UART_MODE_TX_RX;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if(HAL_UART_Init(&huart1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }
}

/**
* @brief deinitialize the UART object and MCU peripheral
*
* @details
*
* @param None 
*
* @retval None	
*/
void HW_UART1_DeInit(void)
{
  HAL_UART_DeInit(&huart1);
}

/**
* @brief initialize the UART object and MCU peripheral
*
* @details
*
* @param None 
*
* @retval None	
*/
void HW_UART3_Init(void)    //BT
{
  /*## Configure the UART peripheral ######################################*/
  /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
  /* UART3 configured as follow:
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - Parity = ODD parity
      - BaudRate = 115200 baud
      - Hardware flow control disabled (RTS and CTS signals) */  
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;  
  if(HAL_UART_Init(&huart3) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }
}

/**
* @brief deinitialize the UART object and MCU peripheral
*
* @details
*
* @param None 
*
* @retval None	
*/
void HW_UART3_DeInit(void)
{
  HAL_UART_DeInit(&huart3);   
}


/* MspInit and DeInit function definitions -----------------------------------*/
/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for UART interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  if (huart->Instance == USART1){
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable USART1 clock */
    __HAL_RCC_USART1_CLK_ENABLE(); 
    __NOP( );
    __NOP( );    
    /*##-2- Configure peripheral GPIO ##########################################*/  
    /* UART TX GPIO pin configuration  */  
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
 
		HW_GPIO_Init(RS485_USART1_TX_GPIO_Port,RS485_USART1_TX_Pin,&GPIO_InitStruct);
		HW_GPIO_Init(RS485_USART1_RX_GPIO_Port,RS485_USART1_RX_Pin,&GPIO_InitStruct);	
    
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    HAL_GPIO_WritePin( RS485_USART1_DE_GPIO_Port,RS485_USART1_DE_Pin, GPIO_PIN_RESET);     
    HW_GPIO_Init(RS485_USART1_DE_GPIO_Port,RS485_USART1_DE_Pin,&GPIO_InitStruct);   
    /* Peripheral interrupt init */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);     
   		
  } else if (huart->Instance == USART3){
    
#if USE_USART3_DMA == 1
    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    
    /* DMA interrupt init */
    /* DMA1_Channel2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
#endif    
    
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable USART1 clock */
    __HAL_RCC_USART3_CLK_ENABLE();     
    __NOP( );
    __NOP( );    
    /*##-2- Configure peripheral GPIO ##########################################*/  
    /**USART3 GPIO Configuration    
    PD11     ------> USART3_CTS
    PD12     ------> USART3_RTS
    PC10     ------> USART3_TX
    PC11     ------> USART3_RX 
    */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
   
		HW_GPIO_Init(BT_USART3_CTS_GPIO_Port,BT_USART3_CTS_Pin,&GPIO_InitStruct);
    HW_GPIO_Init(BT_USART3_RTS_GPIO_Port,BT_USART3_RTS_Pin,&GPIO_InitStruct);    
    /* UART TX GPIO pin configuration  */  
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;    
 
		HW_GPIO_Init(BT_USART3_TX_GPIO_Port,BT_USART3_TX_Pin,&GPIO_InitStruct);
    HW_GPIO_Init(BT_USART3_RX_GPIO_Port,BT_USART3_RX_Pin,&GPIO_InitStruct);
    
#if USE_USART3_DMA == 1   
    /* USART3 DMA USART3_TX Init */
    hdma_usart3_tx.Instance = DMA1_Channel2;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK)
    {
      //_Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(huart,hdmatx,hdma_usart3_tx);
#endif      
    
    /* Peripheral interrupt init */
    HAL_NVIC_SetPriority(USART3_IRQn, IRQ_VERY_HIGH_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);     
  }
}

/**
  * @brief UART MSP DeInit
  * @param huart: uart handle
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef GPIO_InitStruct={0};
  
  if (huart->Instance == USART1){
		__HAL_RCC_USART1_FORCE_RESET();
		__HAL_RCC_USART1_RELEASE_RESET();
		
		__HAL_RCC_USART1_CLK_DISABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;  
		
		HW_GPIO_Init(RS485_USART1_TX_GPIO_Port,RS485_USART1_TX_Pin,&GPIO_InitStruct);
		HW_GPIO_Init(RS485_USART1_RX_GPIO_Port,RS485_USART1_RX_Pin,&GPIO_InitStruct);	
    HW_GPIO_Init(RS485_USART1_DE_GPIO_Port,RS485_USART1_DE_Pin,&GPIO_InitStruct);     
		
  } else if (huart->Instance == USART3){
		__HAL_RCC_USART3_FORCE_RESET();
		__HAL_RCC_USART3_RELEASE_RESET();
		
		__HAL_RCC_USART3_CLK_DISABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
		 
    HW_GPIO_Init(BT_USART3_CTS_GPIO_Port,BT_USART3_CTS_Pin,&GPIO_InitStruct); // BL652 hat einen PullDown am UART_CTS Eingang 
    HW_GPIO_Init(BT_USART3_RTS_GPIO_Port,BT_USART3_RTS_Pin,&GPIO_InitStruct);    
    
		HW_GPIO_Init(BT_USART3_TX_GPIO_Port,BT_USART3_TX_Pin,&GPIO_InitStruct); // BL652 hat einen PullUp am UART_Rx Eingang 
		HW_GPIO_Init(BT_USART3_RX_GPIO_Port,BT_USART3_RX_Pin,&GPIO_InitStruct);		
  }
}

/*****END OF FILE****/
