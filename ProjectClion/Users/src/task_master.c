//**************************************************************************************************
// @Module        TASK_MASTER
// @Filename      task_master.c
//--------------------------------------------------------------------------------------------------
// @Platform      stm32l4
//--------------------------------------------------------------------------------------------------
// @Compatible    stm32l476, stm32l452
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the W25Q functionality.
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
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Native header
#include "task_master.h"

// Get terminal inteface
#include "term-srv.h"

// Init interface
#include "Init.h"

// STD lib
#include "string.h"

// Get record manager interface
#include "record_manager.h"

// Get task read sensors
#include "task_read_sensors.h"

// Get task GSM interface
#include "task_GSM.h"

#include "W25Q_drv.h"

#include "stdlib.h"

#include "eeprom_emulation.h"

#include "printf.h"

#include "time_drv.h"

#include "task_terminal.h"



//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None.


//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// None.


//**************************************************************************************************
// Declarations of local (private) data types
//*************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

// Mutex Acquisition Delay
#define TASK_MASTER_MUTEX_DELAY                 (1000U)

// Number of entries to send to the server
#define TASK_GSM_QTY_REC_TO_SEND                (10U)



//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// Buff for record
static uint8_t TASK_MASTER_aRecord[RECORD_MAN_MAX_SIZE_RECORD];

/* Buffer used for displaying Time */
static uint8_t aShowTime[50] = {0};

static TIME_type sTime;

static TIME_type AlarmSens;
static TIME_type AlarmGSM;


//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// None.



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      vTaskMaster()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void vTaskMaster(void *pvParameters)
{
    TaskStatus_t xTaskStatus;

//    W25Q_EraseBlock(0,W25Q_BLOCK_MEMORY_ALL);
//while(1);

    // Load alarm sens value
    if (RESULT_OK == TIME_LoadAlarm(&AlarmSens, TIME_ALARM_SENS))
    {
        DoNothing();
    }
    else
    {
        printf("task_master ERROR: TIME_LoadAlarm RESULT_NOT_OK\r\n");
        // Set sensors alarm
        AlarmSens.tm_hour = 0;
        AlarmSens.tm_min = 1;
        AlarmSens.tm_sec = 0;
    }



    // Load alarm gsm value
    if (RESULT_OK == TIME_LoadAlarm(&AlarmGSM, TIME_ALARM_GSM))
    {
        DoNothing();
    }
    else
    {
        printf("task_master ERROR: TIME_LoadAlarm RESULT_NOT_OK\r\n");
        // Set sensors alarm
        AlarmGSM.tm_hour = 1;
        AlarmGSM.tm_min = 0;
        AlarmGSM.tm_sec = 0;
    }


    for(;;)
    {
        // Update dump eeprom
//        RECORD_MAN_UpdateDumpMem();
//        vTaskResume(TASK_READ_SEN_hHandlerTask);
//        vTaskDelay(2000/portTICK_RATE_MS);

        vTaskDelay(3000/portTICK_RATE_MS);
        // Show current time
        TIME_TimeShow();

        // Check sensors alarm
        if (TRUE == TIME_CheckAlarm(TIME_ALARM_SENS))
        {
            printf("The SENSORS alarm clock rang\r\n");

            // Start measure
            vTaskResume(TASK_READ_SEN_hHandlerTask);

            vTaskDelay(5000/portTICK_RATE_MS);

            // Set Alarm sens
            TIME_SetAlarm(AlarmSens, TIME_ALARM_SENS);
        }
        else
        {
            DoNothing();
        }

        // Check GSM alarm
        if (TRUE == TIME_CheckAlarm(TIME_ALARM_GSM))
        {
            printf("The GSM alarm clock rang\r\n");

            // Send data to server
            vTaskResume(TASK_GSM_hHandlerTask);

            // Set Alarm GSM
            TIME_SetAlarm(AlarmGSM, TIME_ALARM_GSM);
        }
        else
        {
            DoNothing();
        }



        // Check TASK_GSM state
        vTaskGetInfo(TASK_GSM_hHandlerTask,&xTaskStatus,pdTRUE,eInvalid );

        if (eSuspended == xTaskStatus.eCurrentState)
        {
            // Check TASK_READ_SEN state
            vTaskGetInfo(TASK_READ_SEN_hHandlerTask,&xTaskStatus,pdTRUE,eInvalid );

            if (eSuspended == xTaskStatus.eCurrentState)
            {
                if (TASK_TERMINAL_TIMEOUT <= TASK_TERMINAL_nTimeOut)
                {

                    printf("Go to standBy mode\r\n");

                    // Enable pullUp for power control GSM
                    HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_C, PWR_GPIO_BIT_4);
                    HAL_PWREx_EnablePullUpPullDownConfig();

                    // Go to StandBy
                    HAL_PWR_EnterSTANDBYMode();
                }
                else
                {
                    DoNothing();
                }
            }
            else
            {
                DoNothing();
            }
        }
        else
        {
            DoNothing();
        }
    }
} // end of vTaskMaster()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

// None.

//****************************************** end of file *******************************************