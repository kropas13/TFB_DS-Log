/*******************************************************************************
* File Name: RTD.h
*******************************************************************************/

#if !defined(RTD_CALC_RTD_H)
#define RTD_CALC_RTD_H

#include "stm32l1xx.h"

/***************************************
*   Conditional Compilation Parameters
****************************************/
#define RTD_CALC_FLOAT 0 // Float calculation needs about double the time by an ARM Cortex M3 (about 25us@32MHz and Integer needs about 13us@32MHz)
// The code size is about the same for float calculating or integer by an ARM Cortex M3 (Float calc. is 40Bytes smaller)

/***************************************
*    Enumerated Types and Parameters
***************************************/
/* Enumerated Types RTDType, Used in parameter RTDType */
typedef enum 
{
    RTD_PT100  = 0,
    RTD_PT1000 = 1,
    RTD_CNT  
} teRTD;    




/***************************************
*        Function Prototypes
***************************************/

int32_t RTD_GetTemperature(uint32_t ui32Res, teRTD eTyp) ;

#if (!RTD_CALC_FLOAT)
    int32_t RTD_MultShift24(int32_t i32Op1, uint32_t i32Op2) ;
#endif /* (!RTD_CALC_FLOAT) */


#endif /* RTD_CALC_RTD_H */

/* [] END OF FILE */
