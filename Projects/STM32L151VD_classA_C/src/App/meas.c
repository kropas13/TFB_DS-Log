/**============================================================================
* @file      meas.c
* @date      2016-11-10
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     measurement functions used by loraModem and lora itself
*            
* VERSION:   
* 
* V0.01      2018-07-05-Kd      Create File
* V1.02      2018-10-29-Kd      REV01 ADG788 IMP_SWx changed to SxA Output (normally open)
*
*============================================================================*/
/* Includes ------------------------------------------------------------------*/
#include "hw.h"
#include "bsp.h"
#include "vcom.h"
#include "Si705x.h"
#include "ads1248.h"
#include "config.h"
#include "meas.h"

#include <math.h>

/* Private define --------------------------------------------------------------*/

// #define USE_PHASE_HW_COMP  // When not activated -> calculate Phase shift by SW from ADC DMA data

//ADC max value
#define PDDADC_FS     4096

//ADC Vbat measurement constants
/* Internal voltage reference, parameter VREFINT_CAL */
#define VREFINT_CAL         ((uint16_t*) ((uint32_t) 0x1FF800F8)) // mV 
/* Factory calibration voltage @ 30°C +/- 5°C */
#define VDDA_VREFINT_CAL    ((uint32_t) 3000) //(+/- 10mV)   

//Battery level definitions SL-2780 Li/SOCI2 (kann nicht schlau detektiert werden)
#define BAT_FULL_mV          3600
#define BAT_EMPTY_mV         2400 // depends on the battery type
#define BAT_FULL_LEVEL        254
#define BAT_EMPTY_LEVEL         1

#define MEAS_PERIOD_CNT                   4
#define MEAS_ADC_SAMPLES_PER_PERIOD_CNT   333
#define ADC_CAPTURE_CNT                   (MEAS_PERIOD_CNT * (MEAS_ADC_SAMPLES_PER_PERIOD_CNT*105/100)) // inkl. 5% reserve 16MHz ADC Clock is the HSI Toleranz (max. +-4%), 1ms Period / 3us = 333.3 meassurements when have 3 Channels@4+12Tcycle

#define CALC_AVG_FACT             16  // Die Average Werte um Faktor 16 anheben (4 Bits) um die höhere Genauigkeit mehrere Messwerte auszunutzen (Max. 16)
#define CALC_AVG_FACT_SQUAREROOT   4  // Die Wuzel von CALC_AVG_FACT

#define DEF_CALC_IMP_FLOAT       // When not defined calc integer SquareRoute
#define IMP_I2_AVG_MEAS_CNT_MAX        8 // Max. 8 measures on I2

/* Private typedef -------------------------------------------------------------*/


/* Private macro ---------------------------------------------------------------*/
typedef enum {
  ADC_DMA_CH_First = 0,
  ADC_DMA_CH_U  = 0,  // Invertiert
  ADC_DMA_CH_I1,
  ADC_DMA_CH_I2,      // I1 * 101  
  ADC_DMA_CH_Cnt // 3 * (1400 Bytes * 2) = 8400 Bytes
} teADC_DMA_CH;

/* Private variables -----------------------------------------------------------*/
t_sMEASExtSenseData sMEASExtSenseData;
t_sBSP_ShiftReg_1   sShiftReg_1;
t_sBSP_ShiftReg_2   sShiftReg_2;
volatile uint8_t    g_u8Meas_ADC_Cnt;
volatile uint32_t   g_u32MeasADC_DMA_Idx;
#ifdef USE_PHASE_HW_COMP
volatile uint32_t   g_au32MeasUCompCnt[MEAS_PERIOD_CNT];
volatile uint32_t   g_au32MeasICompCnt[MEAS_PERIOD_CNT];
#endif
int16_t             g_ai16MeasADC_DMA[ADC_CAPTURE_CNT*ADC_DMA_CH_Cnt];
int16_t             g_i16MeasADC_Phase_U_I1_100mg; // +- 900
int16_t             g_i16MeasADC_Phase_U_I2_100mg; // +- 900

#ifdef DEF_CALC_IMP_FLOAT
 float              g_afMeasADC[ADC_DMA_CH_Cnt];
#else
  uint16_t          g_aui16MeasADC[ADC_DMA_CH_Cnt];
#endif


/* ext. functions ------------------------------------------------------*/
void appHandleUI(void);

/* Private function ------------------------------------------------------------*/
/**
 * @brief   Integer square root for RMS
 * @param   sqrtAvg  (sum(x squared) /  count)
 * @retval                approximate square root
 *
 * Approximate integer square root, used for RMS calculations.
*/
#ifndef DEF_CALC_IMP_FLOAT
static unsigned int sqrtI( unsigned long sqrtArg )
{
    unsigned int answer, x;
    unsigned long temp;
  
    if ( sqrtArg == 0 ) return 0;       // undefined result
    if ( sqrtArg == 1 ) return 1;       // identity
    answer = 0;             // integer square root
    for( x=0x8000; x>0; x=x>>1 )
    {               // 16 bit shift
        answer |= x;            // possible bit in root
        temp = answer * answer; // fast unsigned multiply
        if (temp == sqrtArg)  break;    // exact, found it
        if (temp > sqrtArg) answer ^= x;    // too large, reverse bit
    }
    return answer;          // approximate root
}
#endif 
 
