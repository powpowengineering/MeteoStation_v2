//**************************************************************************************************
// @Module        DS18B20
// @Filename      ds18b20.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the ds18b20 sensor interface.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef DS18B20_H
#define DS18B20_H


//**************************************************************************************************
// Project Includes
//**************************************************************************************************

#include "stm32l4xx_hal.h"

#include "general_types.h"

#include "ds18b20_cfg.h"



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// None.


//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// DS18B20 resolution
#define DS18B20_RESOLUTION_9_BIT                (uint8_t)(0U)
#define DS18B20_RESOLUTION_10_BIT               (uint8_t)(1U << 5)
#define DS18B20_RESOLUTION_11_BIT               (uint8_t)(2U << 5)
#define DS18B20_RESOLUTION_12_BIT               (uint8_t)(3U << 5)


//**************************************************************************************************
// Declarations of global (public) variables
//**************************************************************************************************

// None.


//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

// Read ROM ID
extern STD_RESULT DS18B20_GetID(const uint8_t nCh, uint64_t *const ID);
// Get temperature
extern STD_RESULT DS18B20_GetTemperature(uint8_t nCh, const uint64_t *const ID, float *const t );
// Set Resolution
extern STD_RESULT DS18B20_SetResolution(uint8_t nCh, const uint8_t* ID,const uint8_t* resolution);

#endif // #ifndef DS18B20_H

//****************************************** end of file *******************************************