 /******************************************************************************
  * @file    vcom.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    15-September-2016
  * @brief   manages virtual com port
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  
#include "hw.h"
#include "vcom.h"
#if USE_VCOM_ON_RS485_USART1 == 1 
  #include "RS485.h"
#endif
#include "bsp.h"
#include <stdarg.h>


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BUFSIZE 500

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint16_t iw;
static char buff[BUFSIZE];

/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
//------------------------------------------------------------------------------
/**
* @brief sends string with UART
*
* @details
*
* @param character to be sent, as defined by common PRINTF command  
*
* @retval None	
*/
//------------------------------------------------------------------------------
void vcom_UART_Send(const char *format, ... )
{
  va_list args;
  va_start(args, format);
  
  /*convert into string at buff[0] of length iw*/
  iw = vsnprintf(&buff[0],sizeof(buff), format, args);

#if USE_VCOM_ON_RS485_USART1 == 1 
    RS485_TxDataPolled( (uint8_t *)&buff[0], iw, 300);
#elif USE_VCOM_ON_X403_BT_USART3 == 1
    HAL_UART_Transmit(&huart3,(uint8_t *)&buff[0], iw, 300);
#endif  
  va_end(args);
}


//------------------------------------------------------------------------------
/**
* @brief transforms an uint into its binary representation string
*
* @details
*
* @param value: value to be transformed
*        result: string where result should be saved 
*
* @retval None	
*/
//------------------------------------------------------------------------------
void Uint2BinStr(uint16_t value, char* result) {
  char* ptr = result;
	int tmp_value;		
	do {
	  tmp_value = value;
		value /= 2;
		*ptr++ = "01" [(tmp_value - value * 2)];
	} while ( value );
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
