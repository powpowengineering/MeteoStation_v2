//**************************************************************************************************
// @Module        TERMINAL
// @Filename      task_terminal.c
//--------------------------------------------------------------------------------------------------
// @Platform      PLATFORM_NAME
//--------------------------------------------------------------------------------------------------
// @Compatible    COMPATIBLE_PROCESSOR_MODULE
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the MODULE functionality.
//                [Description...]
//
//                Abbreviations:
//                  ABBR0 -
//                  ABBR1 -
//
//                Global (public) functions:
//                  MODULE_functionZero()
//                  MODULE_functionOne()
//
//                Local (private) functions:
//                  MODULE_functionTwo()
//                  MODULE_functionThree()
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Native header
#include <W25Q_drv.h>
#include "task_terminal.h"

#include "term-srv.h"

#include "printf.h"

// STD lib
#include "string.h"
#include "stdlib.h"

#include "Init.h"
#include "time_drv.h"
#include "circ_buffer.h"
#include "stm32l4xx_ll_usart.h"
#include "record_manager.h"
#include "eeprom_emulation.h"


//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// Terminal handler
TaskHandle_t TASK_TERMINAL_hHandlerTask;

uint32_t TASK_TERMINAL_nTimeOut;



//**************************************************************************************************
// Declarations of local (private) data types
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

#define TASK_TERMINAL_SIZE_BUF              (128U)



//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

static void test_cmd1(const char* data);
static void TASK_MASTER_ReadRecordCMD(const char* data);
static void TASK_MASTER_WriteRecordCMD(const char* data);
static void TASK_MASTER_TestEECMD(const char* data);
static void TASK_MASTER_SetDate(const char* data);
static void TASK_MASTER_SetTime(const char* data);
static void TASK_MASTER_SetPeriodAlarmSens(const char* data);
static void TASK_MASTER_SetPeriodAlarmGSM(const char* data);
static void TASK_MASTER_ClearFlash(const char* data);

static term_srv_cmd_t cmd_list[] = {
        { .cmd = "command1", .len = 8, .handler = test_cmd1 },
        { .cmd = "ReadRecord", .len = 10, .handler = TASK_MASTER_ReadRecordCMD },
        { .cmd = "StoreRecord", .len = 11, .handler = TASK_MASTER_WriteRecordCMD },
        { .cmd = "TestEE", .len = 6, .handler = TASK_MASTER_TestEECMD },
        { .cmd = "SetDate", .len = 7, .handler = TASK_MASTER_SetDate },
        { .cmd = "SetTime", .len = 7, .handler = TASK_MASTER_SetTime },
        { .cmd = "AlarmSens", .len = 9, .handler = TASK_MASTER_SetPeriodAlarmSens },
        { .cmd = "AlarmGSM", .len = 8, .handler = TASK_MASTER_SetPeriodAlarmGSM },
        { .cmd = "FlashErase", .len = 10, .handler = TASK_MASTER_ClearFlash }
};

static stCIRCBUF stCirBuf;
static U8 pBuff[TASK_TERMINAL_SIZE_BUF];



//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// Send data to terminal
static void TASK_TERMINAL_Send(const char* data, int16_t size);




