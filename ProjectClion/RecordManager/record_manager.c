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
#include "record_manager.h"

// Get W25Q module interface
#include "W25Q_drv.h"

// Get checksum module interface
#include "checksum.h"

#include "printf.h"
#include "stdlib.h"
#include "ftoa.h"

// Get eeprom emulation interface
#include "eeprom.h"

// RTOS
#include "FreeRTOS.h"



//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// Mutex to get resource
xSemaphoreHandle RECORD_MAN_xMutex;



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

// Virtual address
#define RECORD_MAN_VIR_ADR_LSB_LAST_RECORD           (uint16_t)(0x01)
#define RECORD_MAN_VIR_ADR_MSB_LAST_RECORD           (uint16_t)(0x02)
#define RECORD_MAN_VIR_ADR32_LAST_RECORD             ((uint32_t)(RECORD_MAN_VIR_ADR_LSB_LAST_RECORD) | \
                                                     ((uint32_t)(RECORD_MAN_VIR_ADR_MSB_LAST_RECORD) << 16U))

#define RECORD_MAN_VIR_ADR_LSB_NEXT_RECORD           (uint16_t)(0x03)
#define RECORD_MAN_VIR_ADR_MSB_NEXT_RECORD           (uint16_t)(0x04)
#define RECORD_MAN_VIR_ADR32_NEXT_RECORD             ((uint32_t)(RECORD_MAN_VIR_ADR_LSB_NEXT_RECORD) | \
                                                     ((uint32_t)(RECORD_MAN_VIR_ADR_MSB_NEXT_RECORD) << 16U))

#define RECORD_MAN_VIR_ADR_LSB_QTY_RECORD            (uint16_t)(0x05)
#define RECORD_MAN_VIR_ADR_MSB_QTY_RECORD            (uint16_t)(0x06)
#define RECORD_MAN_VIR_ADR32_QTY_RECORD              ((uint32_t)(RECORD_MAN_VIR_ADR_LSB_QTY_RECORD) | \
                                                     ((uint32_t)(RECORD_MAN_VIR_ADR_MSB_QTY_RECORD) << 16U))

// Size record number in bytes
#define RECORD_MAN_SIZE_RECORD_NUM_BYTES        (4U)

// Header record
#define RECORD_MAN_HEADER_MARKER           (0x10U)
// End marker record
#define RECORD_MAN_END_MARKER              (0x03U)

// State machine
#define RECORD_MAN_STATE_PARSING_START_MARKER      (0U)
#define RECORD_MAN_STATE_PARSING_NUM_PACKAGE       (1U)
#define RECORD_MAN_STATE_PARSING_PAYLOAD           (2U)
#define RECORD_MAN_STATE_PARSING_DOUBLE_HEADER_MARKER     (3U)
#define RECORD_MAN_STATE_PARSING_END_MARKER        (4U)



//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// Init flag
uint8_t RECORD_MAN_bInitialezed = FALSE;

// Array max size data record
static uint8_t RECORD_MAN_aRecordDataPackage[RECORD_MAN_MAX_SIZE_RECORD];

//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// Create record package
static STD_RESULT RECORD_MAN_CreateRecord(const uint8_t *pDataSource,
                                          const uint32_t nDataSourceQty,
                                          uint8_t *pDataRecord,
                                          uint32_t *pSizeDataRecord,
                                          uint32_t nRecordNumber);