void CalcADC_DMA(uint16_t ui16MeasPeriodCnt)
{
  uint32_t      ui32Result;
  teADC_DMA_CH  eAdcDmaCh;
  int16_t*      pi16Value;
  int16_t       i16Value;
  uint8_t       ui8PeriodIdx;
  uint16_t      ui16IdxInPeriodAndChannel, ui16IdxCh;  
  uint16_t      ui32Idx;
  uint32_t      ui32ADCAverage_Sum[ADC_DMA_CH_Cnt] = {0}; 
  uint16_t      ui16ADCAverage[ADC_DMA_CH_Cnt]; 
  uint64_t      ui64ADCSquare_Sum[ADC_DMA_CH_Cnt] = {0}; 
  uint32_t      ui32ADC_Cnt = 0;
  uint16_t      ui16MeasADC_SamplesPerPeriodAndChannel, ui16MeasADC_SamplesPerPeriodAndChannel_270;
  uint16_t      au16MeasADC_UCompPhase_Idx[MEAS_PERIOD_CNT] = {0};  // 0..360 Wingelgrad
  uint16_t      au16MeasADC_I1CompPhase_Idx[MEAS_PERIOD_CNT] = {0}; // 0..360 Wingelgrad  
  uint16_t      au16MeasADC_I2CompPhase_Idx[MEAS_PERIOD_CNT] = {0}; // 0..360 Wingelgrad  
  bool          bMeasADC_CompPhaseInvert[ADC_DMA_CH_Cnt] = {0};
  uint16_t      au16MeasADC_CompPhaseInvert_Idx[ADC_DMA_CH_Cnt];
  int32_t       i32AveragePhase_U_I1, i32AveragePhase_U_I2;
#ifdef DEF_CALC_IMP_FLOAT  
  double        dResult;
#endif  
  
  pi16Value = g_ai16MeasADC_DMA;
  g_u32MeasADC_DMA_Idx -= g_u32MeasADC_DMA_Idx%ADC_DMA_CH_Cnt; 
  ui16MeasADC_SamplesPerPeriodAndChannel = g_u32MeasADC_DMA_Idx / ui16MeasPeriodCnt / ADC_DMA_CH_Cnt;
  ui16MeasADC_SamplesPerPeriodAndChannel_270 = ui16MeasADC_SamplesPerPeriodAndChannel/4*3;

  // Mittelwert summieren
  for ( eAdcDmaCh=ADC_DMA_CH_First, ui32Idx=0; ui32Idx<g_u32MeasADC_DMA_Idx; ui32Idx++, pi16Value++)
  {
    if ((uint16_t)(*pi16Value) <= 0x0FFF)
    {       
      ui32ADCAverage_Sum[eAdcDmaCh] += *pi16Value; 
      if (++eAdcDmaCh >= ADC_DMA_CH_Cnt)
      {
        eAdcDmaCh = ADC_DMA_CH_First;
        ui32ADC_Cnt++;
      }
    }
  }
  // Mittelwert berechnen 
  for ( eAdcDmaCh=ADC_DMA_CH_First; eAdcDmaCh<ADC_DMA_CH_Cnt; eAdcDmaCh++)
      ui16ADCAverage[eAdcDmaCh] = (uint16_t)(ui32ADCAverage_Sum[eAdcDmaCh] * CALC_AVG_FACT / ui32ADC_Cnt);      
  
  // Nulldurchgänge erkennen für Phasenverschiebung, Quadrieren und Summieren  
  pi16Value = g_ai16MeasADC_DMA;
  for ( eAdcDmaCh=ADC_DMA_CH_First, ui32Idx=0, ui16IdxCh=0, ui16IdxInPeriodAndChannel=0, ui8PeriodIdx=0; ui32Idx<g_u32MeasADC_DMA_Idx; ui32Idx++, pi16Value++)
  {
    if ((uint16_t)(*pi16Value) <= 0x0FFF)
    {            
      i16Value  = (int16_t)((int32_t)*pi16Value * CALC_AVG_FACT - ui16ADCAverage[eAdcDmaCh]);
      
      // Nulldurchgänge
      // 3 Kanäle mit je ca. 333 Samples je Periode (1ms) und 4 Perioden sind im DMA aufgezeichnet
      // Die ersten 30° verwerfen (einschwingen)
      if ((ui8PeriodIdx < MEAS_PERIOD_CNT) && (ui16IdxCh > ui16MeasADC_SamplesPerPeriodAndChannel/12))
      {
        if (eAdcDmaCh == ADC_DMA_CH_U)
        {
          // 180° Durchgang -> von + nach - (Signal invertiert)
          if (!bMeasADC_CompPhaseInvert[eAdcDmaCh] && (i16Value < 0))
          {
            bMeasADC_CompPhaseInvert[eAdcDmaCh] = true;
            au16MeasADC_UCompPhase_Idx[ui8PeriodIdx] = ui16IdxInPeriodAndChannel;
            au16MeasADC_CompPhaseInvert_Idx[eAdcDmaCh] = ui16MeasADC_SamplesPerPeriodAndChannel_270;
          }
          else if (bMeasADC_CompPhaseInvert[eAdcDmaCh] && (!--au16MeasADC_CompPhaseInvert_Idx[eAdcDmaCh]))
            bMeasADC_CompPhaseInvert[eAdcDmaCh] = false;          
        }
        else if (eAdcDmaCh == ADC_DMA_CH_I1)
        {
          // 180° Durchgang -> von - nach +
          if (!bMeasADC_CompPhaseInvert[eAdcDmaCh] && (i16Value > 0))
          {
            bMeasADC_CompPhaseInvert[eAdcDmaCh] = true;            
            au16MeasADC_I1CompPhase_Idx[ui8PeriodIdx] = ui16IdxInPeriodAndChannel;
            au16MeasADC_CompPhaseInvert_Idx[eAdcDmaCh] = ui16MeasADC_SamplesPerPeriodAndChannel_270;
          }
          else if (bMeasADC_CompPhaseInvert[eAdcDmaCh] && (!--au16MeasADC_CompPhaseInvert_Idx[eAdcDmaCh]))
            bMeasADC_CompPhaseInvert[eAdcDmaCh] = false;          
        }
        else
        {
          // 180° Durchgang -> von - nach +
          if (!bMeasADC_CompPhaseInvert[eAdcDmaCh] && (i16Value > 0))
          {
            bMeasADC_CompPhaseInvert[eAdcDmaCh] = true;            
            au16MeasADC_I2CompPhase_Idx[ui8PeriodIdx] = ui16IdxInPeriodAndChannel;
            au16MeasADC_CompPhaseInvert_Idx[eAdcDmaCh] = ui16MeasADC_SamplesPerPeriodAndChannel_270;
          }
          else if (bMeasADC_CompPhaseInvert[eAdcDmaCh] && (!--au16MeasADC_CompPhaseInvert_Idx[eAdcDmaCh]))
            bMeasADC_CompPhaseInvert[eAdcDmaCh] = false;          
        }
      }
      // Quadrieren
      ui32Result = (uint32_t)((int32_t)i16Value * (int32_t)i16Value);
      // Summieren
      ui64ADCSquare_Sum[eAdcDmaCh] += ui32Result; 
      
      if (++eAdcDmaCh >= ADC_DMA_CH_Cnt)
      {
        eAdcDmaCh = ADC_DMA_CH_First;
        ui16IdxCh++;
        if (++ui16IdxInPeriodAndChannel > ui16MeasADC_SamplesPerPeriodAndChannel)
        {
          ui16IdxInPeriodAndChannel = 0;
          ui8PeriodIdx++;
        }        
      }        
    }
  }
  // RMS Wurzel ziehen
  for ( eAdcDmaCh=ADC_DMA_CH_First; eAdcDmaCh<ADC_DMA_CH_Cnt; eAdcDmaCh++)
  {     
      // RMS Wurzel ziehen
#ifdef DEF_CALC_IMP_FLOAT           
      dResult = (double)ui64ADCSquare_Sum[eAdcDmaCh] / ui32ADC_Cnt / CALC_AVG_FACT;
      g_afMeasADC[eAdcDmaCh] = (float)sqrt(dResult) / CALC_AVG_FACT_SQUAREROOT; // 2024-03-15-Kd new / CALC_AVG_FACT_SQUAREROOT (da wir die CALC_AVG_FACT auch quadriert haben)
#else
      ui32Result = ui64ADCSquare_Sum[eAdcDmaCh] / ui32ADC_Cnt / CALC_AVG_FACT; 
      g_aui16MeasADC[eAdcDmaCh] = sqrtI(ui32Result) / CALC_AVG_FACT_SQUAREROOT; // 2024-03-15-Kd new / CALC_AVG_FACT_SQUAREROOT    
#endif    
  }
  
  // Phasenverschiebung berechnen  
  for (ui32Idx=0,i32AveragePhase_U_I1=0, i32AveragePhase_U_I2=0; ui32Idx<ui8PeriodIdx; ui32Idx++)
  { // 2020-06-15-Kd V1.08 gem. Herr Kronenberg Phasenwinkel hat falsches Vorzeichen
    i32AveragePhase_U_I1 += (int32_t)au16MeasADC_I1CompPhase_Idx[ui8PeriodIdx] - (int32_t)au16MeasADC_UCompPhase_Idx[ui8PeriodIdx]; // 2020-06-15-Kd V1.08 changed polarity -> old au16MeasADC_UCompPhase_Idx[ui8PeriodIdx] - au16MeasADC_I1CompPhase_Idx[ui8PeriodIdx];
    i32AveragePhase_U_I2 += (int32_t)au16MeasADC_I2CompPhase_Idx[ui8PeriodIdx] - (int32_t)au16MeasADC_UCompPhase_Idx[ui8PeriodIdx]; // 2020-06-15-Kd V1.08 changed polarity -> old au16MeasADC_UCompPhase_Idx[ui8PeriodIdx] - au16MeasADC_I2CompPhase_Idx[ui8PeriodIdx];
  }
  // Mitteln und von Index 333 Messungen pro Periode nach Winkelgrad berechnen
  i16Value = i32AveragePhase_U_I1 * 3600 / ui16MeasADC_SamplesPerPeriodAndChannel / ui8PeriodIdx ; // 2020-06-15-Kd V1.08 neu mit [0.1°] Auflösung rechnen
  if (i16Value > 900)
      g_i16MeasADC_Phase_U_I1_100mg = 900;
  else if (i16Value < -900)
      g_i16MeasADC_Phase_U_I1_100mg = -900;
  else
    g_i16MeasADC_Phase_U_I1_100mg = i16Value;
  i16Value = i32AveragePhase_U_I2 * 3600 / ui16MeasADC_SamplesPerPeriodAndChannel / ui8PeriodIdx;
  
  // 2024-03-18-Kd V1.11 ISL28130 Bandbreite von 400kHz und Gain 100 ergeben ein Phasenerror je nach AD8231 Gain noch etwas unterschiedlich
  switch (sShiftReg_2.eOP_GainSetupImpI)
  {
     case eOP_GAIN_1:
     case eOP_GAIN_2:
     case eOP_GAIN_4:
        i16Value -= 120;
     break;
     case eOP_GAIN_8:
        i16Value -= 100;
     break;        
     case eOP_GAIN_16:
        i16Value -= 60;
     break;  
     case eOP_GAIN_32:
        i16Value -= 40;
     break; 
     case eOP_GAIN_64:
        i16Value -= 10;
     break;      
}
  
  if (i16Value > 900)
      g_i16MeasADC_Phase_U_I2_100mg = 900;
  else if (i16Value < -900)
      g_i16MeasADC_Phase_U_I2_100mg = -900;
  else
    g_i16MeasADC_Phase_U_I2_100mg = i16Value;  
  
}


