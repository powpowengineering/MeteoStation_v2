//**************************************************************************************************
// @Module        AM2305
// @Filename      am2305.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the AM2305 module.
//                
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef AM2305_H
#define AM2305_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

#include "stm32l1xx.h"

#include "general_types.h"

// Get configuration of the program module
#include "am2305_cfg.h"



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of global (public) variables
//**************************************************************************************************

// None.


//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

// Init am2305
extern void AM2305_Init(void);
// Get humidity and temperature
extern STD_RESULT AM2305_GetHumidityTemperature(float *const humidity,float *const temperature);

#endif // #ifndef AM2305_H

//****************************************** end of file *******************************************