//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      vTaskTerminal()
//--------------------------------------------------------------------------------------------------
// @Description   [description...]
//--------------------------------------------------------------------------------------------------
// @Notes
//--------------------------------------------------------------------------------------------------
// @ReturnValue   returnValue - [description...]
//--------------------------------------------------------------------------------------------------
// @Parameters    parameterZero - [description...]
//**************************************************************************************************
void vTaskTerminal(void *pvParameters)
{

    // Init Terminal
    term_srv_init(TASK_TERMINAL_Send,
                  cmd_list,
                  sizeof (cmd_list) / sizeof (term_srv_cmd_t));

    TASK_TERMINAL_nTimeOut = 60U * 1000U;

    stCirBuf.itemSize = 1U;

    // Init Cir buffer
    CIRCBUF_Init(&stCirBuf,
                 pBuff,
                 TASK_TERMINAL_SIZE_BUF);

    for(;;)
    {
        if (0U != CIRCBUF_GetNumberOfItems(&stCirBuf))
        {
            S8 data = 0;
            CIRCBUF_GetData(&data, &stCirBuf);
            term_srv_process(data);
        }
        else
        {
            DoNothing();
        }

        TASK_TERMINAL_nTimeOut += TASK_TERMINAL_DELAY;
        vTaskDelay(TASK_TERMINAL_DELAY/portTICK_RATE_MS);
    }
} // end of vTaskTerminal()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      TASK_TERMINAL_Send()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_TERMINAL_Send(const char* data, int16_t size)
{
    while (size)
    {
        while ((INIT_TERMINAL_USART_NUM->ISR & USART_ISR_TXE) != USART_ISR_TXE);
        INIT_TERMINAL_USART_NUM->TDR = *data;
        ++data;
        --size;
    }
} // end of TASK_TERMINAL_Send()



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
    TASK_TERMINAL_Send("test_cmd1", strlen("test_cmd1"));
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
//    uint32_t nRecordSize = 0U;
//
//    // Attempt get mutex
//    if (pdTRUE == xSemaphoreTake(RECORD_MAN_xMutex, TASK_MASTER_MUTEX_DELAY))
//    {
//        // Load record
//        if (RESULT_OK == RECORD_MAN_Load(0,TASK_MASTER_aRecord,
//                                         &nRecordSize))
//        {
//            INIT_TerminalSend("Record load successful", strlen("Record load successful"));
//        }
//        else
//        {
//            INIT_TerminalSend("Record load error", strlen("Record load error"));
//        }
//
//        // Return mutex
//        xSemaphoreGive(RECORD_MAN_xMutex);
//    }
//    else
//    {
//        DoNothing();
//    }
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

//    uint8_t aDataSource[RECORD_MAN_SIZE_OF_RECORD_BYTES];
//    uint32_t nQtyRecord = 0U;
//
//    // Clear buf
//    for (int i = 0; i < RECORD_MAN_SIZE_OF_RECORD_BYTES; ++i)
//    {
//        aDataSource[i] = 0U;
//    }
//
//    // Prepare data
//    for (int i = 0; i < RECORD_MAN_SIZE_OF_RECORD_BYTES - 1U; ++i)
//    {
//        aDataSource[i] = rand()%255;
//    }
//
//    // Attempt get mutex
//    if (pdTRUE == xSemaphoreTake(RECORD_MAN_xMutex, TASK_MASTER_MUTEX_DELAY))
//    {
//        if (RESULT_OK == RECORD_MAN_Store(aDataSource,
//                                          RECORD_MAN_SIZE_OF_RECORD_BYTES,
//                                          &nQtyRecord))
//        {
//            printf("Record store OK; Quantity records %d",nQtyRecord);
//        }
//        else
//        {
//            INIT_TerminalSend("Record store error", strlen("Record store error"));
//        }
//
//        // Return mutex
//        xSemaphoreGive(RECORD_MAN_xMutex);
//    }
//    else
//    {
//        INIT_TerminalSend("Mutex is busy", strlen("Mutex is busy"));
//    }
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
//    uint32_t nAdr32Value_A = 0x00010002;
//    uint32_t nValue_A = 0xBEEFDEED;
//
//    VirtAddVarTab[0] = 0x0001;
//    VirtAddVarTab[1] = 0x0002;
//
//    // Write word in EE
//    if (RESULT_OK == EE_WriteVariable32(nAdr32Value_A,
//                       nValue_A))
//    {
//        nValue_A = 0;
//        // Read word
//        if (RESULT_OK == EE_ReadVariable32(nAdr32Value_A,
//                                           &nValue_A))
//        {
//            printf("Value_A = %x",nValue_A);
//        }
//        else
//        {
//            INIT_TerminalSend("Read EE 32-bit value ERROR",
//                              strlen("Read EE 32-bit value ERROR"));
//        }
//    }
//    else
//    {
//        INIT_TerminalSend("Write EE 32-bit value ERROR",
//                          strlen("Write EE 32-bit value ERROR"));
//    }

} // end of TASK_MASTER_TestEECMD()