/* Exported functions ----------------------------------------------------------*/
// 2020-06-26-Kd V1.09 new (only measure configured channels)
//------------------------------------------------------------------------------
bool MEASGetExtADCSensors( t_eMEASExtSenseTempType aeMEASExtSenseTempType[EXT_SENSE_CHANNELS], int16_t i16ColdJunctionT_GC_100, uint32_t u32ExtPowerOnTickstart_ms,
    uint8_t  u8EnabledTemperatureChMsk, uint8_t  u8EnabledVoltageChMsk, uint8_t  u8EnabledCurrentChMsk, uint8_t  u8EnabledCorrVoltageChMsk, uint8_t  u8EnabledCorrCurrentChMsk)
{
  bool                          bOk = true;  
  bool                          bChannelOk;  
  uint8_t                       u8Ch, u8Msk;
  int32_t                       i32ADCConvData, i32ADCConvDataTemp;
  uint32_t                      u32ExtPowerOnSince_ms, u32ExtSensorWaitMeassure_ms;
  float                         fScaledValue;
  t_eADS1248_Board_MeasureTyp   eADS1248_MeasureTyp;  
  // tsADS1248_ChSysOffset         sADS1248_ChSysOffset;
  bool                          bScaling;
  bool                          bSysPowerOff = !BSP_IsSystemVccON();
	
	/* Turn on Si705x */	   
  if (bSysPowerOff)
  {
    BSP_SystemVcc(true);  // VCC System on  
    u32ExtPowerOnTickstart_ms = HAL_GetTick();
  }
  
  ADS1248_Init();  
  
  memset(&sShiftReg_1,0,sizeof(sShiftReg_1));
  BSP_ClearShiftRegister_1();
  memset(&sShiftReg_2,0,sizeof(sShiftReg_2));
  BSP_ClearShiftRegister_2();
    
// Sense Temp      
  if (u8EnabledTemperatureChMsk)
  {
    for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++,u8Msk<<=1)
    {    
      if ((u8EnabledTemperatureChMsk & u8Msk) == 0)
        continue;
      bScaling = true; 
      sShiftReg_1.bMuxTEMP_PT1000 = false;
      sShiftReg_1.ui8MuxTemp_Msk = u8Msk;
      switch (aeMEASExtSenseTempType[u8Ch])
      {
        case eMEAS_EXTSENSETEMPTYPE_PT1000_2Wire: eADS1248_MeasureTyp = eADS1248_MeasureTyp_PT1000_2Wire; sShiftReg_1.bMuxTEMP_PT1000 = true; break;
        case eMEAS_EXTSENSETEMPTYPE_PT1000_3Wire: eADS1248_MeasureTyp = eADS1248_MeasureTyp_PT1000_3Wire; sShiftReg_1.bMuxTEMP_PT1000 = true; bScaling = false; break;
        case eMEAS_EXTSENSETEMPTYPE_PT100_2Wire:  eADS1248_MeasureTyp = eADS1248_MeasureTyp_PT100_2Wire; break;
        case eMEAS_EXTSENSETEMPTYPE_PT100_3Wire:  eADS1248_MeasureTyp = eADS1248_MeasureTyp_PT100_3Wire; bScaling = false; break;
        case eMEAS_EXTSENSETEMPTYPE_TC_E:         eADS1248_MeasureTyp = eADS1248_MeasureTyp_ThermoCouple_E; break;
        case eMEAS_EXTSENSETEMPTYPE_TC_J:         eADS1248_MeasureTyp = eADS1248_MeasureTyp_ThermoCouple_J; break;
        case eMEAS_EXTSENSETEMPTYPE_TC_K:         eADS1248_MeasureTyp = eADS1248_MeasureTyp_ThermoCouple_K; break;
        case eMEAS_EXTSENSETEMPTYPE_TC_N:         eADS1248_MeasureTyp = eADS1248_MeasureTyp_ThermoCouple_N; break;
        case eMEAS_EXTSENSETEMPTYPE_TC_R:         eADS1248_MeasureTyp = eADS1248_MeasureTyp_ThermoCouple_R; break;
        case eMEAS_EXTSENSETEMPTYPE_TC_S:         eADS1248_MeasureTyp = eADS1248_MeasureTyp_ThermoCouple_S; break;      
        case eMEAS_EXTSENSETEMPTYPE_TC_T:         eADS1248_MeasureTyp = eADS1248_MeasureTyp_ThermoCouple_T; break;
      }     
      bChannelOk = false;      
      // 2019-03-07-Kd V1.07 new discharge of C707 over current PTxxxx (so the C707 will only be discharged when a PTxxxx is connected), simplified ADC communication 
      // and MUX switching compared to V1.06, the meassurement takes over all the same time.
      ADS1148_SetupMeasure( eADS1248_MeasureTyp_PT_Discharge_AIN, i16ColdJunctionT_GC_100); // 2019-03-06-Kd switch off current sources   
      BSP_SetShiftRegister_1( &sShiftReg_1); // connect next PTxxxx over MUX
      BSP_SleepDelayMs(9);  // Discharge of C707 over ext. PTxxxx (1k+5k6+5k6 Ohm -> Tau = 47nF * 12kOhm = 0.6ms)
      if (ADS1148_SetupMeasure( eADS1248_MeasureTyp, i16ColdJunctionT_GC_100 ))    
      {    
        BSP_SleepDelayMs(17); // 2019-03-06-Kd Settling Time >= V1.06 (Config 00 2-Wire       @PTxxxx 2-wire with 999 Ohm 3ms= 0.11gC  10ms= 0.08gC  20ms= 0.06gC  30ms=0.04gC  50ms=0.02gC)
                              //                                      (Config 01 2- or 3-Wire @PTxxxx 2-wire with 999 Ohm 3ms=-0.16gC  10ms=-0.18gC  20ms=-0.19gC)
        // Crosstalk from an open prior channel to an used channel: Config 00 2-Wire -> +0.15gC     Config 01 2-Wire or 3-Wire -> +0.10gC
        if (ADS1248_Measure( &i32ADCConvData, &fScaledValue, bScaling))
        {
          if ((aeMEASExtSenseTempType[u8Ch] == eMEAS_EXTSENSETEMPTYPE_PT1000_3Wire) || (aeMEASExtSenseTempType[u8Ch] == eMEAS_EXTSENSETEMPTYPE_PT100_3Wire))
          {
            if (aeMEASExtSenseTempType[u8Ch] == eMEAS_EXTSENSETEMPTYPE_PT1000_3Wire)
              eADS1248_MeasureTyp = eADS1248_MeasureTyp_PT1000_3Wire_Inv;
            else
              eADS1248_MeasureTyp = eADS1248_MeasureTyp_PT100_3Wire_Inv;
            if (ADS1148_SetupMeasure( eADS1248_MeasureTyp, i16ColdJunctionT_GC_100 ))    
            {            
              BSP_SleepDelayMs(10); // Settling Time 
              if (ADS1248_Measure( &i32ADCConvDataTemp, &fScaledValue, false))
              {
                i32ADCConvData = (i32ADCConvData + i32ADCConvDataTemp) / 2;
                fScaledValue = ADS1248_Board_Scale( i32ADCConvData, eADS1248_MeasureTyp, i16ColdJunctionT_GC_100);              
                bChannelOk = true;
              }
            }          
          }
          else
            bChannelOk = true;
        }        
      }    
      if (bChannelOk) 
      {
        // 2018-11-20-Kd neu ab V1.02 Vergleichstellentemperatur mittels Spannung in ADS1248_Board_Scale() einrechnen
        // alt Vergleichstellentemperatur mittels Temperatur einrechnen (nicht korrekt.) etwas ungenauer 
        // if (aeMEASExtSenseTempType[u8Ch] >= eMEAS_EXTSENSETEMPTYPE_TC_E)
        //  fScaledValue += (float)i16ColdJunctionT_GC_100/100;
        sMEASExtSenseData.fTemp_gC[u8Ch] = fScaledValue;    
      }
      else
        bOk = false;
    }
    memset(&sShiftReg_1,0,sizeof(sShiftReg_1));
    BSP_ClearShiftRegister_1();       
  }
  
