/*******************************************************************************
* File Name: Thermocouple.h
*******************************************************************************/

#if !defined(THERMOCOUPLE_CALC_Thermocouple_H)
#define THERMOCOUPLE_CALC_Thermocouple_H

#include "stm32l1xx.h"

/***************************************
*   Conditional Compilation Parameters
****************************************/

#define TC_CALC_FLOAT 0 // Float calculation needs about double the time by an ARM Cortex M3 (about 62us@32MHz and Integer needs about 29us@32MHz for the K-Type)
// The float calculating needs about 1kByte lesser code size than the integer calulation for an ARM Cortex M3

/***************************************
*       Enum Types
***************************************/

/* ThermocoupleTypes */
typedef enum 
{
    THERMOCOUPLE_B   = 0,  // 100..250*C max. 0.45°C calc. error; 251..1820°C  max. 0.20°C calc. error
    THERMOCOUPLE_C   = 1, 
    THERMOCOUPLE_D   = 2,
    THERMOCOUPLE_E   = 3,  // -250..-200°C max. -0.91°C calc. error, -199..1000°C max. -0.08°C calc. error
    THERMOCOUPLE_J   = 4,  // -210..1200°C max. 0.17°C calc. error
    THERMOCOUPLE_K   = 5,  // -200..1372°C max. 0.06°C calc. error
    THERMOCOUPLE_L   = 6,
    THERMOCOUPLE_N   = 7,  // -250..-200°C max. 1.27°C calc. error, -199..1300°C max. 0.05°C calc. error  
    THERMOCOUPLE_R   = 8,  // -50..1768°C max. -0.14°C calc. error
    THERMOCOUPLE_S   = 9,  // -50..1768°C max. 0.12°C calc. error
    THERMOCOUPLE_T   = 10, // -250..-200°C max. 1.48°C calc. error, -199..400°C max. 0.05°C calc. error
    THERMOCOUPLE_U   = 11,    
    THERMOCOUPLE_CNT
} teTHERMOCOUPLE;                                        


/***************************************
*        Function Prototypes
***************************************/

int32_t Thermocouple_GetTemperature(int32_t voltage, teTHERMOCOUPLE eTyp) ;
int32_t Thermocouple_GetVoltage(int32_t temperature, teTHERMOCOUPLE eTyp) ;

#if (!TC_CALC_FLOAT)
    int32_t Thermocouple_MultShift24(int32_t i32Op1, int32_t i32Op2) ;
#endif /* (!TC_CALC_FLOAT) */

#endif /* THERMOCOUPLE_CALC_Thermocouple_H */

/* [] END OF FILE */
