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

#include "eeprom.h"

#include "printf.h"



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
#define TASK_MASTER_MUTEX_DELAY      (1000U)

// Number of entries to send to the server
#define TASK_GSM_QTY_REC_TO_SEND            (10U)


//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// Buff for record
static uint8_t TASK_MASTER_aRecord[RECORD_MAN_MAX_SIZE_RECORD];



//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

static void test_cmd1(const char* data);
static void TASK_MASTER_ReadRecordCMD(const char* data);
static void TASK_MASTER_WriteRecordCMD(const char* data);
static void TASK_MASTER_TestEECMD(const char* data);


static term_srv_cmd_t cmd_list[] = {
        { .cmd = "command1", .len = 8, .handler = test_cmd1 },
        { .cmd = "ReadRecord", .len = 10, .handler = TASK_MASTER_ReadRecordCMD },
        { .cmd = "StoreRecord", .len = 11, .handler = TASK_MASTER_WriteRecordCMD },
        { .cmd = "TestEE", .len = 6, .handler = TASK_MASTER_TestEECMD },
};



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
    W25Q_EraseBlock(0,W25Q_BLOCK_MEMORY_4KB);

    EE_WriteVariable32(RECORD_MAN_VIR_ADR32_NEXT_RECORD,
                       0);

    EE_WriteVariable32(RECORD_MAN_VIR_ADR32_LAST_RECORD,
                       0);

    // Init Terminal
    term_srv_init(INIT_TerminalSend,
                  cmd_list,
                  4);

    vTaskResume(TASK_READ_SEN_hHandlerTask);

    for(;;)
    {
        if (USART2->ISR & USART_ISR_RXNE)
        {
            uint8_t data = USART2->RDR;
            term_srv_process(data);
        }

        // Send data to server every TASK_GSM_QTY_REC_TO_SEND records
        // Attempt get mutex
        if (pdTRUE == xSemaphoreTake(RECORD_MAN_xMutex, TASK_MASTER_MUTEX_DELAY))
        {
            uint32_t nLastRecord = 0U;
            uint32_t nNextRecord = 0U;
            uint32_t nQtyBytes = 0U;

            // Get the last entry sent to the server
            EE_ReadVariable32(RECORD_MAN_VIR_ADR32_LAST_RECORD,
                              &nLastRecord);

            // Get the current record
            EE_ReadVariable32(RECORD_MAN_VIR_ADR32_NEXT_RECORD,
                              &nNextRecord);

            // Return mutex
            xSemaphoreGive(RECORD_MAN_xMutex);

            if ((nNextRecord > nLastRecord) && ((nNextRecord - nLastRecord) >= TASK_GSM_QTY_REC_TO_SEND))
            {
                vTaskResume(TASK_GSM_hHandlerTask);
            }
        }
        else
        {
            DoNothing();
        }

        vTaskDelay(2000/portTICK_RATE_MS);
    }
} // end of vTaskMaster()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      test_cmd1()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void test_cmd1(const char* data)
{
    INIT_TerminalSend("test_cmd1", strlen("test_cmd1"));
}// end of test_cmd1()



//**************************************************************************************************
// @Function      TASK_MASTER_ReadRecordCMD()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_MASTER_ReadRecordCMD(const char* data)
{
    uint32_t nRecordSize = 0U;

    // Attempt get mutex
    if (pdTRUE == xSemaphoreTake(RECORD_MAN_xMutex, TASK_MASTER_MUTEX_DELAY))
    {
        // Load record
        if (RESULT_OK == RECORD_MAN_Load(0,TASK_MASTER_aRecord,
                                         &nRecordSize))
        {
            INIT_TerminalSend("Record load successful", strlen("Record load successful"));
        }
        else
        {
            INIT_TerminalSend("Record load error", strlen("Record load error"));
        }

        // Return mutex
        xSemaphoreGive(RECORD_MAN_xMutex);
    }
    else
    {
        DoNothing();
    }
} // end of TASK_MASTER_ReadRecordCMD()



//**************************************************************************************************
// @Function      TASK_MASTER_WriteRecordCMD()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_MASTER_WriteRecordCMD(const char* data)
{

    uint8_t aDataSource[RECORD_MAN_SIZE_OF_RECORD_BYTES];
    uint32_t nQtyRecord = 0U;

    // Clear buf
    for (int i = 0; i < RECORD_MAN_SIZE_OF_RECORD_BYTES; ++i)
    {
        aDataSource[i] = 0U;
    }

    // Prepare data
    for (int i = 0; i < RECORD_MAN_SIZE_OF_RECORD_BYTES - 1U; ++i)
    {
        aDataSource[i] = rand()%255;
    }

    // Attempt get mutex
    if (pdTRUE == xSemaphoreTake(RECORD_MAN_xMutex, TASK_MASTER_MUTEX_DELAY))
    {
        if (RESULT_OK == RECORD_MAN_Store(aDataSource,
                                          RECORD_MAN_SIZE_OF_RECORD_BYTES,
                                          &nQtyRecord))
        {
            printf("Record store OK; Quantity records %d",nQtyRecord);
        }
        else
        {
            INIT_TerminalSend("Record store error", strlen("Record store error"));
        }

        // Return mutex
        xSemaphoreGive(RECORD_MAN_xMutex);
    }
    else
    {
        INIT_TerminalSend("Mutex is busy", strlen("Mutex is busy"));
    }
} // end of TASK_MASTER_WriteRecordCMD()



//**************************************************************************************************
// @Function      TASK_MASTER_TestEECMD()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_MASTER_TestEECMD(const char* data)
{
    uint32_t nAdr32Value_A = 0x00010002;
    uint32_t nValue_A = 0xBEEFDEED;

    VirtAddVarTab[0] = 0x0001;
    VirtAddVarTab[1] = 0x0002;

    // Write word in EE
    if (RESULT_OK == EE_WriteVariable32(nAdr32Value_A,
                       nValue_A))
    {
        nValue_A = 0;
        // Read word
        if (RESULT_OK == EE_ReadVariable32(nAdr32Value_A,
                                           &nValue_A))
        {
            printf("Value_A = %x",nValue_A);
        }
        else
        {
            INIT_TerminalSend("Read EE 32-bit value ERROR",
                              strlen("Read EE 32-bit value ERROR"));
        }
    }
    else
    {
        INIT_TerminalSend("Write EE 32-bit value ERROR",
                          strlen("Write EE 32-bit value ERROR"));
    }

} // end of TASK_MASTER_TestEECMD()



//****************************************** end of file *******************************************