// Corr U and I 
  if (u8EnabledCorrVoltageChMsk | u8EnabledCorrCurrentChMsk)  
  {
    bScaling = true;  
    for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++,u8Msk<<=1)
    {
      if (((u8EnabledCorrVoltageChMsk | u8EnabledCorrCurrentChMsk) & u8Msk) == 0)
        continue;
        
      sShiftReg_2.ui8MuxCorrU_Msk = u8Msk;            
      sShiftReg_2.ui8MuxCorrI_Msk = u8Msk;  
      BSP_SetShiftRegister_2( &sShiftReg_2);
      
      bChannelOk = true;
      while (1)
      {
        if (u8EnabledCorrVoltageChMsk & u8Msk)
        {
          if (!ADS1148_SetupMeasure( eADS1248_MeasureTyp_Corr_U, i16ColdJunctionT_GC_100 ))    
          {
            bChannelOk = false;
            break;
          }
          BSP_SleepDelayMs(2); // Settling Time
          if (!ADS1248_Measure( &i32ADCConvData, &fScaledValue, bScaling))
          { 
            bChannelOk = false;
            break;
          }
          sMEASExtSenseData.fCorrU_mV[u8Ch] = fScaledValue; 
        }
        else
          BSP_SleepDelayMs(50); // Same settling Time for current measurement
        if (u8EnabledCorrCurrentChMsk & u8Msk)
        {
          if (!ADS1148_SetupMeasure( eADS1248_MeasureTyp_Corr_I, i16ColdJunctionT_GC_100))    
          {
            bChannelOk = false;
            break;
          }                     
          BSP_SleepDelayMs(10); // Settling Time
          if (!ADS1248_Measure( &i32ADCConvData, &fScaledValue, bScaling))
          {
            bChannelOk = false;
            break;
          }              
          sMEASExtSenseData.fCorrI_uA[u8Ch] = fScaledValue;                                   
        }
        break;
      } // end while (1)
      if (!bChannelOk)
        bOk = false;
    }
    memset(&sShiftReg_2,0,sizeof(sShiftReg_2));
    BSP_ClearShiftRegister_2(); 
  }
  
// Sense U and I  (2018-10-26-Kd ab V1.02 neu am Schluss messen, damit ext. Sensoren mehr Vorlaufzeit haben)
  if (u8EnabledVoltageChMsk | u8EnabledCurrentChMsk)  
  {  
    // 2018-10-29-Kd V1.02 Vorlaufzeit abwarten
    u32ExtPowerOnSince_ms = HAL_GetTick() - u32ExtPowerOnTickstart_ms;    
    if (u32ExtPowerOnSince_ms < (1000 * (uint32_t)sCfg.sAppCfg.u8ExtSensorsStartupTime_s))
    {
      u32ExtSensorWaitMeassure_ms = (1000 * (uint32_t)sCfg.sAppCfg.u8ExtSensorsStartupTime_s) - u32ExtPowerOnSince_ms;
      while (u32ExtSensorWaitMeassure_ms) 
      {
        if (u32ExtSensorWaitMeassure_ms > 100)
        {
          BSP_SleepDelayMs(100);
          appHandleUI();  // Handle the UI in 100ms Intervall (Resetbutton and Serialcommunication)
          u32ExtSensorWaitMeassure_ms -= 100;
        }
        else
        {
          BSP_SleepDelayMs(u32ExtSensorWaitMeassure_ms);
          u32ExtSensorWaitMeassure_ms = 0;
        }
      }
    }  
    bScaling = true;  
    for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++,u8Msk<<=1)
    {
      if (((u8EnabledVoltageChMsk | u8EnabledCurrentChMsk) & u8Msk) == 0)
        continue;
      sShiftReg_1.ui8MuxUI_Msk = u8Msk;            
      BSP_SetShiftRegister_1( &sShiftReg_1);
      
      bChannelOk = true;
      while (1)
      {
        if (u8EnabledVoltageChMsk & u8Msk)
        {
          // 2019-01-07-Kd new V1.03 eADS1248_MeasureTyp_U_HighRes
          if (!ADS1148_SetupMeasure( (sCfg.sAppCfg.u8ChMskVoltageRangeHighRes & u8Msk) ? eADS1248_MeasureTyp_U_HighRes : eADS1248_MeasureTyp_U, i16ColdJunctionT_GC_100 ))    
          {
            bChannelOk = false;
            break;            
          }          
          BSP_SleepDelayMs(2); // Settling Time (2..10ms +300..250uV oder 20..50ms +200..150uV bei 25V Eingangsspannung auf Vorkanal)        
          if (!ADS1248_Measure( &i32ADCConvData, &fScaledValue, bScaling))
          {
            bChannelOk = false;
            break;   
          }        
          sMEASExtSenseData.fU_V[u8Ch] = fScaledValue;   
        }
        if (u8EnabledCurrentChMsk & u8Msk)
        {          
          if (!ADS1148_SetupMeasure( eADS1248_MeasureTyp_I, i16ColdJunctionT_GC_100 ))    
          {
            bChannelOk = false;
            break;             
          }
          BSP_SleepDelayMs(2); // Settling Time
          if (!ADS1248_Measure( &i32ADCConvData, &fScaledValue, bScaling))
          {
            bChannelOk = false;
            break;
          }              
          sMEASExtSenseData.fI_mA[u8Ch] = fScaledValue;            
        }
        break;
      } // end while (1)
      if (!bChannelOk)
        bOk = false;
    }
    memset(&sShiftReg_1,0,sizeof(sShiftReg_1));
    BSP_ClearShiftRegister_1();   
  }
  
  ADS1248_DeInit();    
  
  if (bSysPowerOff)
    BSP_SystemVcc(false);  // VCC System off   
  
  return bOk;
}

