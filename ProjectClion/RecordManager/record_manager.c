//**************************************************************************************************
// @Module        RECORD_MAN
// @Filename      record_manager.c
//--------------------------------------------------------------------------------------------------
// @Platform      STM32
//--------------------------------------------------------------------------------------------------
// @Compatible    STM32L4xx
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the RECORD_MAN functionality.
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
#inlclude "record_manager.h"

// Get W25Q module interface
#include "W25Q_drv.h"

// Get checksum module interface
#include "checksum.h"

#include "printf.h"
#include "stdlib.h"
#include "ftoa.h"

// Get eeprom emulation interface
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
#define TEST_FLASH_MAX_SIZE_RECORD                 (100U)
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

// Init flag
U8 RECORD_MAN_bInitialezed = FALSE;





static uint8_t dataRead[SIZE_BUF];
static uint8_t dataWrite[SIZE_BUF];

TEST_EE_METEO_DATA TEST_EE_MeteoData;
uint8_t aRecordDataPackage[TEST_FLASH_MAX_SIZE_RECORD];
uint8_t aRecordReadBack[TEST_FLASH_MAX_SIZE_RECORD];
uint32_t nSizeDataRecord;


//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// Create record package
static STD_RESULT RECORD_MAN_CreateRecord(const TEST_EE_METEO_DATA *pMeteoData,
                                          uint8_t *pDataRecord,
                                          uint32_t *pSizeDataRecord);

// Parsing record
static STD_RESULT RECORD_MAN_ParsingRecord(TEST_EE_METEO_DATA *pMeteoData,
                                           const uint8_t *pDataRecord,
                                           uint32_t pSizeDataRecord);



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      RECORD_MAN_Init()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void RECORD_MAN_Init(void)
{
    if (FALSE == RECORD_MAN_bInitialezed)
    {
        // Init W25Q
        W25Q_Init();

        // Init eeprom emulation
        EE_Init();
        


        RECORD_MAN_bInitialezed = TRUE;
    }
    else
    {
        DoNothing();
    }
} // end of RECORD_MAN_Init()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      RECORD_MAN_CreateRecord()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT RECORD_MAN_CreateRecord(const TEST_EE_METEO_DATA *pMeteoData,
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
        if (TEST_FLASH_MAX_SIZE_RECORD > nSizeRecord)
        {
            pDataRecord[nSizeRecord] = *pMeteoDataByte;
            nSizeRecord++;

        }
        else
        {
            enResult = RESULT_NOT_OK;
            break;
        }

        if (TEST_FLASH_MAX_SIZE_RECORD > nSizeRecord)
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

    if ((RESULT_OK == enResult) && (TEST_FLASH_MAX_SIZE_RECORD > (nSizeRecord + 2)))
    {
        pDataRecord[nSizeRecord] = CH_SUM_CalculateCRC8(pDataRecord,
                                                        nSizeRecord-1);

        nSizeRecord++;
        pDataRecord[nSizeRecord] = TEST_FLASH_RECORD_END_MARKER;
    }

    *pSizeDataRecord = nSizeRecord + 1;

    return enResult;
} // end of RECORD_MAN_CreateRecord()



//**************************************************************************************************
// @Function      RECORD_MAN_ParsingRecord()
//--------------------------------------------------------------------------------------------------
// @Description   This function finds package and calculates crc8.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT RECORD_MAN_ParsingRecord(TEST_EE_METEO_DATA *pMeteoData,
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
} // end of RECORD_MAN_ParsingRecord()



//****************************************** end of file *******************************************








