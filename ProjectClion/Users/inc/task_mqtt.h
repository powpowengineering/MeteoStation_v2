//**************************************************************************************************
// @Module        TASK_MQTT
// @Filename      task_mqtt.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the TASK_MQTT interface.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef TASK_MQTT_H
#define TASK_MQTT_H


//**************************************************************************************************
// Project Includes
//**************************************************************************************************

#include "stm32l1xx.h"

#include "general_types.h"

// FreeRtos
#include "FreeRTOS.h"
#include "task.h"



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

extern TaskHandle_t HandleTask_MQTT;


//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

// Task sensors read.
extern void vTaskMQTT(void *pvParameters);

#endif // #ifndef TASK_MQTT_H

//****************************************** end of file *******************************************