//**************************************************************************************************
// @Function      TASK_MASTER_SetDate()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_MASTER_SetDate(const char* data)
{
    // Get parameter
    char* pArg = memchr(data,' ',strlen(data));
    char* pEnd;
    int day = 0U;
    int month = 0U;
    int year = 0U;

    TIME_type time;

    // Check parameter
    if ((NULL != pArg) && (2 < strlen(pArg)))
    {
        day = strtol(pArg,&pEnd,10U);
        month = strtol(pEnd + 1U,&pEnd,10U);
        year = strtol(pEnd + 1U,&pEnd,10U);

        if ((0U == day) || (0U == month) || (0U == year))
        {
            TASK_TERMINAL_Send("Parameter ERROR",strlen("Parameter ERROR"));
        }
        else
        {
            // Set date
            time.tm_year = year;
            time.tm_mon = month;
            time.tm_mday = day;

            TIME_SetDate(time);

            TASK_TERMINAL_Send(pArg + 1U, strlen(pArg));
        }
    }
    else
    {
        TASK_TERMINAL_Send("Parameter ERROR",strlen("Parameter ERROR"));
    }
} // end of TASK_MASTER_SetDate()



//**************************************************************************************************
// @Function      TASK_MASTER_SetTime()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_MASTER_SetTime(const char* data)
{
    // Get parameter
    char* pArg = memchr(data,' ',strlen(data));
    char* pEnd;
    int seconds = 0U;
    int minutes = 0U;
    int hour = 0U;

    TIME_type time;

    // Check parameter
    if ((NULL != pArg) && (2 < strlen(pArg)))
    {
        hour = strtol(pArg,&pEnd,10U);
        minutes = strtol(pEnd + 1U,&pEnd,10U);
        seconds = strtol(pEnd + 1U,&pEnd,10U);

//        if ((0U == seconds) || (0U == month) || (0U == year))
//        {
//            TASK_TERMINAL_Send("Parameter ERROR",strlen("Parameter ERROR"));
//        }
//        else
//        {
            // Set date
            time.tm_sec= seconds;
            time.tm_min= minutes;
            time.tm_hour = hour;

            TIME_SetTime(time);

            TASK_TERMINAL_Send(pArg + 1U, strlen(pArg));
//        }
    }
    else
    {
        TASK_TERMINAL_Send("Parameter ERROR",strlen("Parameter ERROR"));
    }
} // end of TASK_MASTER_SetTime()



//**************************************************************************************************
// @Function      TASK_MASTER_SetPeriodAlarmSens()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_MASTER_SetPeriodAlarmSens(const char* data)
{
    // Get parameter
    char* pArg = memchr(data,' ',strlen(data));
    char* pEnd;
    int hour = 0U;
    int minutes = 0U;
    int seconds = 0U;

    TIME_type time;

    // Check parameter
    if ((NULL != pArg) && (2 < strlen(pArg)))
    {
        hour = strtol(pArg,&pEnd,10U);
        minutes = strtol(pEnd + 1U,&pEnd,10U);
        seconds = strtol(pEnd + 1U,&pEnd,10U);

        if ((59U < hour) || (59U < minutes) || (59U < seconds))
        {
            TASK_TERMINAL_Send("Parameter ERROR",strlen("Parameter ERROR"));
        }
        else
        {
            // Set alarm
            time.tm_hour = hour;
            time.tm_min = minutes;
            time.tm_sec = seconds;

            // Store new alarm value
            TIME_StoreAlarm(time, TIME_ALARM_SENS);

            // Set new alarm value
            TIME_SetAlarm(time, TIME_ALARM_SENS);

            TASK_TERMINAL_Send(pArg + 1U, strlen(pArg));
        }
    }
    else
    {
        TASK_TERMINAL_Send("Parameter ERROR",strlen("Parameter ERROR"));
    }
}// end TASK_MASTER_SetPeriodAlarmSens()