//------------------------------------------------------------------------------
void DAC_XferCpltCB(void)
{
  static bool bADC_running = false;    
  
  if (!bADC_running && g_u8Meas_ADC_Cnt)
  {
    bADC_running = true;
#ifdef USE_PHASE_HW_COMP
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    HAL_TIM_IC_Start(&htim4, TIM_CHANNEL_3);
    HAL_TIM_IC_Start(&htim4, TIM_CHANNEL_4);             
#endif    
    HAL_ADC_Start_DMA(&hadc, (uint32_t*) g_ai16MeasADC_DMA, ADC_DMA_CH_Cnt*ADC_CAPTURE_CNT);  
  }
  else if (g_u8Meas_ADC_Cnt)
  {
    g_u8Meas_ADC_Cnt--;    
#ifdef USE_PHASE_HW_COMP    
    g_au32MeasUCompCnt[g_u8Meas_ADC_Cnt] = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_3);   
    g_au32MeasICompCnt[g_u8Meas_ADC_Cnt] = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_4); 
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);
    __HAL_TIM_SET_COUNTER(&htim4, 0); 
#endif    
    if (!g_u8Meas_ADC_Cnt)
    {
      // CNDTR has the rest of the length (starts with ADC_DMA_CH_Cnt*ADC_CAPTURE_CNT and ends with 0 and will be reloaded with ADC_DMA_CH_Cnt*ADC_CAPTURE_CNT)
      g_u32MeasADC_DMA_Idx = ADC_DMA_CH_Cnt*ADC_CAPTURE_CNT - hdma_adc.Instance->CNDTR;  
      HAL_ADC_Stop_DMA(&hadc);
#ifdef USE_PHASE_HW_COMP      
      HAL_TIM_IC_Stop(&htim4, TIM_CHANNEL_3);
      HAL_TIM_IC_Stop(&htim4, TIM_CHANNEL_4);       
#endif      
      bADC_running = false;
    }
  } 

}


