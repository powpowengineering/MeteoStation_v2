#include <argz.h>
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

#include "task_test_EE.h"
// drivers
#include "W25Q_drv.h"
#include "checksum.h"
#include "printf.h"
#include "stdlib.h"
#include "ftoa.h"
#include "eeprom.h"
#include "W25Q_drv.h"
#include "cmsis_os.h"


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

#define TEST_EE_SIZE_READ_BUF           (64U)


//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

uint8_t nRecordBuf[TEST_EE_SIZE_RECORD_BUF];
uint8_t nReadBuf[TEST_EE_SIZE_READ_BUF];

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
    uint64_t ID;
    uint16_t ManufID;
    // Init EEPROM
    W25Q_Init();
    W25Q_ReadUniqueID(&ID);
    W25Q_ReadManufactureID(&ManufID);
    W25Q_UnLockGlobal();

    W25Q_EraseBlock(PAGE0_BASE_ADDRESS, W25Q_BLOCK_MEMORY_4KB);
    W25Q_EraseBlock(PAGE1_BASE_ADDRESS, W25Q_BLOCK_MEMORY_4KB);

    EE_Init();
    uint16_t result;
    uint8_t nWriteDataTest = 0;
    uint32_t nCurrentAdr = 0;
    uint32_t nNumberOfRec = 0;

    // Init virtual address table EE
    for (int i = 0; i < NB_OF_VAR; i++)
    {
        VirtAddVarTab[i] = i;
    }



    printf("EEPROM Init\n\r");

//    if (RESULT_OK == W25Q_UnLockGlobal())
//    {
//        taskENTER_CRITICAL();
//        // Erase W25
//        enResult = W25Q_EraseBlock(0, W25Q_BLOCK_MEMORY_ALL);
//        taskEXIT_CRITICAL();
//
//        if (RESULT_OK == enResult) {
//            printf("W25 erased\n\r");
//        } else {
//            printf("W25 erased fail\n\r");
//        }
//    }
//    result = EE_WriteVariable(EE_VER_ADR_LAST_SEND, 0x0);
//    result = EE_WriteVariable(EE_VER_ADR_CURRENT_WR, 0x0);

    while(1)
    {
//        for (int i=0;i<TEST_EE_QTY_FLASH_RECORDS;i++)
//        {
//            // Generate record data
//            for (int j = 0; j < TEST_EE_SIZE_RECORD_DATA; j++)
//            {
//                nRecordBuf[j] = rand()%255;
//            }
//
//            // Calculate CRC8
//            nRecordBuf[TEST_EE_SIZE_RECORD_DATA + TEST_EE_SIZE_CRC8 - 1] =
//                    CH_SUM_CalculateCRC8(nRecordBuf,
//                                         TEST_EE_SIZE_RECORD_DATA);
//
//            // Write record data
//            if (RESULT_OK == W25Q_WriteData(nCurrentAdr,nRecordBuf,TEST_EE_SIZE_RECORD_BUF))
//            {
//                printf("Write record OK ");
//            }
//            else
//            {
//                printf("Write FAIL");
//            }
//
//            nCurrentAdr += TEST_EE_SIZE_RECORD_BUF;
//            nNumberOfRec++;
//        }
        uint16_t resultEEWrite = 0;
        for (int i = 0; i < NB_OF_VAR; i++)
        {
            resultEEWrite = EE_WriteVariable(i, i);

            switch(resultEEWrite)
            {
                case (PAGE_FULL):
                    printf("PAGE_FULL\r\n");
                    break;
                case (NO_VALID_PAGE):
                    printf("NO_VALID_PAGE\r\n");
                    break;
                case (HAL_OK):
//                    printf("FLASH_COMPLETE\r\n");
                    break;
                case (HAL_ERROR):
                    printf("FLASH_ERROR\r\n");
                    break;
                case (HAL_BUSY):
                    printf("FLASH_BUSY\r\n");
                    break;
                case (HAL_TIMEOUT):
                    printf("FLASH_TIMEOUT\r\n");
                    break;
                default:break;
            }
        }


        // Read PAGE0 EE
        printf("Page0\r\n");
        uint32_t nAdr = PAGE0_BASE_ADDRESS;
        while (nAdr < PAGE0_BASE_ADDRESS + PAGE_SIZE)
        {
            if (RESULT_OK == W25Q_ReadData(nAdr,nReadBuf, TEST_EE_SIZE_READ_BUF))
            {
                uint32_t nWord = 0;
                for (int j = 0; j < TEST_EE_SIZE_READ_BUF; j++)
                {
                    nWord |= (uint32_t)nReadBuf[j] << ((j%4) * 8);
                    if ((j%4 == 0x0) && (j != 0))
                    {
                        printf("|");
                        printf("%08x",nWord);
                        nWord = 0;
                    }

                }
                printf("\r\n");
            }
            else
            {
                printf("Error read\r\n");
            }
            nAdr += TEST_EE_SIZE_READ_BUF;
        }

        printf("\r\n\r\nPage1\r\n");

        // Read PAGE1 EE
         nAdr = PAGE1_BASE_ADDRESS;
        while (nAdr < PAGE1_BASE_ADDRESS + PAGE_SIZE)
        {
            if (RESULT_OK == W25Q_ReadData(nAdr,nReadBuf, TEST_EE_SIZE_READ_BUF))
            {
                uint32_t nWord = 0;
                for (int j = 0; j < TEST_EE_SIZE_READ_BUF; j++)
                {
                    nWord |= (uint32_t)nReadBuf[j] << ((j%4) * 8);
                    if ((j%4 == 0x0) && (j != 0))
                    {
                        printf("|");
                        printf("%08x",nWord);
                        nWord = 0;
                    }

                }
                printf("\r\n");
            }
            else
            {
                printf("Error read\r\n");
            }
            nAdr += TEST_EE_SIZE_READ_BUF;
        }
        vTaskDelay(100/portTICK_RATE_MS);
    }
}// end vTaskTestEE()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

// None.



//****************************************** end of file *******************************************








