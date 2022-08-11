//**************************************************************************************************
// @Module        TASK_READ_SENSORS
// @Filename      task_sensors_read.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the TASK_SENSOR_READ interface.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef TASK_READ_SENSORS_H
#define TASK_READ_SENSORS_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Get HAL interface
#include "stm32l4xx_hal.h"

// Get general types
#include "general_types.h"

// Get module cfg
#include "task_read_sensors_cfg.h"

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

extern TaskHandle_t    TASK_READ_SEN_hHandlerTask;



//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

// Task sensors read.
extern void vTaskReadSensors(void *pvParameters);



#endif // #ifndef TASK_READ_SENSORS_H

//****************************************** end of file *******************************************