//**************************************************************************************************
// @Module        TASK_TEST_EE
// @Filename      task_test_EE.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the TASK_TEST_EE interface.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef TASK_TEST_EE_H
#define TASK_TEST_EE_H


//**************************************************************************************************
// Project Includes
//**************************************************************************************************

#include "stm32l4xx_hal.h"

#include "general_types.h"

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

// Task sensors read.
_Noreturn extern void vTaskTestEE(void *pvParameters);

#endif // #ifndef TASK_TEST_EE_H

//****************************************** end of file *******************************************