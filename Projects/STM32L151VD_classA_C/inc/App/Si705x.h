/**============================================================================
* @file      Si705x.h
* @date      2018-06-14
*
* @author    D. Koller
*
* @copyright comtac AG,
*            Allenwindenstrasse 1,
*            CH-8247 Flurlingen
*
* @brief     * Header of driver for SHT3X Sensor from sensirion
*            
* VERSION:   
* 
* V0.01      2018-06-14-Kd      Create File			
*
*============================================================================*/
/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __SI705X_H
#define __SI705X_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
   #include <stdbool.h>
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

/*!
 * \brief  gets temperature in °C
 *
 * \param  *temperature  : Pointer to temperature 
 *	
 * \retval result        : 
 *
 */
bool Si705x_GetTemp(float* fTemperature);

#ifdef __cplusplus
}
#endif

#endif //__SI705X_H

/*****END OF FILE****/