// 2020-06-15-Kd V1.08 Mittelung
void GetIntADCSensorsImpedance( uint8_t u8DAC_4Period_Cnt, uint16_t u16ADC_Vref_mV, float* pfADC_U_mV, float* pfADC_I1_mV, float* pfADC_I2_mV, bool bUseFirstValues)
{  
  bool    bGainWasChangedBefore;
  float   faryADC_I2_mV[IMP_I2_AVG_MEAS_CNT_MAX];
  float   faryADC_I2_Sort_mV[IMP_I2_AVG_MEAS_CNT_MAX];  
  int16_t i16aryMeasADC_Phase_U_I2_100mg[IMP_I2_AVG_MEAS_CNT_MAX];
  int16_t i16aryMeasADC_Phase_U_I2_Sort_100mg[IMP_I2_AVG_MEAS_CNT_MAX];
  float   fADC_U_mV = 0;
  float   fADC_I1_mV = 0; 
  float   fADC_I2_mV = 0;
  float   fI2_mv;
  int16_t i16MeasADC_Phase_U_I2_100mg;
  int16_t i16MeasADC_Phase_U_I2_sum_100mg = 0;
  uint8_t u8Cnt = 0;
  uint8_t u8CntSort, u8SortIdx, u8CntHide;
  uint8_t u8Msk;
  
#ifdef DEBUG  
  memset((void*)&faryADC_I2_mV, 0, sizeof(faryADC_I2_mV));
  memset((void*)&i16aryMeasADC_Phase_U_I2_100mg, 0, sizeof(i16aryMeasADC_Phase_U_I2_100mg));   
  memset((void*)&faryADC_I2_Sort_mV, 0, sizeof(faryADC_I2_Sort_mV));
  memset((void*)&i16aryMeasADC_Phase_U_I2_Sort_100mg, 0, sizeof(i16aryMeasADC_Phase_U_I2_Sort_100mg));  
#endif
  bGainWasChangedBefore = !bUseFirstValues;
  u8Msk = sShiftReg_1.ui8MuxImp_Msk;
  if (u8DAC_4Period_Cnt > IMP_I2_AVG_MEAS_CNT_MAX)
    u8DAC_4Period_Cnt = IMP_I2_AVG_MEAS_CNT_MAX;  
  for (u8Cnt=0; u8Cnt<u8DAC_4Period_Cnt; u8Cnt++)
  {
  // Initialize with invalid values  
    faryADC_I2_Sort_mV[u8Cnt] = -10000000;
    i16aryMeasADC_Phase_U_I2_Sort_100mg[u8Cnt] = 0x7FFF;
    
    if (bUseFirstValues)
    {
      fADC_U_mV   = *pfADC_U_mV; // take from last measure
      fADC_I1_mV  = *pfADC_I1_mV;      
      fI2_mv      = *pfADC_I2_mV;   
      i16MeasADC_Phase_U_I2_sum_100mg = g_i16MeasADC_Phase_U_I2_100mg;
      bUseFirstValues = false;
    }
    else
    {
      if (GetHWRev() > 0) // 2020-06-17-Kd save battery
      {               
        sShiftReg_2.bImpOP_EN = true;
        BSP_SetShiftRegister_2( &sShiftReg_2); // ca. 110us Enable PGA-OpAmp (and first call change gain)          
        sShiftReg_1.ui8MuxImp_Msk = u8Msk;
        BSP_SetShiftRegister_1( &sShiftReg_1); // ca. 70us Switch On ext. impedance          
        if (bGainWasChangedBefore)
        {
          BSP_SleepDelayMs(2); // Settling Time (After Gain change)
          bGainWasChangedBefore = false;
        }
      }      
      memset(g_ai16MeasADC_DMA,0xFF,sizeof(g_ai16MeasADC_DMA)); // takes about 2.1ms
      // Messung mit 3 * 1us -> 333kSps über 4 ganze DAC Perioden
      g_u8Meas_ADC_Cnt = MEAS_PERIOD_CNT; 
      while (g_u8Meas_ADC_Cnt); // 4-5ms      
       
      if (GetHWRev() > 0) // 2020-06-17-Kd save battery  and switch off PGA-OpAmp and ext. impedance from MUX while calculating
      {
        sShiftReg_2.bImpOP_EN = false;
        BSP_SetShiftRegister_2( &sShiftReg_2); // Disable PGA-OpAmp
        sShiftReg_1.ui8MuxImp_Msk = 0;
        BSP_SetShiftRegister_1( &sShiftReg_1); //  Switch Off ext. impedance 
      } 
      
      CalcADC_DMA(MEAS_PERIOD_CNT); // ca. 17ms DEF_CALC_IMP_FLOAT (or 14ms Integer)      

      i16MeasADC_Phase_U_I2_sum_100mg += g_i16MeasADC_Phase_U_I2_100mg;
    #ifdef DEF_CALC_IMP_FLOAT        
      fADC_U_mV  += (float)u16ADC_Vref_mV * g_afMeasADC[0] / PDDADC_FS; //PA_6 --> IMP_U_AI
      fADC_I1_mV += (float)u16ADC_Vref_mV * g_afMeasADC[1] / PDDADC_FS; //PA_7 --> IMP_I_1_AI      
      fI2_mv      = (float)u16ADC_Vref_mV * g_afMeasADC[2] / PDDADC_FS; //PC_4 --> IMP_I_2_AI      
    #else
      fADC_U_mV  += (float)((uint32_t)u16ADC_Vref_mV * (uint32_t)g_aui16MeasADC[0]) / PDDADC_FS; //PA_6 --> IMP_U_AI
      fADC_I1_mV += (float)((uint32_t)u16ADC_Vref_mV * (uint32_t)g_aui16MeasADC[1]) / PDDADC_FS; //PA_7 --> IMP_I_1_AI      
      fI2_mv      = (float)((uint32_t)u16ADC_Vref_mV * (uint32_t)g_aui16MeasADC[2]) / PDDADC_FS; //PC_4 --> IMP_I_2_AI
    #endif  
    }
    fADC_I2_mV += fI2_mv;
    faryADC_I2_mV[u8Cnt] = fI2_mv;  
    i16aryMeasADC_Phase_U_I2_100mg[u8Cnt] = g_i16MeasADC_Phase_U_I2_100mg;
  }
      
  if (u8DAC_4Period_Cnt <= 1)
  {
    *pfADC_U_mV = fADC_U_mV;
    *pfADC_I1_mV = fADC_I1_mV;
    *pfADC_I2_mV = fADC_I2_mV;
  }
  else
  {
    *pfADC_U_mV = fADC_U_mV/u8DAC_4Period_Cnt;
    *pfADC_I1_mV = fADC_I1_mV/u8DAC_4Period_Cnt;            
           
    if (u8DAC_4Period_Cnt >=4)
    {
      // Sort the values LSB is the lowest value
      for (u8Cnt=0; u8Cnt<u8DAC_4Period_Cnt; u8Cnt++)
      {
        fADC_I2_mV = faryADC_I2_mV[u8Cnt];
        for (u8SortIdx=0,u8CntSort=0; u8CntSort<u8DAC_4Period_Cnt; u8CntSort++)        
          if (fADC_I2_mV > faryADC_I2_mV[u8CntSort])
              u8SortIdx++;                  
        
        faryADC_I2_Sort_mV[u8SortIdx] = fADC_I2_mV; // lowest value first   
        
        i16MeasADC_Phase_U_I2_100mg = i16aryMeasADC_Phase_U_I2_100mg[u8Cnt];
        for (u8SortIdx=0,u8CntSort=0; u8CntSort<u8DAC_4Period_Cnt; u8CntSort++)        
          if (i16MeasADC_Phase_U_I2_100mg > i16aryMeasADC_Phase_U_I2_100mg[u8CntSort])
              u8SortIdx++;                  
        i16aryMeasADC_Phase_U_I2_Sort_100mg[u8SortIdx] = i16MeasADC_Phase_U_I2_100mg;
      }
      // all same values are sorted to same sorted list index -> search for unused places and fill them with last valid value
      for (fADC_I2_mV=faryADC_I2_Sort_mV[0],i16MeasADC_Phase_U_I2_100mg=i16aryMeasADC_Phase_U_I2_Sort_100mg[0],u8SortIdx=1; u8SortIdx<u8DAC_4Period_Cnt; u8SortIdx++)
      {
        if (faryADC_I2_Sort_mV[u8SortIdx] == -10000000)
          faryADC_I2_Sort_mV[u8SortIdx] = fADC_I2_mV;
        else
          fADC_I2_mV = faryADC_I2_Sort_mV[u8SortIdx];
        if (i16aryMeasADC_Phase_U_I2_Sort_100mg[u8SortIdx] == 0x7FFF)
          i16aryMeasADC_Phase_U_I2_Sort_100mg[u8SortIdx] = i16MeasADC_Phase_U_I2_100mg;
        else
          i16MeasADC_Phase_U_I2_100mg = i16aryMeasADC_Phase_U_I2_Sort_100mg[u8SortIdx];
      }
      
      if (u8DAC_4Period_Cnt >= 8)
        u8CntHide = 2; // Delete the 2 lowest and the 2 highest values and average the rest
      else
        u8CntHide = 1; // Delete the lowest and the highest values and average the rest
      
      for (i16MeasADC_Phase_U_I2_sum_100mg=0, fADC_I2_mV=0, u8Cnt=0, u8SortIdx=u8CntHide; u8SortIdx< u8DAC_4Period_Cnt-u8CntHide; u8SortIdx++, u8Cnt++)
      {
        fADC_I2_mV += faryADC_I2_Sort_mV[u8SortIdx];        
        i16MeasADC_Phase_U_I2_sum_100mg += i16aryMeasADC_Phase_U_I2_Sort_100mg[u8SortIdx];   
      }
      *pfADC_I2_mV = fADC_I2_mV / u8Cnt; 
      g_i16MeasADC_Phase_U_I2_100mg = i16MeasADC_Phase_U_I2_sum_100mg/u8Cnt;       
    }
    else     
    {
      *pfADC_I2_mV = fADC_I2_mV/u8DAC_4Period_Cnt;    
      g_i16MeasADC_Phase_U_I2_100mg = i16MeasADC_Phase_U_I2_sum_100mg/u8DAC_4Period_Cnt; 
    }
  }
  
  sShiftReg_1.ui8MuxImp_Msk = u8Msk; // set back
}
  

