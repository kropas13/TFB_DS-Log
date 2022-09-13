/**============================================================================
* @file      hw_adc.c
* @date      2016-12-09
*
* @author    A. Ramirez
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     initialization and definition of adc instance for this project
*            
* VERSION:   
* 
* V0.01      2016-12-09-Ra      Create File 			
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "hw.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*!
 * Flag to indicate if the ADC is Initialized
 */
static bool bADC_SwitchOffHSI = false;

/*!
 * adc handle variable
 */
ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;

/* Private function prototypes -----------------------------------------------*/
/* Exported functions ---------------------------------------------------------*/
/**
  * @brief This function initializes the ADC
  * @param none
  * @retval none
  */
void HW_ADC1_SingleConv_Init(  void )
{
  /* Enable HSI (ADC uses HSI Clock) */
  if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET)
  {
    bADC_SwitchOffHSI = true; 
    
    __HAL_RCC_HSI_ENABLE();
    /* Wait till HSI is ready */
    while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET)
    {
    }
  } 
  
  hadc.Instance  = ADC1;
  hadc.Init.ClockPrescaler       = ADC_CLOCK_ASYNC_DIV2; // 16MHz/2=8MHz goes also down to 1.8V
  hadc.Init.Resolution           = ADC_RESOLUTION_12B;
  hadc.Init.ScanConvMode         = ADC_SCAN_DISABLE;
  hadc.Init.NbrOfConversion      = 1; /*not required since ADC_SCAN_DISABLE */
  hadc.Init.ContinuousConvMode   = DISABLE;		
  hadc.Init.ExternalTrigConv     = ADC_EXTERNALTRIGCONVEDGE_NONE; 
  hadc.Init.LowPowerAutoWait     = ADC_AUTOWAIT_UNTIL_DATA_READ; /* ADC_DelaySelectionConfig( ADC1, ADC_DelayLength_Freeze ); */
  hadc.Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_IDLE_DELAY_PHASES;

  __HAL_RCC_ADC1_CLK_ENABLE();   
  __NOP( );
  __NOP( );
  
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief This function De-initializes the ADC
  * @param none
  * @retval none
  */
void HW_ADC1_Single_DeInit( void )
{
  /* Disable adc block after being used */
  __HAL_ADC_DISABLE( &hadc);
  __HAL_RCC_ADC1_CLK_DISABLE();
  
    if (bADC_SwitchOffHSI)
    {
      bADC_SwitchOffHSI = false;
      __HAL_RCC_HSI_DISABLE();  
    }     
}

/**
  * @brief This function reads an adc channel in block mode
  * @param Channel
  * @retval Value
  */
uint16_t HW_ADC1_Read( uint32_t Channel, uint16_t AverageCnt)
{

  ADC_ChannelConfTypeDef adcConf;
  uint16_t adcData = 0;
  uint32_t adcSum = 0;
  uint16_t adcCnt;     
  
  HW_ADC1_SingleConv_Init();	     	   
    
  /* wait the the Vrefint used by adc is set*/
  while (__HAL_PWR_GET_FLAG(PWR_FLAG_VREFINTRDY) == RESET) {};
	      
  /* configure adc channel */
  adcConf.Channel = Channel;
  adcConf.Rank = ADC_REGULAR_RANK_1; 
  adcConf.SamplingTime = ADC_SAMPLETIME_192CYCLES;
	
  if (HAL_ADC_ConfigChannel(&hadc, &adcConf) != HAL_OK) 
  {
    Error_Handler();
  }  	

  if (AverageCnt < 1)
    AverageCnt = 1;

  for (adcCnt=0; adcCnt < AverageCnt; adcCnt++)
  {
    /* Start the conversion process */
    HAL_ADC_Start( &hadc);  
    
    /* Wait for the end of conversion */
    HAL_ADC_PollForConversion( &hadc, HAL_MAX_DELAY );      
    
    /* Get the converted value of regular channel */
    adcSum += HAL_ADC_GetValue( &hadc);  
  }
  
  adcData = adcSum / adcCnt; 
  
  HW_ADC1_Single_DeInit();

  return adcData;
}

void HW_ADC_MultipleConv_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;    
 
  /* Enable HSI (ADC uses HSI Clock) */
  if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET)
  {
    bADC_SwitchOffHSI = true; 
    
    __HAL_RCC_HSI_ENABLE();
    /* Wait till HSI is ready */
    while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET)
    {
    }
  }    

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = ADC_AUTOWAIT_DISABLE;
  hadc.Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
  hadc.Init.ChannelsBank = ADC_CHANNELS_BANK_A;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.NbrOfConversion = 3;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  
	__HAL_RCC_ADC1_CLK_ENABLE();   
  __NOP( );
  __NOP( );
    
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_4CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* ADC1 DMA Init */
  /* ADC Init */
  hdma_adc.Instance = DMA1_Channel1;
  hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
  hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  hdma_adc.Init.Mode = DMA_NORMAL; // DMA_CIRCULAR;
  hdma_adc.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  if (HAL_DMA_Init(&hdma_adc) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_LINKDMA(&hadc,DMA_Handle,hdma_adc);  
}

void HW_ADC_MultipleConv_DeInit( void )
{
    __HAL_ADC_DISABLE( &hadc);  
    __HAL_RCC_ADC1_CLK_DISABLE();  

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(hadc.DMA_Handle);
  
    if (bADC_SwitchOffHSI)
    {
      bADC_SwitchOffHSI = false;
      __HAL_RCC_HSI_DISABLE();  
    }
}




/*****END OF FILE****/
