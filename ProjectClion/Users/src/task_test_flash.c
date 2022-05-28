//**************************************************************************************************
// @Module        TASK_TEST_FLASH
// @Filename      task_test_flash.c
//--------------------------------------------------------------------------------------------------
// @Platform      STM32
//--------------------------------------------------------------------------------------------------
// @Compatible    STM32L151
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the TASK_SENSOR_READ functionality.
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

#define SIZE_BUF            (0x400UL)

// Test constants
#define TEST_FLASH_NUMBER_OF_ITERATIONS_TEST_1          (3U)

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

uint8_t dataRead[SIZE_BUF];
uint8_t dataWrite[SIZE_BUF];


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
// @Function      vTaskTestFlash()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void vTaskTestFlash(void *pvParameters)
{
    uint64_t ID;
    uint16_t ManufID;
    uint32_t adr=0;
    uint16_t nNumberOfPass=0;
    uint16_t nNumberOfFail=0;
    uint16_t nNumberOfEmpty=0;
    uint16_t nNumberOfNotEmpty=0;
    uint32_t nSizeRead=0;

    W25Q_Init();

    vTaskDelay(1000/portTICK_RATE_MS);

    W25Q_ReadUniqueID(&ID);
    W25Q_ReadManufactureID(&ManufID);

    printf("Manufacture ID %d\n\r",(uint8_t)(ManufID>>8));

    // Test 1: Erase/write/read random number of 4k sectors. Address multiple of 4K.
    printf("Start Test 1:\n\r");
    // erase/write/read/check data
    for (int i=0;i<TEST_FLASH_NUMBER_OF_ITERATIONS_TEST_1;i++)
    {
        // get address
        adr = (rand() / W25Q_QTY_SECTORS) * W25Q_CAPACITY_SECTOR_BYTES;
        printf("Address %x; ",adr);

        // Erase sector 4K
        if (RESULT_OK == W25Q_EraseBlock(adr,W25Q_BLOCK_MEMORY_4KB))
        {
            printf("Erase 4K PASS; ");
        }
        else
        {
            printf("Erase 4K FAIL; ");
        }

        // Prepare dataWrite
        for(int j=0;j<SIZE_BUF-1;j++)
        {
            dataWrite[j] = rand()%255;
        }
        dataWrite[SIZE_BUF-1] = CH_SUM_CalculateCRC8(dataWrite, SIZE_BUF-1);

        // Write sector 4K
        if (RESULT_OK == W25Q_WriteData(adr,dataWrite,SIZE_BUF))
        {
            printf("Write 4K PASS; ");
        }
        else
        {
            printf("Write 4K FAIL; ");
        }

        // Read sector 4K
        if (RESULT_OK == W25Q_ReadData(adr,dataRead, SIZE_BUF))
        {
            printf("Read 4K PASS");
        }
        else
        {
            printf("Read 4K FAIL");
        }

        // Check data
        if (dataRead[SIZE_BUF-1] == CH_SUM_CalculateCRC8(dataRead, SIZE_BUF-1))
        {
            printf("Check Sum PASS; TEST 1.%d PASS\r\n",i);
            nNumberOfPass++;
        }
        {
            printf("Check Sum FAIL; TEST 1.%d FAIL\r\n",i);
            nNumberOfFail++;
        }
    }
    printf("Finish Test 1: Tests passed %d ; Tests failed %d\n\r",nNumberOfPass,nNumberOfFail);
    printf("\r\n");
    // End Test 1

    // Test 2: Erase Chip
    // Read all sectors
    for (uint8_t step=0;step<2;step++)
    {
        adr = 0;
        nSizeRead = SIZE_BUF&(W25Q_CAPACITY_SECTOR_BYTES-1);
        nNumberOfNotEmpty = 0;

        for (int i=0;i<W25Q_QTY_SECTORS;i++)
        {
            // Read next sector 4K
            for (uint32_t cntBytes=0; cntBytes < W25Q_CAPACITY_SECTOR_BYTES;)
            {
                if (RESULT_OK == W25Q_ReadData(adr,dataRead, nSizeRead))
                {
                    for (int j=0; j<nSizeRead; j++)
                    {
                        if (0xff != dataRead[j])
                        {
                            nNumberOfNotEmpty++;
                            break;
                        }
                    }
                }
                else
                {
                    printf("Error reading sector address %x\r\n",adr);
                    break;
                }
                cntBytes += nSizeRead;
                adr += nSizeRead;
            }
        }
        printf("Number of not empty sectors  %d\r\n",nNumberOfNotEmpty);

        if (0 == step)
        {
            // Erase chip
            printf("Chip erasing ...\r\n");
            if (RESULT_OK == W25Q_EraseBlock(0,    W25Q_BLOCK_MEMORY_ALL))
            {
                printf("Chip erasing was successful\r\n");
            }
            else
            {
                printf("Chip erase error\r\n");
            }
        }
    }
    if ( 0 == nNumberOfNotEmpty )
    {
        printf("Finish Test 2: Tests passed \n\r");
    }
    else
    {
        printf("Finish Test 2: Tests failed; Number of not empty sectors %d\n\r",nNumberOfNotEmpty);
    }

    printf("\r\n");
    // End Test 2


    while(1)
    {
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}// end of vTaskSensorsRead



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      AM2305_DQLow()
//--------------------------------------------------------------------------------------------------
// @Description   Set low level on DQ pin
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************

// None.

//****************************************** end of file *******************************************








