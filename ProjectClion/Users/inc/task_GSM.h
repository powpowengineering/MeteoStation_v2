//**************************************************************************************************
// @Module        TASK_GSM
// @Filename      task_GSM.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the TASK_GSM module.
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************

#ifndef TASK_GSM_H
#define TASK_GSM_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Get HAL interface
#include "stm32l4xx_hal.h"

#include "compiler.h"

// General types
#include "general_types.h"

// Get configuration of the program module
#include "task_GSM_cfg.h"

// Get RTOS interface
#include "cmsis_os.h"



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

// Task handler
extern TaskHandle_t TASK_GSM_hHandlerTask;



//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

// GSM task
extern void vTaskGSM(void *pvParameters);



#endif // #ifndef TASK_GSM_H

//****************************************** end of file *******************************************
