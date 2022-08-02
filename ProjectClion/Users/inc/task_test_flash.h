//**************************************************************************************************
// @Module        TASK_TEST_FLASH
// @Filename      task_test_flash.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the TASK_TEST_FLASH interface.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef TASK_TEST_FLASH_H
#define TASK_TEST_FLASH_H


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
extern void vTaskTestFlash(void *pvParameters);

// Task test flash with EE
_Noreturn extern void vTaskTestFlashWithEE(void *pvParameters);


#endif // #ifndef TASK_SENSOR_READ_H

//****************************************** end of file *******************************************