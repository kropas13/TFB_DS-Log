/*******************************************************************************
* File Name: RTD.c
*******************************************************************************/

#include "RTD.h"

/***************************************
*  Customizer Generated Defines
***************************************/

#define     RTD_ORDER_POS   (0x5u)
// #define     RTD_POS_INPUT_SCALE   (12)
#define     RTD_POS_COEFF_SCALE   (11)
#define     RTD_ORDER_NEG   (0x4u)
// #define     RTD_NEG_INPUT_SCALE   (12)
#define     RTD_NEG_COEFF_SCALE   (11)

/***************************************
*           API Constants
***************************************/

/* Resistance value at 0 degrees C in milliohms */
#define RTD_ZERO_VAL_PT100             (100000lu)
#define RTD_ZERO_VAL_PT1000            (1000000lu)

#define RTD_FIRST_EL_MAS               (0u)
#define RTD_24BIT_SHIFTING             (24u)
#define RTD_16BIT_SHIFTING             (16u)
#define RTD_8BIT_SHIFTING              (8u)
#define RTD_24BIT_CUTTING              (0xFFFFFFu)
#define RTD_16BIT_CUTTING              (0xFFFFu)
#define RTD_8BIT_CUTTING               (0xFFu)
#define RTD_IN_NORMALIZATION           (14)
#define RTD_IN_FLOAT_NORMALIZATION     (1000u)
#define RTD_OUT_FLOAT_NORMALIZATION    (100u)

#if(!RTD_CALC_FLOAT)


    /*******************************************************************************
    * Function Name: RTD_MultShift24
    ********************************************************************************
    *
    * Summary:
    *  Performs the math function (op1 * op2) >> 24 using 64 bit arithmetic without
    *  any loss of precision and without overflow.
    *
    * Parameters:
    *  op1: Signed 32-bit operand
    *  op2: Unsigned 24-bit operand
    *
    * Return:
    *  Signed 32-bit result of the math calculation
    *
    *******************************************************************************/
    int32_t RTD_MultShift24(int32_t i32Op1, uint32_t i32Op2) 
    {
        int64_t i64Result=0;

        i64Result = (int64_t)i32Op1 * (int64_t)i32Op2;
        
        if (i64Result < 0)
        {
            i64Result = -i64Result;
            i64Result = (int32_t)((uint32_t)((uint64_t)i64Result >> RTD_24BIT_SHIFTING));
            i64Result = -i64Result;
        }
        else
        {
            i64Result = (int32_t)((uint32_t)((uint64_t)i64Result >> RTD_24BIT_SHIFTING));
        }
        return (i64Result);
    }
#endif /* End (!RTD_CALC_FLOAT) */


/*******************************************************************************
* Function Name: RTD_GetTemperature
********************************************************************************
*
* Summary:
*  Calculates the temperature from RTD resistance.
*
* Parameters:
*  ui32Res: Resistance in milliohms.
*
* Return:
*  Temperature in 1/100ths degrees C.
*
*******************************************************************************/
int32_t RTD_GetTemperature(uint32_t ui32Res, teRTD eTyp) 