// Parsing record
static STD_RESULT RECORD_MAN_ParsingRecord(const uint8_t *pDataRecord,
                                           uint32_t nSizeRecord,
                                           uint8_t *pData,
                                           uint32_t *pSizeData);



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
// @Function      RECORD_MAN_Store()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
STD_RESULT RECORD_MAN_Store(const uint8_t *pData,
                            uint32_t nDataQty)
{
    STD_RESULT enResult = RESULT_NOT_OK;
    uint32_t nAdrNextRecord = 0U;
    uint32_t nSizeRecord = 0U;
    uint32_t nRecordNumber = 0U;


    if (TRUE == RECORD_MAN_bInitialezed)
    {
        // Get next record address
        if (RESULT_OK == EE_ReadVariable32(RECORD_MAN_VIR_ADR32_NEXT_RECORD,
                                           &nAdrNextRecord))
        {
            // Get last record number
            EE_ReadVariable32(RECORD_MAN_VIR_ADR32_QTY_RECORD,
                              &nRecordNumber);
            // Create record
            if (RESULT_OK == RECORD_MAN_CreateRecord(pData,
                                                     nDataQty,
                                                     RECORD_MAN_aRecordDataPackage,
                                                     &nSizeRecord,
                                                     nRecordNumber))
            {
                // Write record in flash
                if (RESULT_OK == W25Q_WriteData(nAdrNextRecord,
                                                RECORD_MAN_aRecordDataPackage,
                                                nSizeRecord))
                {
                    // Save new next rescord address
                    nAdrNextRecord += nSizeRecord;

                    // Write new next record adrress in eeprom
                    if (RESULT_OK == EE_WriteVariable32(RECORD_MAN_VIR_ADR32_NEXT_RECORD,
                                        nAdrNextRecord))
                    {
                        enResult = RESULT_OK;
                    }
                    else
                    {
                        enResult = RESULT_NOT_OK;
                    }
                }
                else
                {
                    enResult = RESULT_NOT_OK;
                }
            }
            else
            {
                enResult = RESULT_NOT_OK;
            }
        }
        else
        {
            enResult = RESULT_NOT_OK;
        }
    }
    else
    {
        enResult = RESULT_NOT_OK;
    }

    return enResult;
} // end of RECORD_MAN_Store()



