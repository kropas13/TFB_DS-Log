/**============================================================================
* @file      hw_dac.c
* @date      2018-06-21
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     initialization and definition of dac instance for this project
*            
* VERSION:   
* 
* V0.01      2018-06-21-Kd      Create File 			
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include "hw.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DAC_12Bit              // When not defined -> 8 Bit 2024-03-18-Kd neu 12Bit DAC verwenden um weniger Störungen zu haben
#define DAC_PERIOD_Sinus_Hz       1000  // 1kHz Sinus
// 2024-03-18-Kd old <= V1.10 #define DAC_PERIOD_DATA_CNT       200   // 200 * 1kHz = 200kHz DAC Update rate (5us Settling time)
#define DAC_PERIOD_DATA_CNT       400   // 400 * 1kHz = 400kHz DAC Update rate (2.5us Settling time)
#define DAC_DMA_TIMER_FREQ_Hz     (DAC_PERIOD_Sinus_Hz * DAC_PERIOD_DATA_CNT)

/* External variables --------------------------------------------------------*/
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac_ch2;
void (* XferCpltCB)(void);
#ifdef DAC_12Bit
  uint16_t          au16Sinus1kHz[DAC_PERIOD_DATA_CNT];
#else
  uint8_t           au8Sinus1kHz[DAC_PERIOD_DATA_CNT];
#endif  

/* Exported macros -----------------------------------------------------------*/
/* Private functions -------------------------------------------------------- */ 
   
/* IRQ callback functions ---------------------------------------------------------*/
void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef* hdac)
{
  if (XferCpltCB)
    XferCpltCB();
}

/* Exported functions ------------------------------------------------------- */ 
void HW_DAC_FillSinusTable(void)
{
  /* Konstante Pi definieren */
  const double Pi = 3.141592653;
  
  double winkel;
  double rad;
  double sinus;
  int    i;
  
  /* Schleife zur Berechnung der Sinuswerte */
  for (i = 0; i < DAC_PERIOD_DATA_CNT; i++)
  {
      winkel = (360.0 / DAC_PERIOD_DATA_CNT) * i;
  
      rad = winkel * Pi / 180; /* Berechnen des Bogenmaßwinkels */
      sinus = sin(rad);
    #ifdef DAC_12Bit
      au16Sinus1kHz[i] = 2048 + (int16_t)(sinus * (4096.0/3.3 * 1.0)); // 12Bit=4096Stufen / 3.3V * 1Vp
    #else
      au8Sinus1kHz[i] = 128 + (int8_t)(sinus * (256.0/3.3 * 1.0)); // 8Bit=256Stufen / 3.3V * 1Vp
    #endif
      
  }
}

void HW_DAC_Start_1kHz_1Vpp( void )
{
  HAL_TIM_Base_Start(&htim6); // Timer Runs with 100kHz / 1kHz = 100 Datapoints per Periode
#ifdef DAC_12Bit
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*)au16Sinus1kHz, sizeof(au16Sinus1kHz)/2, DAC_ALIGN_12B_R);
#else  
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*)au8Sinus1kHz, sizeof(au8Sinus1kHz), DAC_ALIGN_8B_R);
#endif  
  
}

void HW_DAC_Stop( void )
{
  HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
  HAL_TIM_Base_Stop(&htim6);
}

/** 
* @brief  Init the DAC.
* @param  None
* @return None
*/
void HW_DAC_Init(void (* XferCpltCallback)(void) )
{
  DAC_ChannelConfTypeDef sConfig;

  XferCpltCB = XferCpltCallback;
    /**DAC Initialization 
    */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

    /**DAC channel OUT2 config 
    */
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/** 
* @brief  DeInit the DAC.
* @param  None
* @return None
*/
void HW_DAC_DeInit(void)
{
  HAL_DAC_DeInit(&hdac);
}

void HAL_DAC_MspInit(DAC_HandleTypeDef* dacHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(dacHandle->Instance==DAC)
  {
    HW_TIM6_Init(DAC_DMA_TIMER_FREQ_Hz); // 200kHz (DAC Settling Time 5us)    
    
    /* DAC clock enable */
    __HAL_RCC_DAC_CLK_ENABLE();
  
    /**DAC GPIO Configuration    
    PA5     ------> DAC_OUT2 
    */
    GPIO_InitStruct.Pin = IMP_DAC_SIGNAL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(IMP_DAC_SIGNAL_GPIO_Port, &GPIO_InitStruct);

    /* DAC DMA Init */
    __HAL_RCC_DMA1_CLK_ENABLE();
    /* DAC_CH2 Init */
    hdma_dac_ch2.Instance = DMA1_Channel3;
    hdma_dac_ch2.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_dac_ch2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dac_ch2.Init.MemInc = DMA_MINC_ENABLE;
  #ifdef DAC_12Bit
    hdma_dac_ch2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dac_ch2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  #else
    hdma_dac_ch2.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_dac_ch2.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  #endif
    hdma_dac_ch2.Init.Mode = DMA_CIRCULAR;
    hdma_dac_ch2.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    
    if (HAL_DMA_Init(&hdma_dac_ch2) != HAL_OK)
    {
      Error_Handler(); // _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(dacHandle,DMA_Handle2,hdma_dac_ch2);
    
  /* Enable the DMA1_Channel3 IRQ Channel */
    __HAL_DMA_DISABLE_IT( &hdma_dac_ch2, DMA_IT_HT); // Disable Half transfer complete IRQ
    HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);         
  }
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef* dacHandle)
{

  if(dacHandle->Instance==DAC)
  {
    
    /* Peripheral clock disable */
    __HAL_RCC_DAC_CLK_DISABLE();
  
    /**DAC GPIO Configuration    
    PA5     ------> DAC_OUT2 
    */
    HAL_GPIO_DeInit(IMP_DAC_SIGNAL_GPIO_Port, IMP_DAC_SIGNAL_Pin);

    /* DAC DMA DeInit */
    HAL_DMA_DeInit(dacHandle->DMA_Handle2);

    HW_TIM6_DeInit();
  }
} 

/*****END OF FILE****/