#if (RTD_CALC_FLOAT)
{

    /***************************************
    *  Customizer Generated Coefficients
    ***************************************/       
    const float aafRTD_coeffPos[RTD_CNT][RTD_ORDER_POS] = {
      { -24579.95, 0.2359961,  9.954889E-08, -3.085104E-14, 1.756593E-19 }, // PT100 
      { -24579.95, 0.0235996,  9.954889E-10, -3.085103E-17, 1.756593E-23 }  // PT1000
    };   
      
    const float aafRTD_coeffNeg[RTD_CNT][RTD_ORDER_NEG] = {
        { -24208.89, 0.2227501,  2.520354E-07, -5.877933E-13 }, // PT100
        { -24208.89, 0.02227501, 2.520355E-09, -5.877933E-16 }  // PT1000
    };

    uint8_t ui8Idx=0u;
    float fResNorm=0.0f;
    float fTemp=0.0f;

    fResNorm = (float)ui32Res;

    if (((eTyp == RTD_PT100) && (ui32Res > RTD_ZERO_VAL_PT100)) ||
        ((eTyp == RTD_PT1000) && (ui32Res > RTD_ZERO_VAL_PT1000)))
    {
         /* Temperature above 0 degrees C */
        for (ui8Idx = RTD_ORDER_POS - 1u; ui8Idx > 0u; ui8Idx--)
        {
            fTemp = (aafRTD_coeffPos[eTyp][ui8Idx] + fTemp) * fResNorm;
        }
        fTemp = fTemp + aafRTD_coeffPos[eTyp][RTD_FIRST_EL_MAS];
    }
    else
    {
        /* Temperature below 0 degrees C */
        for (ui8Idx = RTD_ORDER_NEG - 1u; ui8Idx > 0u; ui8Idx--)
        {
            fTemp = (aafRTD_coeffNeg[eTyp][ui8Idx] + fTemp) * fResNorm;
        }
        fTemp = fTemp + aafRTD_coeffNeg[eTyp][RTD_FIRST_EL_MAS];
    }
    return ((int32_t)(fTemp));
}
#else
{
    /***************************************
    *  Customizer Generated Coefficients
    ***************************************/
    const int32_t aai32RTD_coeffPos[RTD_CNT][RTD_ORDER_POS] = { 
        { -50339727, 253398830, 56041043, -9105619, 27181954 }, // PT100
        { -50339727, 202719064, 35866267, -4662077, 11133728 }  // PT1000
    };       
    const int32_t aai32RTD_coeffNeg[RTD_CNT][RTD_ORDER_NEG] = {
        { -49579803, 239176079, 141883352, -173485967 }, // PT100
        { -49579803, 191340863,  90805345,  -88824815 }  // PT1000
    };

    const int8_t ai8RTD_inputScale[RTD_CNT]= {9,12};
         
    uint8_t ui8Idx;
    int32_t i32Temp=0;   

    if (((eTyp == RTD_PT100) && (ui32Res > RTD_ZERO_VAL_PT100)) ||
        ((eTyp == RTD_PT1000) && (ui32Res > RTD_ZERO_VAL_PT1000)))
    {
         /* Temperature above 0 degrees C */
        ui32Res = ui32Res << (RTD_IN_NORMALIZATION - ai8RTD_inputScale[eTyp]);

        for (ui8Idx = RTD_ORDER_POS - 1u; ui8Idx > 0u; ui8Idx--)
        {
            i32Temp = RTD_MultShift24((aai32RTD_coeffPos[eTyp][ui8Idx] + i32Temp), ui32Res);
        }
        i32Temp = (int32_t)((uint32_t)((uint64_t)(int32_t)(i32Temp + aai32RTD_coeffPos[eTyp][RTD_FIRST_EL_MAS]) >> 
                       RTD_POS_COEFF_SCALE));
    }

    else
    {
        /* Temperature below 0 degrees C */
        ui32Res = ui32Res << (RTD_IN_NORMALIZATION - ai8RTD_inputScale[eTyp]);

        for (ui8Idx = RTD_ORDER_NEG - 1u; ui8Idx > 0u; ui8Idx--)
        {
            i32Temp = RTD_MultShift24((aai32RTD_coeffNeg[eTyp][ui8Idx] + i32Temp), ui32Res);
        }

        i32Temp = (int32_t)((uint32_t)((uint64_t)(int32_t)(i32Temp + aai32RTD_coeffNeg[eTyp][RTD_FIRST_EL_MAS]) >> 
                       RTD_NEG_COEFF_SCALE));
    }
    return (i32Temp);
}
#endif /* End RTD_CALC_FLOAT */

/* [] END OF FILE */