bool MEASGetIntADCSensors( uint8_t  u8EnabledChMsk)
{
const double      Pi = 3.141592653;  
  bool            bOk = true;  
  uint8_t         u8Ch, u8Msk, u8GainU, u8GainI, u8MsgAvgCh;
  int16_t         i16PhaseAngle_100mg;
  uint16_t        u16ADC_Vref_mV;
  float           fADC_U_mV, fADC_I1_mV, fADC_I2_mV;
  bool            bSysPowerOff = !BSP_IsSystemVccON();
  bool            bGainChanged;
  bool            bI2;
  bool            bOpenInput;
  double          rad;
  
	
	/* Turn on Si705x */	   
  if (bSysPowerOff)
    BSP_SystemVcc(true);  // VCC System on  
  
  BSP_SleepDelayMs(5); // Settling Time
  
  g_u8Meas_ADC_Cnt = 0;
  memset(&sShiftReg_1,0,sizeof(sShiftReg_1));
  BSP_ClearShiftRegister_1();    
    
  if (GetHWRev() == 0)
  {
    // ACHTUNG REV00 bei ADG788 ist NO und NC verkehrt
    sShiftReg_1.ui8MuxImp_Msk = 0xFF;
    BSP_SetShiftRegister_1( &sShiftReg_1); 
  }
  
  BSP_SleepDelayMs(5); // Settling Time
  u16ADC_Vref_mV = MEASGetADCmV(ADC_CHANNEL_VREFINT, 16);

#ifdef USE_PHASE_HW_COMP  
  HW_TIM4_Init();
#endif  
  HW_ADC_MultipleConv_Init();  
  HW_DAC_Init(DAC_XferCpltCB); 
  HW_DAC_Start_1kHz_1Vpp();

  memset(&sShiftReg_2,0,sizeof(sShiftReg_2));
  sShiftReg_2.bImpOP_EN = true;  // DANI ACHTUNG : HW REV00 Comp. OpAmp kann -1.6V auf uC Eingang bringen !!! 
  for (u8Ch=0,u8Msk=0x01; u8Ch<EXT_SENSE_CHANNELS; u8Ch++,u8Msk<<=1)
  {
    if ((u8EnabledChMsk & u8Msk) == 0) // 2020-06-26-Kd V1.09 new (only measure configured channels)
      continue;
      
    sShiftReg_2.eOP_GainSetupImpI = eOP_GAIN_1;
    sShiftReg_2.eOP_GainSetupImpU = eOP_GAIN_1;
    u8GainU = 1;
    u8GainI = 1;
    bI2 = true;
    bOpenInput = false;
    BSP_SetShiftRegister_2( &sShiftReg_2); // Gain 1  
    if (GetHWRev() == 0)
      sShiftReg_1.ui8MuxImp_Msk = ~u8Msk; // ACHTUNG bei ADG788 ist NO und NC verkehrt   
    else
      sShiftReg_1.ui8MuxImp_Msk = u8Msk;
    BSP_SetShiftRegister_1( &sShiftReg_1);      
    // Auto Gain    
    while (1) 
    {                 
      if (GetHWRev() <= 0)
        BSP_SleepDelayMs(2); // Settling Time
            
      GetIntADCSensorsImpedance( 1, u16ADC_Vref_mV, &fADC_U_mV, &fADC_I1_mV, &fADC_I2_mV, false);                     
      
      bGainChanged = false;
      if ((sShiftReg_2.eOP_GainSetupImpU < eOP_GAIN_128) && (sShiftReg_2.eOP_GainSetupImpI < eOP_GAIN_128))
      {
        if (fADC_U_mV < 500) // Treshold 3V3 / 2 = 1.65 / 1.414 / 2.2 =  Uref / SquareRoot(2) / NextGainFactor = 0.53Vrms -> 0.5V
        {
          sShiftReg_2.eOP_GainSetupImpU++;
          u8GainU *= 2;
          bGainChanged = true;
        }
        // Erster Durchlauf und I2 schon im Anschlag (max. 1.65/1.414 = 1.17Vrms -> 1.1)
        if ((sShiftReg_2.eOP_GainSetupImpI == eOP_GAIN_1) && (fADC_I2_mV > 1100))
          bI2 = false;
        if (bI2)
        {
          if (fADC_I2_mV < 500) 
          {   
            // 2020-06-15-Kd we go up to GAIN 101*64 for I2 measurement -> so we can measure up to about 3.7MOhm with best gain (open Input)
            if (sShiftReg_2.eOP_GainSetupImpI >= eOP_GAIN_64)             
            {
              bOpenInput = true; // we have > 3.5MOhm Impedance -> open input
            }
            else
            {
              sShiftReg_2.eOP_GainSetupImpI++;
              u8GainI *= 2;
              bGainChanged = true;
            }
          }          
        }
        else if (fADC_I1_mV < 500)
        {
          sShiftReg_2.eOP_GainSetupImpI++;
          u8GainI *= 2;
          bGainChanged = true;
        }        
      }
      if (!bGainChanged)
      {  
        if (bI2 && !bOpenInput) // 2020-06-15-Kd mehrere Messungen machen und mitteln falls nicht offener Eingang (gem. Herr Kronenberg -> starkes Rauschen der Impedanz Magnitude Werte bei > 100 kOhm)
        {
          u8MsgAvgCh = 2*sShiftReg_2.eOP_GainSetupImpI; // eOP_GAIN_1 = 1; eOP_GAIN_2 = 2 (100kOhm); eOP_GAIN_4 = 4 (200kOhm); eOP_GAIN_8 = 6 (300kOhm); eOP_GAIN_16..128 = 8 (eOP_GAIN_16 für zB. 500kOhm)
          if(u8MsgAvgCh > 8)            
            u8MsgAvgCh = 8; 
          if (u8MsgAvgCh > 1)          
            GetIntADCSensorsImpedance( u8MsgAvgCh, u16ADC_Vref_mV, &fADC_U_mV, &fADC_I1_mV, &fADC_I2_mV, true); 
        }        
        sMEASExtSenseData.fImpU_mv[u8Ch] = fADC_U_mV / u8GainU;
        if (bI2)
          sMEASExtSenseData.fImpI_uA[u8Ch] = fADC_I2_mV * 10 / ((uint32_t)u8GainI * 101);
        else
          sMEASExtSenseData.fImpI_uA[u8Ch] = fADC_I1_mV * 10 / u8GainI;
        sMEASExtSenseData.fImp_Ohm[u8Ch] = sMEASExtSenseData.fImpU_mv[u8Ch] * 1000 / sMEASExtSenseData.fImpI_uA[u8Ch]; // Scheinleistung (Apparent)  
        if (sMEASExtSenseData.fImp_Ohm[u8Ch] > 2000000.0)
        {
          // Open Input
          sMEASExtSenseData.fImpPhaseShiftAngle_g[u8Ch] = 0;
          sMEASExtSenseData.fImp_Ohm[u8Ch] = 999999.9;
          sMEASExtSenseData.fImpActive_Ohm[u8Ch] = 999999.9;
          sMEASExtSenseData.fImpReactive_Ohm[u8Ch] = 0;
        }
        else
        {
  #ifdef USE_PHASE_HW_COMP        
          g_au32MeasUCompCnt[]
          g_au32MeasiCompCnt[]
  #else          
          if (bI2)          
            i16PhaseAngle_100mg = g_i16MeasADC_Phase_U_I2_100mg;                     
          else          
            i16PhaseAngle_100mg = g_i16MeasADC_Phase_U_I1_100mg;               
  #endif
          rad = (float)i16PhaseAngle_100mg * Pi / 1800; /* Berechnen des Bogenmaßwinkels */
          sMEASExtSenseData.fImpPhaseShiftAngle_g[u8Ch] = (float)i16PhaseAngle_100mg/10; // 2024-03-28-Kd V1.12
          // old sMEASExtSenseData.i8ImpPhaseShiftAngle_g[u8Ch] = (int8_t)(i16PhaseAngle_100mg/10);
          sMEASExtSenseData.fImpActive_Ohm[u8Ch] = fabs(sMEASExtSenseData.fImp_Ohm[u8Ch] * (float)cos(rad)); // Wirkwiderstand
          sMEASExtSenseData.fImpReactive_Ohm[u8Ch] = fabs(sMEASExtSenseData.fImp_Ohm[u8Ch] * (float)sin(rad)); // Blindwiderstand      
          if (sMEASExtSenseData.fImp_Ohm[u8Ch] > 999999.9)
            sMEASExtSenseData.fImp_Ohm[u8Ch] = 999999.9;
          if (sMEASExtSenseData.fImpActive_Ohm[u8Ch] > 999999.9)
            sMEASExtSenseData.fImpActive_Ohm[u8Ch] = 999999.9;
          if (sMEASExtSenseData.fImpReactive_Ohm[u8Ch] > 999999.9)
            sMEASExtSenseData.fImpReactive_Ohm[u8Ch] = 999999.9;        
        }
        break;
      }
      if (GetHWRev() <= 0)
        BSP_SetShiftRegister_2( &sShiftReg_2);        
    }
  }
  
  HW_DAC_Stop();
  HW_DAC_DeInit();
  HW_ADC_MultipleConv_DeInit();  
#ifdef USE_PHASE_HW_COMP  
  HW_TIM4_DeInit();
#endif  
  memset(&sShiftReg_1,0,sizeof(sShiftReg_1));
  BSP_ClearShiftRegister_1(); 
  memset(&sShiftReg_2,0,sizeof(sShiftReg_2));
  BSP_ClearShiftRegister_2();     
  
  if (bSysPowerOff)
    BSP_SystemVcc(false);  // VCC System off     
  
  return bOk;
}