//**************************************************************************************************
// @Function      TASK_MASTER_SetPeriodAlarmGSM()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_MASTER_SetPeriodAlarmGSM(const char* data)
{
    // Get parameter
    char* pArg = memchr(data,' ',strlen(data));
    char* pEnd;
    int hour = 0U;
    int minutes = 0U;
    int seconds = 0U;

    TIME_type time;

    // Check parameter
    if ((NULL != pArg) && (2 < strlen(pArg)))
    {
        hour = strtol(pArg,&pEnd,10U);
        minutes = strtol(pEnd + 1U,&pEnd,10U);
        seconds = strtol(pEnd + 1U,&pEnd,10U);

        if ((59U < hour) || (59U < minutes) || (59U < seconds))
        {
            TASK_TERMINAL_Send("Parameter ERROR",strlen("Parameter ERROR"));
        }
        else
        {
            // Set alarm
            time.tm_hour = hour;
            time.tm_min = minutes;
            time.tm_sec = seconds;

            // Store new alarm value
            TIME_StoreAlarm(time, TIME_ALARM_GSM);

            // Set new alarm value
            TIME_SetAlarm(time, TIME_ALARM_GSM);

            TASK_TERMINAL_Send(pArg + 1U, strlen(pArg));
        }
    }
    else
    {
        TASK_TERMINAL_Send("Parameter ERROR",strlen("Parameter ERROR"));
    }
} // end of TASK_MASTER_SetPeriodAlarmGSM()



//**************************************************************************************************
// @Function      TASK_MASTER_SetPeriodAlarmGSM()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_MASTER_ClearFlash(const char* data)
{
    U32 AdrNext = 0U;
    STD_RESULT enResult = RESULT_OK;
    // Write new alarm value in EEPROM
    // Attempt get mutex
    if (pdTRUE == xSemaphoreTake(RECORD_MAN_xMutex, 10000 / portTICK_PERIOD_MS))
    {

        if (RESULT_OK == EMEEP_Store(RECORD_MAN_VIR_ADR32_NEXT_RECORD,
                                     (U8*)&(AdrNext),
                                     RECORD_MAN_SIZE_VIR_ADR))
        {
            printf("task_terminal: AdrNext was clear\r\n");
        }
        else
        {
            printf("task_terminal ERROR: AdrNext wasn't cleare\r\n");
        }

        // Return mutex
        xSemaphoreGive(RECORD_MAN_xMutex);
    }
    else
    {
        printf("task_terminal: Mutex of record manager is busy\r\n");
    }

    // Clear FLASH
    taskENTER_CRITICAL();
    for (int nNumberSector = 0;
         nNumberSector < (W25Q_CAPACITY_ALL_MEMORY_BYTES - (3U * W25Q_CAPACITY_SECTOR_BYTES)) / W25Q_CAPACITY_SECTOR_BYTES;
         nNumberSector++)
    {
        U32 adr = nNumberSector * W25Q_CAPACITY_SECTOR_BYTES;
        if (RESULT_OK == W25Q_EraseBlock(adr , W25Q_BLOCK_MEMORY_4KB))
        {
            DoNothing();
        }
        else
        {
            printf("task_terminal: ERROR Erase\r\n");
            enResult = RESULT_NOT_OK;
        }
    }
    taskEXIT_CRITICAL();

    if (RESULT_OK == enResult)
    {
        printf("FLASH erased successfully\r\n");
    }
    else
    {
        printf("FLASH erased ERROR\r\n");
    }

}




void USART2_IRQHandler(void)
{
    if (INIT_TERMINAL_USART_NUM->ISR & USART_ISR_RXNE)
    {
        TASK_TERMINAL_nTimeOut = 0U;
        uint8_t data = USART2->RDR;
        CIRCBUF_PutData(&data, &stCirBuf);
    }
    LL_USART_ClearFlag_ORE(INIT_TERMINAL_USART_NUM);

}




//****************************************** end of file *******************************************
