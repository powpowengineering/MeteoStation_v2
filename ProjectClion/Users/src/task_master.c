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

    for(;;)
    {
        // Update dump eeprom
//        RECORD_MAN_UpdateDumpMem();
        vTaskResume(TASK_READ_SEN_hHandlerTask);

        // Show current time
        TIME_TimeShow();

        // Check sensors alarm
        if (TRUE == TIME_CheckAlarm(TIME_ALARM_SENS))
        {
            printf("The SENSORS alarm clock rang\r\n");

            // Start measure
            vTaskResume(TASK_READ_SEN_hHandlerTask);

            // Set sensors alarm
            sTime.tm_min = 0;
            sTime.tm_sec = 30;

            TIME_SetAlarm(sTime, TIME_ALARM_SENS);
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

            // Set sensors alarm
            sTime.tm_min = 5;
            sTime.tm_sec = 0;

            TIME_SetAlarm(sTime, TIME_ALARM_GSM);
        }
        else
        {
            DoNothing();
        }

        vTaskDelay(2000/portTICK_RATE_MS);

        // Check TASK_GSM state
        vTaskGetInfo(TASK_GSM_hHandlerTask,&xTaskStatus,pdTRUE,eInvalid );

        if (eSuspended == xTaskStatus.eCurrentState)
        {
            // Check TASK_READ_SEN state
            vTaskGetInfo(TASK_READ_SEN_hHandlerTask,&xTaskStatus,pdTRUE,eInvalid );

            if (eSuspended == xTaskStatus.eCurrentState)
            {
                // W25Q power down
//                W25Q_PowerDown();
//                printf("W25Q power down\r\n");

                // Go to StandBy
                printf("Go to standBy mode\r\n");
                HAL_PWREx_EnableGPIOPullUp(PWR_GPIO_C, PWR_GPIO_BIT_4);
                HAL_PWREx_EnablePullUpPullDownConfig();
                HAL_PWR_EnterSTANDBYMode();
//                HAL_PWREx_EnterSHUTDOWNMode();
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