//------------------------------------------------------------------------------
bool MEASGetColdJunctionTemperature( int16_t * i16ColdJuncTemp_GC_100 )
{
  bool      bRet = false;
  bool      bSysPowerOff = !BSP_IsSystemVccON();
  float     fTemp = 0;
	
	/* Turn on Si705x */	   
  if (bSysPowerOff)  
  {
    BSP_SystemVcc(true);  // VCC System on
    BSP_SleepDelayMs(80); // Max. 80ms for the whole op. temp. range
  }    
  
	/* I2C1 init */
	HW_I2C1_Init();
  /* Measurement  */
  bRet = Si705x_GetTemp(&fTemp);
				
  /* I2C1 deinit */
	HW_I2C1_DeInit();	
	/* Turn off Si705x */
  if (bSysPowerOff)
    BSP_SystemVcc(false);  // VCC System off 
	
	//otherwise we parse the measured value into an int and return false
	*i16ColdJuncTemp_GC_100 = (int16_t)(fTemp*100);
	return bRet; 
}

//------------------------------------------------------------------------------
/**
* @brief This function calculates the battery voltage from the ADC Value
*
* @details
*
* @param None 
*
* @retval Battery voltage in mV (uint16_t)   	
*/
//------------------------------------------------------------------------------
uint16_t MEASGetUbat( void ) 
{
  GPIO_InitTypeDef initStruct={0};  
  //Battery ADC Channel
  uint16_t MeasuredLevel = 0;
  uint32_t milliVolt     = 0;
  //VDDA
  uint16_t u16VREFINT = 0;
  uint32_t u32VDDA    = 0;
  
  // Activate the meassure resistor bridge
  initStruct.Mode   = GPIO_MODE_OUTPUT_PP;
  initStruct.Speed  = GPIO_SPEED_MEDIUM;  
  initStruct.Pull   = GPIO_NOPULL;       
  initStruct.Pin    = MEAS_VBAT_ACTIVATE_Pin;  
  HAL_GPIO_Init(MEAS_VBAT_ACTIVATE_GPIO_Port, &initStruct);   
  HAL_GPIO_WritePin( MEAS_VBAT_ACTIVATE_GPIO_Port, MEAS_VBAT_ACTIVATE_Pin, GPIO_PIN_RESET); 
    
  // VREF INT
  u16VREFINT = HW_ADC1_Read(ADC_CHANNEL_VREFINT,8);	
  // Read the current value of battery ADC Channel
  MeasuredLevel = HW_ADC1_Read(ADC_CHANNEL_10,8); //PC_0 --> AI_UBat 	  
	
  //reconfigure pins to analogical
  initStruct.Mode   = GPIO_MODE_ANALOG;  
  HAL_GPIO_Init(MEAS_VBAT_ACTIVATE_GPIO_Port, &initStruct);  
  
  if (u16VREFINT == 0) {
    milliVolt = 0;
  } else {
    // VDDA = VDDA_VREFINT_CAL(3V, factory calibration) * VREF_INT_CAL (Calib. value) / VREFINT_DATA (u16VREFINT)			
    u32VDDA = ((uint32_t) VDDA_VREFINT_CAL * (*VREFINT_CAL) ) / u16VREFINT;
    // ADC value in millivolt measured taking the actual VDDA and calculated over the resistor bridge R302 and R304
    milliVolt = (( uint32_t )MeasuredLevel * u32VDDA * (33 + 27) ) / PDDADC_FS / 33;
  }		
		
  return ( uint16_t ) milliVolt;		
}

//------------------------------------------------------------------------------
/**
* @brief This function calculates the battery level --> this needs probably to be done differently for SenseTwo
*        ASK DANI 2016-11-01-Ra  
*
* @details
*
* @param None
*
* @retval Battery Level --> 1 is 2400mV 254 is 3000 mV (uint8_t)   	
*/
//------------------------------------------------------------------------------
uint8_t MEASGetBatteryLevel( void ) 
{
    uint8_t batteryLevel = 1;
    uint16_t measuredLevel = 0;
     
    measuredLevel = MEASGetUbat( );

    if( measuredLevel >= BAT_FULL_mV )  
    {
        batteryLevel = BAT_FULL_LEVEL;
    }
    else if( measuredLevel <= BAT_EMPTY_mV ) 
    {
        batteryLevel = BAT_EMPTY_LEVEL;
    }
    else
    {
        batteryLevel += ((measuredLevel - BAT_EMPTY_mV)*(BAT_FULL_LEVEL - BAT_EMPTY_LEVEL))/(BAT_FULL_mV - BAT_EMPTY_mV);
    }
    return batteryLevel;
}

//------------------------------------------------------------------------------
/**
* @brief This function calculates the voltage on a channel from its ADC Value
*
* @details
*
* @param ADCChannel to be measured 
*
* @retval Voltage in mV on the selected channel	
*/
//------------------------------------------------------------------------------
uint16_t MEASGetADCmV (uint32_t ADCChannel, uint16_t u16AverageCnt)  
{
	  //ADC CHANNEL TO BE MEASURED
    uint16_t MeasuredLevel = 0;
    uint32_t milliVolt = 0;
	  //VDDA
	  uint16_t u16VREFINT = 0;
	  uint32_t u32VDDA = 0;

	  // VREF INT
		u16VREFINT = HW_ADC1_Read(ADC_CHANNEL_VREFINT, u16AverageCnt);
    
	  if (ADCChannel != ADC_CHANNEL_VREFINT){
	    // ADC CHANNEL
      MeasuredLevel = HW_ADC1_Read(ADCChannel, u16AverageCnt);   
    }
		
    if (u16VREFINT == 0) {
      milliVolt = 0;
    } else {
	    // VDDA = VDDA_VREFINT_CAL(3V, factory calibration) * VREF_INT_CAL (Calib. value) / VREFINT_DATA (u16VREFINT)			
			u32VDDA = (((uint32_t) VDDA_VREFINT_CAL * (*VREFINT_CAL) ) / u16VREFINT);
			if (ADCChannel != ADC_CHANNEL_VREFINT){
			  // ADC value in millivolt measured taking the actual VDDA
        milliVolt = (( uint32_t )MeasuredLevel * u32VDDA ) / PDDADC_FS;
      } else {
				milliVolt = u32VDDA;
			}	
		}		
    return ( uint16_t ) milliVolt;
}


/*****END OF FILE****/