//**************************************************************************************************
// @Function      RECORD_MAN_Load()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
extern STD_RESULT RECORD_MAN_Load(uint32_t nAdrRecord,
                                  uint8_t *pRecord,
                                  uint32_t* nQtyBytes)
{
    STD_RESULT enResult = RESULT_NOT_OK;

    if (TRUE == RECORD_MAN_bInitialezed)
    {
        // Read record
        if (RESULT_OK == W25Q_ReadData(nAdrRecord,
                                       RECORD_MAN_aRecordDataPackage,
                                       (uint32_t)RECORD_MAN_MAX_SIZE_RECORD))
        {
            if (RESULT_OK == RECORD_MAN_ParsingRecord(RECORD_MAN_aRecordDataPackage,
                                     (uint32_t)RECORD_MAN_MAX_SIZE_RECORD,
                                     pRecord,
                                     nQtyBytes))
            {
                enResult = RESULT_OK;
            }
            else
            {
                enResult = RESULT_NOT_OK;
            }
        }
        else
        {
            enResult = RESULT_NOT_OK;
        }
    }
    else
    {
        enResult = RESULT_NOT_OK;
    }

    return enResult;
} // end of RECORD_MAN_Load()



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
static STD_RESULT RECORD_MAN_CreateRecord(const uint8_t *pDataSource,
                                          const uint32_t nDataSourceQty,
                                          uint8_t *pDataRecord,
                                          uint32_t *pSizeDataRecord,
                                          uint32_t nRecordNumber)
{
    STD_RESULT enResult = RESULT_OK;
    uint32_t nSizeRecord = 0U;
    const uint8_t *pItem = (uint8_t*)&nRecordNumber;
    uint8_t nCRC8 = 0U;

    // Clear buf
    for (uint32_t nIndex = 0; nIndex < RECORD_MAN_MAX_SIZE_RECORD; nIndex++)
    {
        pDataRecord[nIndex] = 0U;
    }

    // Calculate crc8 of number package
    nCRC8 = CH_SUM_CalculateCRC8(pItem,
                                 4U);

    // Calculate crc8 of number package and data
    nCRC8 = CH_SUM_CalculateCRC8WithBegin(pDataSource,
                                          nDataSourceQty,
                                          nCRC8);

    // Write header
    pDataRecord[nSizeRecord] = (uint8_t)RECORD_MAN_HEADER_MARKER;
    nSizeRecord++;

    for (int i = 0; i < nDataSourceQty + RECORD_MAN_SIZE_RECORD_NUM_BYTES + RECORD_MAN_SIZE_CRC8; i++)
    {
        if (RECORD_MAN_SIZE_RECORD_NUM_BYTES == i)
        {
            pItem = pDataSource;
        }
        else if ((nDataSourceQty + RECORD_MAN_SIZE_RECORD_NUM_BYTES) == i)
        {
            pItem = &nCRC8;
        }

        if (RECORD_MAN_MAX_SIZE_RECORD > nSizeRecord)
        {
            pDataRecord[nSizeRecord] = *pItem;
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
            if (RECORD_MAN_HEADER_MARKER == *pItem)
            {
                pDataRecord[nSizeRecord] = (uint8_t)RECORD_MAN_HEADER_MARKER;
                nSizeRecord++;
            }
            else if  (RECORD_MAN_END_MARKER == *pItem)
            {
                pDataRecord[nSizeRecord] = (uint8_t)RECORD_MAN_END_MARKER;
                nSizeRecord++;
            }
        }
        else
        {
            enResult = RESULT_NOT_OK;
            break;
        }
        pItem++;
    }

    if ((RESULT_OK == enResult) && (RECORD_MAN_MAX_SIZE_RECORD > (nSizeRecord + 1U)))
    {
        pDataRecord[nSizeRecord] = RECORD_MAN_END_MARKER;
    }
    else
    {
        enResult = RESULT_NOT_OK;
    }

    *pSizeDataRecord = nSizeRecord + 1U;

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
static STD_RESULT RECORD_MAN_ParsingRecord(const uint8_t *pDataRecord,
                                           uint32_t nSizeRecord,
                                           uint8_t *pData,
                                           uint32_t *pSizeData)
{
    STD_RESULT enResult = RESULT_NOT_OK;
    uint8_t nStateMachine = RECORD_MAN_STATE_PARSING_START_MARKER;
    uint32_t nSizeData = 0U;
    uint32_t nIndexDataRecord = 0U;

    while ((0U != nSizeRecord) && (RESULT_NOT_OK == enResult))
    {
        switch (nStateMachine)
        {
            case RECORD_MAN_STATE_PARSING_START_MARKER :
                if (RECORD_MAN_HEADER_MARKER == pDataRecord[nIndexDataRecord])
                {
                    nStateMachine = RECORD_MAN_STATE_PARSING_PAYLOAD;
                }
                nIndexDataRecord++;
                break;
            case RECORD_MAN_STATE_PARSING_PAYLOAD :
                if (RECORD_MAN_HEADER_MARKER == pDataRecord[nIndexDataRecord])
                {
                    nStateMachine = RECORD_MAN_STATE_PARSING_DOUBLE_HEADER_MARKER;
                }
                else if (RECORD_MAN_END_MARKER == pDataRecord[nIndexDataRecord])
                {
                    if (0U == (nSizeRecord - 1U))
                    {
                        if (pDataRecord[nIndexDataRecord - 1U] != CH_SUM_CalculateCRC8(pData,
                                                                                       nSizeData))
                        {
                            enResult = RESULT_NOT_OK;
                        }
                        else
                        {
                            enResult = RESULT_OK;
                            *pSizeData = nSizeData - 1U;
                        }
                    }
                    else
                    {
                        nStateMachine = RECORD_MAN_STATE_PARSING_END_MARKER;
                    }
                }
                else
                {
                    pData[nSizeData] = pDataRecord[nIndexDataRecord];
                    nSizeData++;
                }
                nIndexDataRecord++;
                break;
            case RECORD_MAN_STATE_PARSING_DOUBLE_HEADER_MARKER :
                if (RECORD_MAN_HEADER_MARKER == pDataRecord[nIndexDataRecord])
                {
                    nStateMachine = RECORD_MAN_STATE_PARSING_PAYLOAD;
                    pData[nSizeData] = pDataRecord[nIndexDataRecord];
                    nSizeData++;
                    nIndexDataRecord++;
                }
                else
                {
                    nStateMachine = RECORD_MAN_STATE_PARSING_PAYLOAD;
                    nSizeData = 0U;
                    nIndexDataRecord++;
                }
                break;
            case RECORD_MAN_STATE_PARSING_END_MARKER :
                if (RECORD_MAN_END_MARKER == pDataRecord[nIndexDataRecord])
                {
                    nStateMachine = RECORD_MAN_STATE_PARSING_PAYLOAD;
                    pData[nSizeData] = pDataRecord[nIndexDataRecord];
                    nSizeData++;
                    nIndexDataRecord++;
                }
                else
                {
                    if (pDataRecord[nIndexDataRecord - 2U] != CH_SUM_CalculateCRC8(pData,
                                                                                   nSizeData - 1U))
                    {
                        enResult = RESULT_NOT_OK;
                    }
                    else
                    {
                        enResult = RESULT_OK;
                        *pSizeData = nSizeData;
                    }
                }
                break;
            default:break;
        }
        nSizeRecord--;
    }

    return enResult;
} // end of RECORD_MAN_ParsingRecord()



//****************************************** end of file *******************************************