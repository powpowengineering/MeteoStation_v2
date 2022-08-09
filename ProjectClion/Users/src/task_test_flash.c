#include <argz.h>
//**************************************************************************************************
// @Module        TASK_TEST_FLASH
// @Filename      task_test_flash.c
//--------------------------------------------------------------------------------------------------
// @Platform      STM32
//--------------------------------------------------------------------------------------------------
// @Compatible    STM32L4xx
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

#include "cmsis_os.h"
#include "eeprom.h"



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

// Type of meteo data. Only 32-bit data!!!
typedef struct TEST_EE_METEO_DATA_struct
{
    uint32_t nTimeUnixTime;
    float fTemperature;
    float fPressure;
    float fHumidity;
    float fWindSpeed;
    float dVoltageBattery;
}TEST_EE_METEO_DATA;



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

#define SIZE_BUF            (64UL)

// Test constants
#define TEST_FLASH_NUMBER_OF_ITERATIONS_TEST_1          (10U)

// Virtual address
#define EE_VER_ADR_LSB_LAST_RECORD           (uint16_t)(0xCC)
#define EE_VER_ADR_MSB_LAST_RECORD           (uint16_t)(0xDD)
#define EE_VER_ADR_LSB_NEXT_RECORD           (uint16_t)(0xAA)
#define EE_VER_ADR_MSB_NEXT_RECORD           (uint16_t)(0xBB)

// Header record
#define TEST_FLASH_RECORD_HEADER_MARKER           (0x10U)
// End marker record
#define TEST_FLASH_RECORD_END_MARKER              (0x03U)
// Max record package in bytes
#define RECORD_MAN_MAX_SIZE_RECORD                 (100U)
// Quantity records to send to server
#define TEST_FLASH_QTY_REC_SEND_SERVER             (10U)

// State machine
#define TEST_FLASH_STATE_PARSING_START_MARKER      (0U)
#define TEST_FLASH_STATE_PARSING_PAYLOAD           (1U)
#define TEST_FLASH_STATE_PARSING_BYTE_STUFFING     (2U)
#define TEST_FLASH_STATE_PARSING_END_MARKER        (3U)

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

static uint8_t dataRead[SIZE_BUF];
static uint8_t dataWrite[SIZE_BUF];

TEST_EE_METEO_DATA TEST_EE_MeteoData;
uint8_t aRecordDataPackage[RECORD_MAN_MAX_SIZE_RECORD];
uint8_t aRecordReadBack[RECORD_MAN_MAX_SIZE_RECORD];
uint32_t nSizeDataRecord;
RTC_HandleTypeDef stRTC;

//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// Create record package
static STD_RESULT TEST_FLASH_CreateRecord(const TEST_EE_METEO_DATA *pMeteoData,
                                       uint8_t *pDataRecord,
                                       uint32_t *pSizeDataRecord);

