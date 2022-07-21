//**************************************************************************************************
// @Module        TASK_TEST_EE
// @Filename      task_test_EE.c
//--------------------------------------------------------------------------------------------------
// @Platform      STM32
//--------------------------------------------------------------------------------------------------
// @Compatible    STM32L151
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the TASK_TEST_EE functionality.
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

// Native header
#include "task_test_flash.h"
// drivers
#include "W25Q_drv.h"
#include "checksum.h"
#include "printf.h"
#include "stdlib.h"
#include "ftoa.h"
#include "eeprom.h"
#include "W25Q_drv.h"
// FreeRtos
#include "FreeRTOS.h"
#include "task.h"


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
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

// Virtual address 1
#define EE_VER_ADR_LAST_SEND           (uint16_t)(0x1)
#define EE_VER_ADR_CURRENT_WR          (uint16_t)(0x2)

#define TEST_EE_QTY_FLASH_RECORDS       (10U)
#define TEST_EE_SIZE_RECORD_DATA        (100U)
#define TEST_EE_SIZE_CRC8               (1U)
#define TEST_EE_SIZE_RECORD_BUF         (TEST_EE_SIZE_RECORD_DATA + TEST_EE_SIZE_CRC8)

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

uint8_t nRecordBuf[TEST_EE_SIZE_RECORD_BUF];


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
// @Function      vTaskTestEE()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void vTaskTestEE(void *pvParameters)
{
    // Init EEPROM
    EE_Init();
    uint16_t result;
    STD_RESULT enResult;
    uint32_t nCurrentAdr = 0;
    uint32_t nNumberOfRec = 0;

    printf("EEPROM Init\n\r");

    if (RESULT_OK == W25Q_UnLockGlobal())
    {
        taskENTER_CRITICAL();
        // Erase W25
        enResult = W25Q_EraseBlock(0, W25Q_BLOCK_MEMORY_ALL);
        taskEXIT_CRITICAL();

        if (RESULT_OK == enResult) {
            printf("W25 erased\n\r");
        } else {
            printf("W25 erased fail\n\r");
        }
    }
    result = EE_WriteVariable(EE_VER_ADR_LAST_SEND, 0x0);
    result = EE_WriteVariable(EE_VER_ADR_CURRENT_WR, 0x0);

    while(1)
    {
        for (int i=0;i<TEST_EE_QTY_FLASH_RECORDS;i++)
        {
            // Generate record data
            for (int j = 0; j < TEST_EE_SIZE_RECORD_DATA; j++)
            {
                nRecordBuf[j] = rand()%255;
            }

            // Calculate CRC8
            nRecordBuf[TEST_EE_SIZE_RECORD_DATA + TEST_EE_SIZE_CRC8 - 1] =
                    CH_SUM_CalculateCRC8(nRecordBuf,
                                         TEST_EE_SIZE_RECORD_DATA);

            // Write record data
            if (RESULT_OK == W25Q_WriteData(nCurrentAdr,nRecordBuf,TEST_EE_SIZE_RECORD_BUF))
            {
                printf("Write record OK ");
            }
            else
            {
                printf("Write FAIL");
            }
            
            nCurrentAdr += TEST_EE_SIZE_RECORD_BUF;
            nNumberOfRec++;
        }



        vTaskDelay(1000/portTICK_RATE_MS);
    }
}// end vTaskTestEE()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

// None.



//****************************************** end of file *******************************************








