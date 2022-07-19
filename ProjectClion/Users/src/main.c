//**************************************************************************************************
// @Module        MAIN
// @Filename      main.c
//--------------------------------------------------------------------------------------------------
// @Platform      STM32
//--------------------------------------------------------------------------------------------------
// @Compatible    STM32L151
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the AM2305 functionality.
//
//
//                Abbreviations:
//                  None.
//
//
//                Global (public) functions:
//
//
//                Local (private) functions:
//
//
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************



//**************************************************************************************************
// Project Includes
//**************************************************************************************************
// stm32 STL
#include "stm32l1xx.h"
// drivers
#include "OneWire.h"
#include "usart_drv.h"
#include "Init.h"
#include "ds18b20.h"
#include "am2305_drv.h"
#include "ftoa.h"
#include "printf.h"
// Freertos
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Include task_sensors_read interface
#include "tasks_sensors_read.h"
// Include task_test_flash interface
#include "task_test_flash.h"
#include "task_mqtt.h"
#include "task_test_EE.h"



//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None.


//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// Queue handle for measure data
xQueueHandle xQueueMeasureData;



//**************************************************************************************************
// Declarations of local (private) data types
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************
#define PRINTF_DISABLE_SUPPORT_FLOAT
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#define TLM_CHANNEL                     (0)

// Prm vTaskSensorsRead
#define TASK_SEN_R_STACK_DEPTH          (256U)
#define TASK_SEN_R_PARAMETERS           (NULL)
#define TASK_SEN_R_PRIORITY             (1U)

// Prm vTaskTestFlash
#define TASK_TEST_FLASH_STACK_DEPTH          (256U)
#define TASK_TEST_FLASH_PARAMETERS           (NULL)
#define TASK_TEST_FLASH_PRIORITY             (1U)

// Prm vTaskMQTT
#define TASK_MQTT_STACK_DEPTH          (800U)
#define TASK_MQTT_PARAMETERS           (NULL)
#define TASK_MQTT_PRIORITY             (1U)

// Prm vTaskEE
#define TASK_EE_STACK_DEPTH          (800U)
#define TASK_EE_PARAMETERS           (NULL)
#define TASK_EE_PRIORITY             (1U)

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************




//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      main()
//--------------------------------------------------------------------------------------------------
// @Description   Main function.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void main(void)
{
    Init();
    // Init OneWire
    ONE_WIRE_init();
    AM2305_Init();
    USART_init();

    xQueueMeasureData = xQueueCreate( 3, sizeof( TASK_SENSOR_READ_DATA ) );



//    xTaskCreate(vTaskSensorsRead,"TaskSensorsRead",TASK_SEN_R_STACK_DEPTH,\
//                TASK_SEN_R_PARAMETERS,\
//                TASK_SEN_R_PRIORITY,NULL);

//    xTaskCreate(vTaskTestFlash,"TaskTestFlash",TASK_TEST_FLASH_STACK_DEPTH,\
//                TASK_TEST_FLASH_PARAMETERS,\
//                TASK_TEST_FLASH_PRIORITY,NULL);

//    xTaskCreate(vTaskMQTT,"TaskMQTT",TASK_MQTT_STACK_DEPTH,\
//                TASK_MQTT_PARAMETERS,\
//                TASK_MQTT_PRIORITY,&HandleTask_MQTT);

    xTaskCreate(vTaskTestEE,"TaskTestEE",TASK_EE_STACK_DEPTH,\
                TASK_EE_PARAMETERS,\
                TASK_EE_PRIORITY,NULL);

    // Blocking MQTT task
//    vTaskSuspend( HandleTask_MQTT );

    vTaskStartScheduler();



    while(1);
}// end of main



//**************************************************************************************************
// @Function      _putchar()
//--------------------------------------------------------------------------------------------------
// @Description   Put char function used by printf.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void _putchar(char character)
{
    USART_PutChar(TLM_CHANNEL, character);
}// end of _putchar



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************




//****************************************** end of file *******************************************