static STD_RESULT TEST_FLASH_ParsingRecord(TEST_EE_METEO_DATA *pMeteoData,
                                           const uint8_t *pDataRecord,
                                           uint32_t pSizeDataRecord);

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
    // Unlock global
    if (RESULT_OK == W25Q_UnLockGlobal())
    {
        printf("Unlock global was successful\r\n");
    }
    else
    {
        printf("Unlock global failed\r\n");
    }

    for (int i=0;i<TEST_FLASH_NUMBER_OF_ITERATIONS_TEST_1;i++)
    {
        // get address
        adr = ((rand()%W25Q_CAPACITY_ALL_MEMORY_BYTES) / W25Q_QTY_SECTORS) * W25Q_CAPACITY_SECTOR_BYTES;

        printf("Address %x; ",adr);

        uint8_t lock=0;
        // Check lock
        if (RESULT_OK == W25Q_GetLock(adr,&lock))
        {
            if ((lock & 0x01) == 0x01)
            {
                printf("Block locked; ");
            }
            else
            {
                printf("Block Unlocked; ");
            }
        }
        else
        {
            printf("Error Read; ");
        }


        // Erase sector 4K
        if (RESULT_OK == W25Q_EraseBlock(adr,W25Q_BLOCK_MEMORY_4KB))
        {
            printf("Erase 4K PASS; ");
        }
        else
        {
            printf("Erase 4K BREAK; ");
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
            printf("Write 4K BREAK; ");
        }

        // Read sector 4K
        if (RESULT_OK == W25Q_ReadData(adr,dataRead, SIZE_BUF))
        {
            printf("Read 4K PASS; ");
        }
        else
        {
            printf("Read 4K BREAK; ");
        }

        // Check data
        if (dataRead[SIZE_BUF-1] == CH_SUM_CalculateCRC8(dataRead, SIZE_BUF-1))
        {
            printf("Check Sum PASS; TEST 1.%d PASS\r\n",i);
            nNumberOfPass++;
        }
        else
        {
            printf("Check Sum FAIL; TEST 1.%d FAIL\r\n",i);
            nNumberOfFail++;
        }
    }
    printf("Finished Test 1: Tests passed %d ; Tests failed %d\n\r",nNumberOfPass,nNumberOfFail);
    printf("\r\n");
    // End Test 1

    // Test 2: Erase Chip
    printf("Test 2 started\r\n");
    printf("Reading all memory\r\n");
    // Read all sectors
    uint32_t nNumberReadBytes=0;
    for (uint8_t step=0;step<2;step++)
    {
        adr = 0;
        nSizeRead = SIZE_BUF & (W25Q_CAPACITY_SECTOR_BYTES-1);
        nNumberOfNotEmpty = 0;
        nNumberReadBytes = 0;

        for (int i=0;i<W25Q_QTY_SECTORS;i++)
        {
            // Read next sector 4K
            for (uint32_t cntBytes=0; cntBytes < W25Q_CAPACITY_SECTOR_BYTES;)
            {
                if (RESULT_OK == W25Q_ReadData(adr,dataRead, nSizeRead))
                {
                    nNumberReadBytes += nSizeRead;
                    printf(" \r");
                    printf("Read %d %% bytes\r",(nNumberReadBytes*100) / W25Q_CAPACITY_ALL_MEMORY_BYTES);
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
        printf("\n\r");
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
// @Function      vTaskTestFlashWithEE()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
_Noreturn void vTaskTestFlashWithEE(void *pvParameters)
{
    uint32_t nAdrNextRecord = 0U;
    uint32_t nAdrLastRecordSendGSM = 0U;
    uint32_t nNumberRecordStore = 0U;

    uint64_t ID;
    uint16_t ManufID;

    // RTC Init
//    stRTC.Instance = RTC;
//    stRTC.Init.HourFormat = RTC_HOURFORMAT_24;
//    stRTC.Init.AsynchPrediv = 0x7F;
//    stRTC.Init.SynchPrediv = 0xFF;
//
//
//
//    if (HAL_OK == HAL_RTC_Init(RTC_HandleTypeDef *hrtc))
//    {
//        printf("RTC init OK\r\n");
//    }
//    else
//    {
//        printf("RTC init ERROR\r\n");
//    }

    // Init EEPROM
    W25Q_Init();
    W25Q_ReadUniqueID(&ID);
    W25Q_ReadManufactureID(&ManufID);
    W25Q_UnLockGlobal();
    W25Q_EraseBlock(PAGE0_BASE_ADDRESS, W25Q_BLOCK_MEMORY_4KB);
    W25Q_EraseBlock(PAGE1_BASE_ADDRESS, W25Q_BLOCK_MEMORY_4KB);
    W25Q_EraseBlock(0, W25Q_BLOCK_MEMORY_64KB);

    EE_Init();
    VirtAddVarTab[0] = EE_VER_ADR_LSB_LAST_RECORD;
    VirtAddVarTab[1] = EE_VER_ADR_MSB_LAST_RECORD;
    VirtAddVarTab[2] = EE_VER_ADR_LSB_NEXT_RECORD;
    VirtAddVarTab[3] = EE_VER_ADR_MSB_NEXT_RECORD;

    EE_WriteVariable(EE_VER_ADR_LSB_NEXT_RECORD, (uint16_t)nAdrNextRecord);
    EE_WriteVariable(EE_VER_ADR_MSB_NEXT_RECORD, (uint16_t)(nAdrNextRecord >> 16));
    EE_WriteVariable(EE_VER_ADR_LSB_LAST_RECORD, (uint16_t)nAdrLastRecordSendGSM);
    EE_WriteVariable(EE_VER_ADR_MSB_LAST_RECORD, (uint16_t)(nAdrLastRecordSendGSM >> 16));

    while(1)
    {
        // Prepare test data
        TEST_EE_MeteoData.fTemperature = ((float)rand()/(float)(RAND_MAX)) * 5;
        TEST_EE_MeteoData.fHumidity = ((float)rand()/(float)(RAND_MAX)) * 5;
        TEST_EE_MeteoData.fPressure = ((float)rand()/(float)(RAND_MAX)) * 5;
        TEST_EE_MeteoData.fWindSpeed = ((float)rand()/(float)(RAND_MAX)) * 5;
        TEST_EE_MeteoData.dVoltageBattery = ((float)rand()/(float)(RAND_MAX)) * 5;
        TEST_EE_MeteoData.nTimeUnixTime = rand();

        // Clear buffer
        for(int i = 0; i < RECORD_MAN_MAX_SIZE_RECORD; i++)
        {
            aRecordDataPackage[i] = 0;
        }

        if (RESULT_OK == TEST_FLASH_CreateRecord(&TEST_EE_MeteoData,
                                              aRecordDataPackage,
                                              &nSizeDataRecord))
        {
            uint16_t nTemp = 0;
            EE_ReadVariable(EE_VER_ADR_LSB_NEXT_RECORD,
                            &nTemp);

            nAdrNextRecord = nTemp;

            EE_ReadVariable(EE_VER_ADR_MSB_NEXT_RECORD,
                            &nTemp);

            nAdrNextRecord |= (((uint32_t)nTemp) << 16) & 0xFFFF0000;

            EE_ReadVariable(EE_VER_ADR_LSB_LAST_RECORD,
                            &nTemp);

            nAdrLastRecordSendGSM = nTemp;

            EE_ReadVariable(EE_VER_ADR_MSB_LAST_RECORD,
                            &nTemp);

            nAdrLastRecordSendGSM |= (((uint32_t)nTemp) << 16) & 0xFFFF0000;

            // Write record in flash
            if (RESULT_OK == W25Q_WriteData(nAdrNextRecord,
                                            aRecordDataPackage,
                                            nSizeDataRecord))
            {
                printf("Record was written in flash\r\n");
                nNumberRecordStore++;
                printf("Number of records saved %d\r\n",nNumberRecordStore);
            }
            else
            {
                printf("Record wasn't written in flash\r\n");
            }

            // Read record from flash
            if (RESULT_OK == W25Q_ReadData(nAdrNextRecord,
                                           aRecordReadBack,
                                           RECORD_MAN_MAX_SIZE_RECORD))
            {
                printf("Flash read OK\r\n");

                if (RESULT_OK == TEST_FLASH_ParsingRecord(NULL,
                                                          aRecordReadBack,
                                                          RECORD_MAN_MAX_SIZE_RECORD))
                {
                    printf("Record read OK\r\n");
                }
                else
                {
                    printf("record read error\r\n");
                }
            }
            else
            {
                printf("Flash read error\r\n");

            }


            nAdrNextRecord += nSizeDataRecord;

            // Check the number of records that were not sent to the server
            if (TEST_FLASH_QTY_REC_SEND_SERVER <= ((nAdrNextRecord - nAdrLastRecordSendGSM) / RECORD_MAN_MAX_SIZE_RECORD))
            {
                printf("Send data to the server\r\n");
                nAdrLastRecordSendGSM = nAdrNextRecord;
            }
            else
            {
                printf("Number of records that were not sent to the server %d\r\n",nAdrNextRecord - nAdrLastRecordSendGSM);
            }
        }
        else
        {
            printf("Record package didn't create\r\n");
        }

        EE_WriteVariable(EE_VER_ADR_LSB_NEXT_RECORD, (uint16_t)nAdrNextRecord);
        EE_WriteVariable(EE_VER_ADR_MSB_NEXT_RECORD, (uint16_t)(nAdrNextRecord >> 16));
        EE_WriteVariable(EE_VER_ADR_LSB_LAST_RECORD, (uint16_t)nAdrLastRecordSendGSM);
        EE_WriteVariable(EE_VER_ADR_MSB_LAST_RECORD, (uint16_t)(nAdrLastRecordSendGSM >> 16));


        // Read PAGE0 EE
        printf("Page0\r\n");
        uint32_t nAdr = PAGE0_BASE_ADDRESS;
        while (nAdr < PAGE0_BASE_ADDRESS + PAGE_SIZE)
        {
            if (RESULT_OK == W25Q_ReadData(nAdr,dataRead, SIZE_BUF))
            {
                uint32_t nWord = 0;
                for (int j = 0; j < SIZE_BUF; j++)
                {
                    nWord |= (uint32_t)dataRead[j] << ((j%4) * 8);
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
            nAdr += SIZE_BUF;
        }

        printf("\r\n\r\nPage1\r\n");

        // Read PAGE1 EE
        nAdr = PAGE1_BASE_ADDRESS;
        while (nAdr < PAGE1_BASE_ADDRESS + PAGE_SIZE)
        {
            if (RESULT_OK == W25Q_ReadData(nAdr,dataRead, SIZE_BUF))
            {
                uint32_t nWord = 0;
                for (int j = 0; j < SIZE_BUF; j++)
                {
                    nWord |= (uint32_t)dataRead[j] << ((j%4) * 8);
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
            nAdr += SIZE_BUF;
        }



        vTaskDelay(1000/portTICK_RATE_MS);
    }


} // end of vTaskTestFlashWithEE()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      TEST_FLASH_CreateRecord()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT TEST_FLASH_CreateRecord(const TEST_EE_METEO_DATA *pMeteoData,
                                        uint8_t *pDataRecord,
                                        uint32_t *pSizeDataRecord)
{
    STD_RESULT enResult = RESULT_OK;
    uint32_t nSizeRecord = 0;
    uint8_t *pMeteoDataByte = (uint8_t*)pMeteoData;

    // Write header
    pDataRecord[nSizeRecord] = (uint8_t)TEST_FLASH_RECORD_HEADER_MARKER;
    nSizeRecord++;

    for (int i = 0; i < sizeof (TEST_EE_METEO_DATA) / sizeof (uint8_t); i++)
    {
        if (RECORD_MAN_MAX_SIZE_RECORD > nSizeRecord)
        {
            pDataRecord[nSizeRecord] = *pMeteoDataByte;
            nSizeRecord++;

        }
        else
        {
            enResult = RESULT_NOT_OK;
            break;
        }

        if (RECORD_MAN_MAX_SIZE_RECORD > nSizeRecord)
        {
            // Byte stuffing
            if (TEST_FLASH_RECORD_HEADER_MARKER == *pMeteoDataByte)
            {
                pDataRecord[nSizeRecord] = (uint8_t)TEST_FLASH_RECORD_HEADER_MARKER;
                nSizeRecord++;
            }
            else if  (TEST_FLASH_RECORD_END_MARKER == *pMeteoDataByte)
            {
                pDataRecord[nSizeRecord] = (uint8_t)TEST_FLASH_RECORD_END_MARKER;
                nSizeRecord++;
            }
        }
        else
        {
            enResult = RESULT_NOT_OK;
            break;
        }
        pMeteoDataByte++;
    }

    if ((RESULT_OK == enResult) && (RECORD_MAN_MAX_SIZE_RECORD > (nSizeRecord + 2)))
    {
        pDataRecord[nSizeRecord] = CH_SUM_CalculateCRC8(pDataRecord,
                                                        nSizeRecord-1);

        nSizeRecord++;
        pDataRecord[nSizeRecord] = TEST_FLASH_RECORD_END_MARKER;
    }

    *pSizeDataRecord = nSizeRecord + 1;

    return enResult;
} // end of TEST_FLASH_CreateRecord()



//**************************************************************************************************
// @Function      TEST_FLASH_ParsingRecord()
//--------------------------------------------------------------------------------------------------
// @Description   This function finds package and calculates crc8.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT TEST_FLASH_ParsingRecord(TEST_EE_METEO_DATA *pMeteoData,
                                           const uint8_t *pDataRecord,
                                           uint32_t pSizeDataRecord)
{
    STD_RESULT enResult = RESULT_NOT_OK;
    uint8_t nStateMachine = TEST_FLASH_STATE_PARSING_START_MARKER;
    uint32_t nCntByte = pSizeDataRecord;
    const uint8_t *pStartRecord;

    while ((0U != nCntByte) && (RESULT_NOT_OK == enResult))
    {
        switch (nStateMachine)
        {
            case TEST_FLASH_STATE_PARSING_START_MARKER :
                if (TEST_FLASH_RECORD_HEADER_MARKER == *pDataRecord)
                {
                    nStateMachine = TEST_FLASH_STATE_PARSING_PAYLOAD;
                    pStartRecord = pDataRecord;
                }
                pDataRecord++;
                break;
            case TEST_FLASH_STATE_PARSING_PAYLOAD :
                if ((TEST_FLASH_RECORD_END_MARKER == *pDataRecord) ||
                    (TEST_FLASH_RECORD_HEADER_MARKER == *pDataRecord))
                {
                    nStateMachine = TEST_FLASH_STATE_PARSING_BYTE_STUFFING;
                }
                pDataRecord++;
                break;
            case TEST_FLASH_STATE_PARSING_BYTE_STUFFING :
                if ((TEST_FLASH_RECORD_END_MARKER == *pDataRecord) ||
                    (TEST_FLASH_RECORD_HEADER_MARKER == *pDataRecord))
                {
                    nStateMachine = TEST_FLASH_STATE_PARSING_PAYLOAD;
                    pDataRecord++;
                }
                else
                {
                    if (*(pDataRecord - 2U) != CH_SUM_CalculateCRC8(pStartRecord,
                                                                    (uint32_t)(pDataRecord - 3U) - (uint32_t)pStartRecord))
                    {
                        enResult = RESULT_NOT_OK;
                    }
                    {
                        enResult = RESULT_OK;
                    }
                }
                break;
            default:break;
        }
        nCntByte--;
    }

    return enResult;
} // end of TEST_FLASH_ParsingRecord()
//****************************************** end of file *******************************************








