//**************************************************************************************************
// @Module        EEPROM Emulation
// @Filename      eeprom_emulation.c
//--------------------------------------------------------------------------------------------------
// @Platform      Independent
//--------------------------------------------------------------------------------------------------
// @Compatible    Independent
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the EEPROM Emulation functionality.
//
//                Abbreviations:
//                  EEPROM - Electrically Erasable Programmable Read-Only Memory
//                  EEP    - EEPROM
//
//                Global (public) functions:
//                  EMEEP_Init()
//                  EMEEP_DeInit()
//                  EMEEP_SetJobCallback()
//                  EMEEP_Load()
//                  EMEEP_Store()
//                  EMEEP_GetJobResult()
//                  EMEEP_GetMemoryStatus()
//                  EMEEP_SetMemoryStatus()
//
//                Local (private) functions:
//                  EMEEP_FindLastValidRecord()
//                  EMEEP_FindPreviousValidRecord()
//                  EMEEP_GetNextRecordAddr()
//                  EMEEP_GetPrevRecordAddr()
//                  EMEEP_IsSignatureValid()
//                  EMEEP_IsChecksumValid()
//                  EMEEP_CalcChecksum()
//                  EMEEP_FlashCallback()
//
//--------------------------------------------------------------------------------------------------
// @Version      
//--------------------------------------------------------------------------------------------------
// @Date          
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment

//**************************************************************************************************



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Native header
#include "eeprom_emulation.h"

// Get error tracer functions
#include "SEGGER_RTT.h"

// Get flash memory functions
#include "W25Q_drv.h"



//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************
#define CPU_WORD_SIZE               (8U)
// Type of each of the record fields
#if (CPU_WORD_SIZE == 32U)
#define EMEEP_CHECKSUM_FIELD_TYPE           uint32_t
#define EMEEP_SIGNATURE_FIELD_TYPE          uint32_t
#define EMEEP_NUMBER_FIELD_TYPE             uint32_t
#elif (CPU_WORD_SIZE == 16U)
#define EMEEP_CHECKSUM_FIELD_TYPE           uint16_t
#define EMEEP_SIGNATURE_FIELD_TYPE          uint32_t
#define EMEEP_NUMBER_FIELD_TYPE             uint32_t
#elif (CPU_WORD_SIZE == 8U)
#define EMEEP_CHECKSUM_FIELD_TYPE           uint8_t
#define EMEEP_SIGNATURE_FIELD_TYPE          uint32_t
#define EMEEP_NUMBER_FIELD_TYPE             uint32_t
#else
#error EMEEP: other platforms than 8, 16 and 32-bit are not supported
#endif // (CPU_WORD_SIZE == xxU)

// Size of each of the record fields
#define EMEEP_CHECKSUM_FIELD_SIZE           (sizeof(EMEEP_CHECKSUM_FIELD_TYPE))
#define EMEEP_SIGNATURE_FIELD_SIZE          (sizeof(EMEEP_SIGNATURE_FIELD_TYPE))
#define EMEEP_NUMBER_FIELD_SIZE             (sizeof(EMEEP_NUMBER_FIELD_TYPE))

// Size of each of the record fields
// !!! "..._FIELD_SIZE" constants has been intentionally duplicated
// because "sizeof()" operator not works in preprocessor
#if (CPU_WORD_SIZE == 32U)
#define EMEEP_CHECKSUM_FIELD_SIZE_PREPROC   (4U)
#define EMEEP_SIGNATURE_FIELD_SIZE_PREPROC  (4U)
#define EMEEP_NUMBER_FIELD_SIZE_PREPROC     (4U)
#elif (CPU_WORD_SIZE == 16U)
#define EMEEP_CHECKSUM_FIELD_SIZE_PREPROC   (2U)
#define EMEEP_SIGNATURE_FIELD_SIZE_PREPROC  (4U)
#define EMEEP_NUMBER_FIELD_SIZE_PREPROC     (4U)
#elif (CPU_WORD_SIZE == 8U)
#define EMEEP_CHECKSUM_FIELD_SIZE_PREPROC   (1U)
#define EMEEP_SIGNATURE_FIELD_SIZE_PREPROC  (4U)
#define EMEEP_NUMBER_FIELD_SIZE_PREPROC     (4U)
#endif // (CPU_WORD_SIZE == xxU)

// Size of record header
#define EMEEP_HEADER_SIZE                   (EMEEP_CHECKSUM_FIELD_SIZE_PREPROC  + \
                                             EMEEP_SIGNATURE_FIELD_SIZE_PREPROC + \
                                             EMEEP_NUMBER_FIELD_SIZE_PREPROC)

// Check EMEEP_INTERNAL_DIAGNOSTICS configuration parameter value
#if ((EMEEP_INTERNAL_DIAGNOSTICS != OFF) && (EMEEP_INTERNAL_DIAGNOSTICS != ON))
#error EMEEP configuration: EMEEP_INTERNAL_DIAGNOSTICS value must be "ON" or "OFF".
#endif // #if ((EMEEP_INTERNAL_DIAGNOSTICS != OFF) && (EMEEP_INTERNAL_DIAGNOSTICS != ON))

// Check EMEEP_USED_BANKS_QTY configuration parameter value
#if ((EMEEP_USED_BANKS_QTY < 1) || (EMEEP_USED_BANKS_QTY > 4))
#error EMEEP configuration: EMEEP_USED_BANKS_QTY value must be [1 ; 4].
#endif // #if ((EMEEP_USED_BANKS_QTY < 1) || (EMEEP_USED_BANKS_QTY > 4))

// Check EMEEP_BANK_0_END_ADDRESS and EMEEP_BANK_0_START_ADDRESS configuration parameters values
#if (EMEEP_BANK_0_END_ADDRESS <= EMEEP_BANK_0_START_ADDRESS)
#error EMEEP configuration: EMEEP_BANK_0_END_ADDRESS value must be > EMEEP_BANK_0_START_ADDRESS.
#endif // #if (EMEEP_BANK_0_END_ADDRESS <= EMEEP_BANK_0_START_ADDRESS)

// Check EMEEP_BANK_0_RECORD_DATA_ARRAY_SIZE configuration parameter value
#if (((EMEEP_BANK_0_END_ADDRESS - EMEEP_BANK_0_START_ADDRESS + 1U) % \
     (EMEEP_BANK_0_RECORD_DATA_ARRAY_SIZE + EMEEP_HEADER_SIZE)) != 0U)
#error EMEEP configuration: EMEEP_BANK_0_RECORD_DATA_ARRAY_SIZE value are invalid.      \
(EMEEP_BANK_0_RECORD_DATA_ARRAY_SIZE + EMEEP_HEADER_SIZE (10 bytes)) must be a multiple \
of the specified memory area size.
#endif // #if (((EMEEP_BANK_0_END_ADDRESS - EMEEP_BANK_0_START_ADDRESS + 1U) % ...

// Check EMEEP_BANK_0_SECTOR_EW_ENDURANCE configuration parameter value
#if (EMEEP_BANK_0_SECTOR_EW_ENDURANCE == 0U)
#error EMEEP configuration: EMEEP_BANK_0_SECTOR_EW_ENDURANCE value must be > 0.
#endif // #if (EMEEP_BANK_0_SECTOR_EW_ENDURANCE > 0)

#if (EMEEP_USED_BANKS_QTY >= 2)

// Check EMEEP_BANK_1_END_ADDRESS and EMEEP_BANK_1_START_ADDRESS configuration parameters values
#if (EMEEP_BANK_1_END_ADDRESS <= EMEEP_BANK_1_START_ADDRESS)
#error EMEEP configuration: EMEEP_BANK_1_END_ADDRESS value must be > EMEEP_BANK_1_START_ADDRESS.
#endif // #if (EMEEP_BANK_1_END_ADDRESS <= EMEEP_BANK_1_START_ADDRESS)

// Check EMEEP_BANK_1_RECORD_DATA_ARRAY_SIZE configuration parameter value
#if (((EMEEP_BANK_1_END_ADDRESS - EMEEP_BANK_1_START_ADDRESS + 1U) % \
     (EMEEP_BANK_1_RECORD_DATA_ARRAY_SIZE + EMEEP_HEADER_SIZE)) != 0U)
#error EMEEP configuration: EMEEP_BANK_1_RECORD_DATA_ARRAY_SIZE value are invalid.      \
(EMEEP_BANK_1_RECORD_DATA_ARRAY_SIZE + EMEEP_HEADER_SIZE (10 bytes)) must be a multiple \
of the specified memory area size.
#endif // #if (((EMEEP_BANK_1_END_ADDRESS - EMEEP_BANK_1_START_ADDRESS + 1U) % ...

// Check EMEEP_BANK_1_SECTOR_EW_ENDURANCE configuration parameter value
#if (EMEEP_BANK_1_SECTOR_EW_ENDURANCE == 0U)
#error EMEEP configuration: EMEEP_BANK_1_SECTOR_EW_ENDURANCE value must be > 0.
#endif // #if (EMEEP_BANK_1_SECTOR_EW_ENDURANCE > 0)

#if (EMEEP_USED_BANKS_QTY >= 3)

// Check EMEEP_BANK_2_END_ADDRESS and EMEEP_BANK_2_START_ADDRESS configuration parameters values
#if (EMEEP_BANK_2_END_ADDRESS <= EMEEP_BANK_2_START_ADDRESS)
#error EMEEP configuration: EMEEP_BANK_2_END_ADDRESS value must be > EMEEP_BANK_2_START_ADDRESS.
#endif // #if (EMEEP_BANK_2_END_ADDRESS <= EMEEP_BANK_2_START_ADDRESS)

// Check EMEEP_BANK_2_RECORD_DATA_ARRAY_SIZE configuration parameter value
#if (((EMEEP_BANK_2_END_ADDRESS - EMEEP_BANK_2_START_ADDRESS + 1U) % \
     (EMEEP_BANK_2_RECORD_DATA_ARRAY_SIZE + EMEEP_HEADER_SIZE)) != 0U)
#error EMEEP configuration: EMEEP_BANK_2_RECORD_DATA_ARRAY_SIZE value are invalid.      \
(EMEEP_BANK_2_RECORD_DATA_ARRAY_SIZE + EMEEP_HEADER_SIZE (10 bytes)) must be a multiple \
of the specified memory area size.
#endif // #if (((EMEEP_BANK_2_END_ADDRESS - EMEEP_BANK_2_START_ADDRESS + 1U) % ...

// Check EMEEP_BANK_2_SECTOR_EW_ENDURANCE configuration parameter value
#if (EMEEP_BANK_2_SECTOR_EW_ENDURANCE == 0U)
#error EMEEP configuration: EMEEP_BANK_2_SECTOR_EW_ENDURANCE value must be > 0.
#endif // #if (EMEEP_BANK_2_SECTOR_EW_ENDURANCE > 0)

#if (EMEEP_USED_BANKS_QTY >= 4)

// Check EMEEP_BANK_3_END_ADDRESS and EMEEP_BANK_3_START_ADDRESS configuration parameters values
#if (EMEEP_BANK_3_END_ADDRESS <= EMEEP_BANK_3_START_ADDRESS)
#error EMEEP configuration: EMEEP_BANK_3_END_ADDRESS value must be > EMEEP_BANK_3_START_ADDRESS.
#endif // #if (EMEEP_BANK_3_END_ADDRESS <= EMEEP_BANK_3_START_ADDRESS)

// Check EMEEP_BANK_3_RECORD_DATA_ARRAY_SIZE configuration parameter value
#if (((EMEEP_BANK_3_END_ADDRESS - EMEEP_BANK_3_START_ADDRESS + 1U) % \
     (EMEEP_BANK_3_RECORD_DATA_ARRAY_SIZE + EMEEP_HEADER_SIZE)) != 0U)
#error EMEEP configuration: EMEEP_BANK_3_RECORD_DATA_ARRAY_SIZE value are invalid.      \
(EMEEP_BANK_3_RECORD_DATA_ARRAY_SIZE + EMEEP_HEADER_SIZE (10 bytes)) must be a multiple \
of the specified memory area size.
#endif // #if (((EMEEP_BANK_3_END_ADDRESS - EMEEP_BANK_3_START_ADDRESS + 1U) % ...

// Check EMEEP_BANK_3_SECTOR_EW_ENDURANCE configuration parameter value
#if (EMEEP_BANK_3_SECTOR_EW_ENDURANCE == 0U)
#error EMEEP configuration: EMEEP_BANK_3_SECTOR_EW_ENDURANCE value must be > 0.
#endif // #if (EMEEP_BANK_3_SECTOR_EW_ENDURANCE > 0)

#endif // #if (EMEEP_USED_BANKS_QTY >= 4)
#endif // #if (EMEEP_USED_BANKS_QTY >= 3)
#endif // #if (EMEEP_USED_BANKS_QTY >= 2)

// Check EMEEP_WEAR_LEVEL_THRS_1 configuration parameter value
#if ((EMEEP_WEAR_LEVEL_THRS_1 == 0U) || (EMEEP_WEAR_LEVEL_THRS_1 > 99U))
#error EMEEP configuration: EMEEP_WEAR_LEVEL_THRS_1 value must be [1 ; 99].
#endif // #if (EMEEP_WEAR_LEVEL_THRS_1 > 0)



//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of local (private) data types
//**************************************************************************************************

// Program module API IDs
typedef enum EMEEP_API_ID_enum
{
    EMEEP_API_ID_INNER_FUNC    = (uint8_t)0U,
    EMEEP_API_ID_INIT          = (uint8_t)1U,
    EMEEP_API_ID_DEINIT        = (uint8_t)2U,
    EMEEP_API_ID_SETJOBCB      = (uint8_t)3U,
    EMEEP_API_ID_LOAD          = (uint8_t)4U,
    EMEEP_API_ID_STORE         = (uint8_t)5U,
    EMEEP_API_ID_GETJOBRESULT  = (uint8_t)6U,
    EMEEP_API_ID_GETMEMSTAT    = (uint8_t)7U,
    EMEEP_API_ID_SETMEMSTAT    = (uint8_t)8U

} EMEEP_API_ID;

#if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
// Diagnostics counters structure
typedef volatile struct EMEEP_DIAG_COUNTERS_str
{
    uint32_t nStoreCallCnt;
    uint32_t nLoadCallCnt;

    uint32_t nFlashReadAttemptCnt;
    uint32_t nFlashReadSuccessCnt;

    uint32_t nFlashEraseAttemptCnt;
    uint32_t nFlashEraseSuccessCnt;

    uint32_t nFlashWriteAttemptCnt;
    uint32_t nFlashWriteSuccessCnt;

    uint32_t nBytesReadSuccessful;
    uint32_t nBytesWrittenSuccessful;
    uint32_t nBytesErasedSuccessful;
    
    EMEEP_NUMBER_FIELD_TYPE nLastValidRecordNumber;

    uint32_t nBytesToRead;

    uint8_t nMemoryWearingLevel;

} EMEEP_DIAG_COUNTERS;
#endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

// Memory bank static configuration container type
typedef struct EMEEP_BANK_STATIC_CFG_str
{
    uint32_t nStartAddress;
    uint32_t nEndAddress;
    uint16_t nRecordDataSize;
    uint32_t nRecordSize;
    uint32_t nMemoryCapacity;
    uint32_t nMemoryEndurance;

} EMEEP_BANK_STATIC_CFG;

// Memory bank parameters container type
typedef struct EMEEP_BANK_PARAMS_str
{
    // Last record relevance flag
    BOOLEAN bValidRecordFound;
    // Reserved field to align structure
    uint8_t nReserved[3U];
    // Address of the last valid record
    uint32_t nLastValidRecordAddress;
    // New record address to be written
    uint32_t nNextRecordAddress;

} EMEEP_BANK_PARAMS;

// Buffer size for the data array of the bank 0
#define EMEEP_BANK_0_BUFFER_SIZE    (EMEEP_BANK_0_RECORD_DATA_ARRAY_SIZE)

// Buffer size for the data array of the bank 1
#if (EMEEP_USED_BANKS_QTY >= 2)
#define EMEEP_BANK_1_BUFFER_SIZE    (EMEEP_BANK_1_RECORD_DATA_ARRAY_SIZE)
#else
#define EMEEP_BANK_1_BUFFER_SIZE    (0U)
#endif // #if (EMEEP_USED_BANKS_QTY >= 2)

// Buffer size for the data array of the bank 2
#if (EMEEP_USED_BANKS_QTY >= 3)
#define EMEEP_BANK_2_BUFFER_SIZE    (EMEEP_BANK_2_RECORD_DATA_ARRAY_SIZE)
#else
#define EMEEP_BANK_2_BUFFER_SIZE    (0U)
#endif // #if (EMEEP_USED_BANKS_QTY >= 3)

// Buffer size for the data array of the bank 3
#if (EMEEP_USED_BANKS_QTY >= 4)
#define EMEEP_BANK_3_BUFFER_SIZE    (EMEEP_BANK_3_RECORD_DATA_ARRAY_SIZE)
#else
#define EMEEP_BANK_3_BUFFER_SIZE    (0U)
#endif // #if (EMEEP_USED_BANKS_QTY >= 4)

// The largest size of all of the data arrays from all of the banks
#define EMEEP_MAX_DATA_BUFFER_SIZE  (MAX(MAX(MAX(EMEEP_BANK_0_BUFFER_SIZE, EMEEP_BANK_1_BUFFER_SIZE), EMEEP_BANK_2_BUFFER_SIZE), EMEEP_BANK_3_BUFFER_SIZE))

// Definition of the one data entry type
// (a one memory record structure)
typedef struct EMEEP_RECORD_str
{
    EMEEP_CHECKSUM_FIELD_TYPE  nChecksum;
    EMEEP_SIGNATURE_FIELD_TYPE nSignature;
    EMEEP_NUMBER_FIELD_TYPE    nNumber;
    uint8_t  dataArray[EMEEP_MAX_DATA_BUFFER_SIZE];

} EMEEP_RECORD;



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

// Program module ID ( = program module prefix)
static const uint8_t EMEEP_moduleID[] = "EMEEP";

// Record signature value
#define EMEEP_RECORD_SIGNATURE_VALUE    (0xDF39F1ABUL)

// Flash driver job events
#define EMEEP_READ_EVENT_ID             (0U)
#define EMEEP_ERASE_EVENT_ID            (1U)
#define EMEEP_WRITE_EVENT_ID            (2U)

// Memory jobs
#define EMEEP_JOB_IDLE                  (0U)
#define EMEEP_JOB_LOAD                  (1U)
#define EMEEP_JOB_STORE                 (2U)

// Maximum percents in 1
#define EMEEP_MAX_PERCENTS              (100UL)

// Record size calculation macro
#define EMEEP_GET_RECORD_SIZE(nBankNumber)       (EMEEP_BANK_##nBankNumber##_RECORD_DATA_ARRAY_SIZE  +  \
                                                  EMEEP_HEADER_SIZE)

// Bank configuration macro
#define EMEEP_BANK_CONFIGURE(nBankNumber)       {EMEEP_BANK_##nBankNumber##_START_ADDRESS,              \
                                                 EMEEP_BANK_##nBankNumber##_END_ADDRESS,                \
                                                 EMEEP_BANK_##nBankNumber##_RECORD_DATA_ARRAY_SIZE,     \
                                                 EMEEP_GET_RECORD_SIZE(nBankNumber),                    \
                                                 ((EMEEP_BANK_##nBankNumber##_END_ADDRESS -             \
                                                   EMEEP_BANK_##nBankNumber##_START_ADDRESS + 1U) /     \
                                                  EMEEP_GET_RECORD_SIZE(nBankNumber)),                  \
                                                 EMEEP_BANK_##nBankNumber##_SECTOR_EW_ENDURANCE}

// Memory bank static configuration container
static const EMEEP_BANK_STATIC_CFG EMEEP_banksStaticCfg[EMEEP_USED_BANKS_QTY] = 
{
    EMEEP_BANK_CONFIGURE(0)
    #if (EMEEP_USED_BANKS_QTY >= 2)
    , EMEEP_BANK_CONFIGURE(1)
    #if (EMEEP_USED_BANKS_QTY >= 3)
    , EMEEP_BANK_CONFIGURE(2)
    #if (EMEEP_USED_BANKS_QTY >= 4)
    , EMEEP_BANK_CONFIGURE(3)
    #endif // #if (EMEEP_USED_BANKS_QTY >= 4)
    #endif // #if (EMEEP_USED_BANKS_QTY >= 3)
    #endif // #if (EMEEP_USED_BANKS_QTY >= 2)
};


//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// Program module initialization state
static BOOLEAN EMEEP_bInitialized;

#if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
// Diagnostic counters
static EMEEP_DIAG_COUNTERS EMEEP_diagCounters[EMEEP_USED_BANKS_QTY];
#endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

// End of load job callback function pointer
static MEM_END_OF_JOB_CALLBACK EMEEP_pLoadCallback;
// End of load job callback event identifier to notify user to
static uint8_t EMEEP_nLoadCallbackEventID;

// End of store job callback function pointer
static MEM_END_OF_JOB_CALLBACK EMEEP_pStoreCallback;
// End of store job callback event identifier to notify user to
static uint8_t EMEEP_nStoreCallbackEventID;

// New record content to be written
// Temporary buffer for new records
static EMEEP_RECORD EMEEP_newRecordsContent[EMEEP_USED_BANKS_QTY];

// Current memory job
static uint8_t EMEEP_nCurrentJob;
// The last (current) job result
static uint8_t EMEEP_nJobResult;

// Memory status mask
static uint32_t EMEEP_nMemoryStatusMask;

// Current memory bank number
static uint8_t EMEEP_nCurrentBankNumber;

// Memory bank parameters container type
static EMEEP_BANK_PARAMS EMEEP_banksParams[EMEEP_USED_BANKS_QTY];


//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// Finds the last valid record in the memory
static void EMEEP_FindLastValidRecord(const uint8_t nBankNumber);

// Finds previous valid record in the memory,
// starting downwards from the specified record.
static uint32_t EMEEP_FindPreviousValidRecord(const uint8_t  nBankNumber,
                                         const uint32_t nFindFrom,
                                         BOOLEAN* const pRecordFound);

// Increments record address taking into account EMEEP memory margins
static uint32_t EMEEP_GetNextRecordAddr(const uint8_t  nBankNumber,
                                   const uint32_t nRecordAddress);

// Decrements record address taking into account EMEEP memory margins
static uint32_t EMEEP_GetPrevRecordAddr(const uint8_t  nBankNumber,
                                   const uint32_t nRecordAddress);

// Checks whether signature of the specified record is valid or not
static BOOLEAN EMEEP_IsSignatureValid(const uint32_t nRecordAddress);

// Checks whether checksum of the specified record is valid or not
static BOOLEAN EMEEP_IsChecksumValid(const uint8_t  nBankNumber,
                                     const uint32_t nRecordAddress);

// Calculates 32-bit arithmetic checksum of the specified data array.
static EMEEP_CHECKSUM_FIELD_TYPE EMEEP_CalcChecksum(const uint8_t* const pDataBuffer,
                                                    const uint32_t nDataQty);

// Returns "checksum" field address from the specified record address
static uint32_t EMEEP_GetChecksumFieldAddress(const uint32_t nRecordAddress);

// Returns "checksum" field value from the specified record address
static EMEEP_CHECKSUM_FIELD_TYPE EMEEP_GetChecksumFieldValue(const uint32_t nRecordAddress);

// Calculates "data" field address from the specified record address
static uint32_t EMEEP_GetDataFieldAddress(const uint32_t nRecordAddress);

// Returns "number" field address from the specified record address
static uint32_t EMEEP_GetNumberFieldAddress(const uint32_t nRecordAddress);

// Calculates "number" field value from the specified record address
static EMEEP_NUMBER_FIELD_TYPE EMEEP_GetNumberFieldValue(const uint32_t nRecordAddress);

// Calls on every end-of-job flash driver event
static void EMEEP_FlashCallback(const uint8_t nEventID,
                                const uint8_t nJobResult);



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      EMEEP_Init()
//--------------------------------------------------------------------------------------------------
// @Description   Initializes SW and HW resources of the program module.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK     - function succeeded
//                RESULT_NOT_OK - function NOT succeeded
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void EMEEP_Init(void)
{
    // Checking whether initialization is done or not
    if (FALSE == EMEEP_bInitialized)
    {
        uint8_t nBankNumber = 0U;

        // Reset job
        EMEEP_nJobResult = MEM_JOB_RESULT_OK;
        EMEEP_nCurrentJob = EMEEP_JOB_IDLE;

        for (nBankNumber = 0U; nBankNumber < EMEEP_USED_BANKS_QTY; nBankNumber ++)
        {
            // Find the last valid record
            EMEEP_nCurrentBankNumber = nBankNumber;
            EMEEP_FindLastValidRecord(nBankNumber);
        }

        EMEEP_bInitialized = TRUE;
    }
    else
    {
        // Program module has been already initialized
        SEGGER_RTT_printf(0, "Eeprom has been already initialized"); 
    }

} // end of EMEEP_Init()



//**************************************************************************************************
// @Function      EMEEP_DeInit()
//--------------------------------------------------------------------------------------------------
// @Description   Deinitializes SW and HW resources of the program module.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK     - function succeeded
//                RESULT_NOT_OK - function NOT succeeded
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void EMEEP_DeInit(void)
{
    // Checking whether initialization is done or not
    if (TRUE == EMEEP_bInitialized)
    {
        EMEEP_bInitialized = FALSE;
    }
    else
    {
        // Program module is not initialized
        SEGGER_RTT_printf(0, "Eeprom is not initialized"); 
    }

} // end of EMEEP_DeInit()



//**************************************************************************************************
// @Function      EMEEP_SetJobCallback()
//--------------------------------------------------------------------------------------------------
// @Description   Sets "end of job" callback function pointer(s) for the specified job type
//                (Load / Store)
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK     - function succeeded
//                RESULT_NOT_OK - function NOT succeeded
//--------------------------------------------------------------------------------------------------
// @Parameters    nJobType  - job type: Load / Store
//                nEventID  - callback event identifier to notify user to
//                pCallback - pointer to an end of read callback function
//**************************************************************************************************
STD_RESULT EMEEP_SetJobCallback(const uint8_t nJobType,
                                const uint8_t nEventID,
                                const MEM_END_OF_JOB_CALLBACK pCallback)
{
    STD_RESULT nFuncResult = RESULT_OK;

    // Checking whether initialization is done or not
    if (TRUE == EMEEP_bInitialized)
    {
        if (EMEEP_JOB_IDLE == EMEEP_nCurrentJob)
        {
            if (NULL_PTR == pCallback)
            {
                // Formally test input pointer for null
                // pCallback CAN be null
                DoNothing();
            }

            switch (nJobType)
            {
                case MEM_JOB_CALLBACK_LOAD:
                    EMEEP_pLoadCallback        = pCallback;
                    EMEEP_nLoadCallbackEventID = nEventID;
                    break;

                case MEM_JOB_CALLBACK_STORE:
                    EMEEP_pStoreCallback        = pCallback;
                    EMEEP_nStoreCallbackEventID = nEventID;
                    break;

                default:
                    // There is no other job types
                    DoNothing();
                    nFuncResult = RESULT_NOT_OK;
                    break;
            }
        }
        else
        {
            // EMEEP program module is busy
            SEGGER_RTT_printf(0, "EMEEP program module is busy"); 
            
            nFuncResult = RESULT_NOT_OK;
        }
    }
    else
    {
        // Program module is not initialized
       SEGGER_RTT_printf(0, "Eeprom is not initialized"); 

        nFuncResult = RESULT_NOT_OK;
    }

    return nFuncResult;

} // end of EMEEP_SetJobCallback()



//**************************************************************************************************
// @Function      EMEEP_Load()
//--------------------------------------------------------------------------------------------------
// @Description   Loads the last valid data image from the emulated EEPROM memory
//                to the specified buffer.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK     - function succeeded
//                RESULT_NOT_OK - function NOT succeeded
//--------------------------------------------------------------------------------------------------
// @Parameters    nSourceAddress - start address of the emulated EEPROM data to be read
//                pDataBuffer    - pointer to the data buffer
//                nDataQty       - size of the emulated EEPROM data image to be read
//**************************************************************************************************
STD_RESULT EMEEP_Load(const uint32_t nSourceAddress,
                      uint8_t* const pDataBuffer,
                      const uint32_t nDataQty)
{
    STD_RESULT nFuncResult = RESULT_OK;

    // Checking whether initialization is done or not
    if (TRUE == EMEEP_bInitialized)
    {
        if (EMEEP_JOB_IDLE == EMEEP_nCurrentJob)
        {
            // Check bank number
            uint8_t nBankNumber = LOBYTE(HIWORD(nSourceAddress));

            if (nBankNumber < EMEEP_USED_BANKS_QTY)
            {
                uint16_t nDataOffset = LOWORD(nSourceAddress);
                EMEEP_nCurrentBankNumber = nBankNumber;

                // Check data quantity parameter
                if (nDataQty <= EMEEP_banksStaticCfg[nBankNumber].nRecordDataSize)
                {
                    // Check source address
                    if ((uint32_t)(nDataOffset + nDataQty) <= EMEEP_banksStaticCfg[nBankNumber].nRecordDataSize)
                    {
                        // Check data buffer pointer
                        if (NULL_PTR != pDataBuffer)
                        {
                            #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                            EMEEP_diagCounters[nBankNumber].nLoadCallCnt ++;
                            EMEEP_diagCounters[nBankNumber].nBytesToRead = nDataQty;
                            #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

                            EMEEP_nJobResult = MEM_JOB_RESULT_PENDING;

                            #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                            EMEEP_diagCounters[nBankNumber].nFlashReadAttemptCnt ++;
                            #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

                            // Set a new current job
                            EMEEP_nCurrentJob = EMEEP_JOB_LOAD;

                            if (RESULT_OK == 
                                W25Q_ReadData(
                                     (EMEEP_GetDataFieldAddress(EMEEP_banksParams[nBankNumber].nLastValidRecordAddress) + nDataOffset),
                                      pDataBuffer, 
                                      nDataQty))
                            {
                                EMEEP_FlashCallback(EMEEP_READ_EVENT_ID, MEM_JOB_RESULT_OK);
                            }
                            else
                            {
                                // Update memory status
                                EMEEP_nMemoryStatusMask |= MEM_STATUS_READ_ERR_MASK;
                                nFuncResult = RESULT_NOT_OK;
                            }
                        }
                        else
                        {
                            // Data buffer is null pointer
                            SEGGER_RTT_printf(0, "Data buffer is null pointer");                       

                            nFuncResult = RESULT_NOT_OK;
                        }
                    }
                    else
                    {
                        // Source address is not valid
                        SEGGER_RTT_printf(0, "Source address is not valid");

                        nFuncResult = RESULT_NOT_OK;
                    }
                }
                else
                {
                    // Data quantity is not valid        
                    SEGGER_RTT_printf(0, "Data quantity is not valid");

                    nFuncResult = RESULT_NOT_OK;
                }
            }
            else
            {
                // Bank number is not valid
                SEGGER_RTT_printf(0, "Bank number is not valid");

                nFuncResult = RESULT_NOT_OK;
            }
        }
        else
        {
            // EMEEP program module is busy
            SEGGER_RTT_printf(0, "EMEEP program module is busy");

            nFuncResult = RESULT_NOT_OK;
        }
    }
    else
    {
        // Program module is not initialized
        SEGGER_RTT_printf(0, "Program module is not initialized");

        nFuncResult = RESULT_NOT_OK;
    }

    if (RESULT_NOT_OK == nFuncResult)
    {
        EMEEP_nJobResult  = MEM_JOB_RESULT_NOT_OK;
        // Reset job type
        EMEEP_nCurrentJob = EMEEP_JOB_IDLE;
    }

    return nFuncResult;

} // end of EMEEP_Load()



//**************************************************************************************************
// @Function      EMEEP_Store()
//--------------------------------------------------------------------------------------------------
// @Description   Stores a new data image to the emulated EEPROM memory from the specified buffer.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK     - function succeeded
//                RESULT_NOT_OK - function NOT succeeded
//--------------------------------------------------------------------------------------------------
// @Parameters    nTargetAddress - start address of the emulated EEPROM data to be written
//                pDataBuffer    - pointer to the emulated EEPROM data image
//                nDataQty       - size of the emulated EEPROM data image to be written
//**************************************************************************************************
STD_RESULT EMEEP_Store(const uint32_t nTargetAddress,
                       const uint8_t* const pDataBuffer,
                       const uint32_t nDataQty)
{
    STD_RESULT nFuncResult = RESULT_OK;

    // Checking whether initialization is done or not
    if (TRUE == EMEEP_bInitialized)
    {
        if (EMEEP_JOB_IDLE == EMEEP_nCurrentJob)
        {
            // Check bank number
            uint8_t nBankNumber = LOBYTE(HIWORD(nTargetAddress));

            if (nBankNumber < EMEEP_USED_BANKS_QTY)
            {
                uint16_t nDataOffset = LOWORD(nTargetAddress);
                uint32_t nRecordSize = EMEEP_banksStaticCfg[nBankNumber].nRecordSize;
                EMEEP_nCurrentBankNumber = nBankNumber;

                // Check data quantity parameter
                if (nDataQty <= EMEEP_banksStaticCfg[nBankNumber].nRecordDataSize)
                {
                    // Check target address
                    if ((uint32_t)(nDataOffset + nDataQty) <= EMEEP_banksStaticCfg[nBankNumber].nRecordDataSize)
                    {
                        // Check data buffer pointer
                        if (NULL_PTR != pDataBuffer)
                        {
                            uint32_t nByteNumber = 0UL;
                            uint32_t nSectorSize = 0UL;

                            #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                            EMEEP_diagCounters[nBankNumber].nStoreCallCnt ++;
                            #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

                            EMEEP_nJobResult = MEM_JOB_RESULT_PENDING;

                            // Prepare the next record
                            EMEEP_newRecordsContent[nBankNumber].nNumber = EMEEP_GetNumberFieldValue(EMEEP_banksParams[nBankNumber].nLastValidRecordAddress) + 1UL;

                            // Prepare signature field
                            EMEEP_newRecordsContent[nBankNumber].nSignature = EMEEP_RECORD_SIGNATURE_VALUE;

                            // Prepare data field
                            if(TRUE == EMEEP_banksParams[nBankNumber].bValidRecordFound)
                            {
                                // Bank already initialized
                                if(RESULT_OK == W25Q_ReadData(EMEEP_GetDataFieldAddress(EMEEP_banksParams[nBankNumber].nLastValidRecordAddress),
                                                              EMEEP_newRecordsContent[nBankNumber].dataArray,
                                                              EMEEP_banksStaticCfg[nBankNumber].nRecordDataSize))
                                {
                                    // Data read synchronously
                                    EMEEP_FlashCallback(EMEEP_READ_EVENT_ID, MEM_JOB_RESULT_OK);
                                }
                                else
                                {
                                    // Flash read failed
                                    EMEEP_FlashCallback(EMEEP_READ_EVENT_ID, MEM_JOB_RESULT_NOT_OK);
                                    nFuncResult = RESULT_NOT_OK;
                                }
                            }
                            else
                            {
                                // Bank not initialized
                                // Clear buffer
                                for (nByteNumber = 0U; nByteNumber < EMEEP_MAX_DATA_BUFFER_SIZE; nByteNumber ++)
                                {
                                    EMEEP_newRecordsContent[nBankNumber].dataArray[nByteNumber] = 0x00U;
                                }
                            }

                            if(RESULT_OK == nFuncResult)
                            {
                                // Data buffer prepared to enter new data
                                // Write new data to buffer
                                for (nByteNumber = 0U; nByteNumber < nDataQty; nByteNumber ++)
                                {
                                    EMEEP_newRecordsContent[nBankNumber].dataArray[nDataOffset + nByteNumber] = pDataBuffer[nByteNumber];
                                }

                                // Prepare checksum field
                                // N.B.: checksum field in EMEEP_RECORD structure
                                // are always 32-bit long for memory alignment purpose, NOT the checksum value itself!
                                EMEEP_newRecordsContent[nBankNumber].nChecksum =
                                    (uint32_t)EMEEP_CalcChecksum((uint8_t*)&EMEEP_newRecordsContent[nBankNumber].nNumber,
                                                            (EMEEP_NUMBER_FIELD_SIZE +
                                                            EMEEP_banksStaticCfg[nBankNumber].nRecordDataSize));

                                nSectorSize = W25Q_CAPACITY_SECTOR_BYTES;

                                if ((EMEEP_banksParams[nBankNumber].nNextRecordAddress & (nSectorSize - 1U)) == 0U)
                                {
                                    // Sector start address is reached
                                    // Erase some sectors (prepare it for writing)
                                    uint32_t nBytesToErase = 0U;
                                    if (nRecordSize < nSectorSize)
                                    {
                                        // Erase one sector to write N records
                                        nBytesToErase = nSectorSize;
                                    }
                                    else
                                    {
                                        // Erase N sectors to write one record
                                        nBytesToErase = nRecordSize;
                                    }

                                        #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                                        EMEEP_diagCounters[nBankNumber].nFlashEraseAttemptCnt ++;
                                        #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

                                        // Set a new current job
                                        EMEEP_nCurrentJob = EMEEP_JOB_STORE;

                                        if (RESULT_OK ==
                                                W25Q_EraseBlock(EMEEP_banksParams[nBankNumber].nNextRecordAddress,
                                                                W25Q_CAPACITY_SECTOR_BYTES))
                                        {
                                            // Wait for callback erase event
                                            EMEEP_FlashCallback(EMEEP_ERASE_EVENT_ID, MEM_JOB_RESULT_OK);

                                            if (RESULT_OK == W25Q_WriteData(EMEEP_banksParams[EMEEP_nCurrentBankNumber].nNextRecordAddress,
                                                                            (U8*)&EMEEP_newRecordsContent[EMEEP_nCurrentBankNumber],
                                                                            EMEEP_banksStaticCfg[EMEEP_nCurrentBankNumber].nRecordSize))
                                            {
                                                // Just wait for callback write event
                                                EMEEP_FlashCallback(EMEEP_WRITE_EVENT_ID, MEM_JOB_RESULT_OK);
                                            }
                                            else
                                            {
                                                // Job failed
                                                EMEEP_nJobResult = MEM_JOB_RESULT_NOT_OK;
                                                EMEEP_nCurrentJob = EMEEP_JOB_IDLE;

                                                // Update memory status
                                                EMEEP_nMemoryStatusMask |= MEM_STATUS_WRITE_ERR_MASK;

                                                if (NULL_PTR != EMEEP_pStoreCallback)
                                                {
                                                    // Notify user
                                                    (EMEEP_pStoreCallback)(EMEEP_nStoreCallbackEventID, EMEEP_nJobResult);
                                                }
                                            }
                                        }
                                        else
                                        {
                                            // Update memory status
                                            EMEEP_nMemoryStatusMask |= MEM_STATUS_ERASE_ERR_MASK;
                                            nFuncResult = RESULT_NOT_OK;
                                        }
								}
								else
								{
									// Sector start address is NOT reached
									// Start to write memory without sector erasing
                                    #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                                    EMEEP_diagCounters[nBankNumber].nFlashWriteAttemptCnt ++;
                                    #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

                                    // Set a new current job
                                    EMEEP_nCurrentJob = EMEEP_JOB_STORE;

                                    if (RESULT_OK ==
                                        W25Q_WriteData(EMEEP_banksParams[nBankNumber].nNextRecordAddress,
                                                        (uint8_t*)&EMEEP_newRecordsContent[nBankNumber],
                                                        nRecordSize))
                                    {
                                        // Wait for callback write event
                                        EMEEP_FlashCallback(EMEEP_WRITE_EVENT_ID, MEM_JOB_RESULT_OK);
                                    }
                                    else
                                    {
                                        // Update memory status
                                        EMEEP_nMemoryStatusMask |= MEM_STATUS_WRITE_ERR_MASK;
                                        nFuncResult = RESULT_NOT_OK;
                                    }

								}
                            }
                            else
                            {
                                // Read last record fail
                                nFuncResult = RESULT_NOT_OK;
                            }
                        }
                        else
                        {
                            // Data buffer is null pointer
                            SEGGER_RTT_printf(0, "Data buffer is null pointer");
                            
                            nFuncResult = RESULT_NOT_OK;
                        }
                    }
                    else
                    {
                        // Target address is not valid                                                  
                        SEGGER_RTT_printf(0, "Target address is not valid");

                        nFuncResult = RESULT_NOT_OK;
                    }
                }
                else
                {
                    // Data quantity is not valid
                    SEGGER_RTT_printf(0, "Data quantity is not valid");
                    
                    nFuncResult = RESULT_NOT_OK;
                }
            }
            else
            {
                // Bank number is not valid
                SEGGER_RTT_printf(0, "Bank number is not valid");
                
                nFuncResult = RESULT_NOT_OK;
            }
        }
        else
        {
            // Program module is busy
            SEGGER_RTT_printf(0, "Program module is busy");
            
            nFuncResult = RESULT_NOT_OK;
        }
    }
    else
    {
        // Program module is not initialized
        SEGGER_RTT_printf(0, "Program module is not initialized");
        
        nFuncResult = RESULT_NOT_OK;
    }

    if (RESULT_NOT_OK == nFuncResult)
    {
        EMEEP_nJobResult  = MEM_JOB_RESULT_NOT_OK;
        // Reset job type
        EMEEP_nCurrentJob = EMEEP_JOB_IDLE;
    }

    return nFuncResult;

} // end of EMEEP_Store()



//**************************************************************************************************
// @Function      EMEEP_GetJobResult()
//--------------------------------------------------------------------------------------------------
// @Description   Returns the last emulated EEPROM memory job result.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK     - function succeeded
//                RESULT_NOT_OK - function NOT succeeded
//--------------------------------------------------------------------------------------------------
// @Parameters    pJobResult - pointer to a buffer receiving job result
//**************************************************************************************************
uint8_t EMEEP_GetJobResult(void)
{
    uint8_t nFuncResult = MEM_JOB_RESULT_NOT_OK;

    // Checking whether initialization is done or not
    if (TRUE == EMEEP_bInitialized)
    {
        nFuncResult = EMEEP_nJobResult;
    }
    else
    {
        // Program module is not initialized
        SEGGER_RTT_printf(0, "Program module is not initialized");
    }
        
    return nFuncResult;

} // end of EMEEP_GetJobResult()



//**************************************************************************************************
// @Function      EMEEP_GetMemoryStatus()
//--------------------------------------------------------------------------------------------------
// @Description   Returns memory status mask.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK     - function succeeded
//                RESULT_NOT_OK - function NOT succeeded
//--------------------------------------------------------------------------------------------------
// @Parameters    pStatusMask - pointer to a buffer receiving status mask
//**************************************************************************************************
uint32_t EMEEP_GetMemoryStatus(void)
{
    uint32_t nFuncResult = 0UL;

    // Checking whether initialization is done or not
    if (TRUE == EMEEP_bInitialized)
    {
        nFuncResult = EMEEP_nMemoryStatusMask;
    }
    else
    {
        // Program module is not initialized
        SEGGER_RTT_printf(0, "Program module is not initialized");
    }

    return nFuncResult;

} // end of EMEEP_GetMemoryStatus()



//**************************************************************************************************
// @Function      EMEEP_SetMemoryStatus()
//--------------------------------------------------------------------------------------------------
// @Description   Sets a new memory status mask.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK     - function succeeded
//                RESULT_NOT_OK - function NOT succeeded
//--------------------------------------------------------------------------------------------------
// @Parameters    nStatusMask - new memory status mask
//**************************************************************************************************
void EMEEP_SetMemoryStatus(const uint32_t nStatusMask)
{
    // Checking whether initialization is done or not
    if (TRUE == EMEEP_bInitialized)
    {
        // Reset the specified status flags
        EMEEP_nMemoryStatusMask &= ~nStatusMask;
    }
    else
    {
        // Program module is not initialized
        SEGGER_RTT_printf(0, "Program module is not initialized");
    }

} // end of EMEEP_SetMemoryStatus()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      EMEEP_FindLastValidRecord()
//--------------------------------------------------------------------------------------------------
// @Description   Finds the last valid record in the memory.
//--------------------------------------------------------------------------------------------------
// @Notes         If the last entry are not
//                valid, this function performs recursive records rollback, trying to find any previous
//                valid records, otherwise function sets the default entry as the last valid entry.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   EMEEP_PREVIOUS_RECORD - the last entry are invalid, function find
//                                         the last valid entry data (performs rollback action)
//                EMEEP_SUCCESS        - function find the last valid entry data
//                EMEEP_DEFAULT_RECORD  - there is no valid entry
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void EMEEP_FindLastValidRecord(const uint8_t nBankNumber)
{
    // Current record index for search process
    uint32_t nRecordIndex = 0U;
    // Search status
    BOOLEAN bRecordFound = FALSE;
    // Current record address
    uint32_t nCurrentRecordAddress = 0U;
    // Address of the record with the largest number
    uint32_t nRecordWithMaxNum = 0UL;
    // Largest number of the record
    EMEEP_NUMBER_FIELD_TYPE nMaximumRecordNumber = 0UL;

    // 1) Find the latest record
    nCurrentRecordAddress = EMEEP_banksStaticCfg[nBankNumber].nStartAddress;
    nRecordWithMaxNum = nCurrentRecordAddress;

    do
    {
        if (TRUE == EMEEP_IsSignatureValid(nCurrentRecordAddress))
        {
            if (NULL_PTR != nCurrentRecordAddress)
            {
                if (EMEEP_GetNumberFieldValue(nCurrentRecordAddress) >= nMaximumRecordNumber)
                {
                    nMaximumRecordNumber = EMEEP_GetNumberFieldValue(nCurrentRecordAddress);
                    nRecordWithMaxNum = nCurrentRecordAddress;
                    bRecordFound = TRUE;
                }
            }
        }

        nCurrentRecordAddress += EMEEP_banksStaticCfg[nBankNumber].nRecordSize;
        nRecordIndex++;
    }
    while (nRecordIndex < EMEEP_banksStaticCfg[nBankNumber].nMemoryCapacity);

    if (TRUE == bRecordFound)
    {
        // 2) Validate the latest record
        if (TRUE == EMEEP_IsChecksumValid(nBankNumber,
                                          nRecordWithMaxNum))
        {
            // The last record is valid
            EMEEP_banksParams[nBankNumber].nLastValidRecordAddress = nRecordWithMaxNum;

            // Calculate the next record address
            EMEEP_banksParams[nBankNumber].nNextRecordAddress = 
                EMEEP_GetNextRecordAddr(nBankNumber,
                                        EMEEP_banksParams[nBankNumber].nLastValidRecordAddress);

            // Set actual status flag
            EMEEP_banksParams[nBankNumber].bValidRecordFound = TRUE;

            #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
            EMEEP_diagCounters[nBankNumber].nLastValidRecordNumber =
                EMEEP_GetNumberFieldValue(EMEEP_banksParams[nBankNumber].nLastValidRecordAddress);
            #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
        }
        else
        {
            // The last record is not valid
            // find valid record from the previous records
            BOOLEAN bPrevValidRecordFound = FALSE;
            uint32_t nPreviousValidRecordAddress = EMEEP_FindPreviousValidRecord(nBankNumber,
                                                                     nRecordWithMaxNum,
                                                                     &bPrevValidRecordFound);

            if (TRUE == bPrevValidRecordFound)
            {
                uint32_t nSectorSize         = 0UL;
                BOOLEAN bMemoryBlank    = FALSE;
                BOOLEAN bSectorBoundary = FALSE;
                uint32_t nNextRecordAddress  = (uint32_t)(EMEEP_GetNextRecordAddr(nBankNumber,
                                                    EMEEP_GetNextRecordAddr(nBankNumber,
                                                        nPreviousValidRecordAddress)));

                // Previous valid record is found
                EMEEP_banksParams[nBankNumber].nLastValidRecordAddress = nPreviousValidRecordAddress;

                // Set actual status flag
                EMEEP_banksParams[nBankNumber].bValidRecordFound = TRUE;

                #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                EMEEP_diagCounters[nBankNumber].nLastValidRecordNumber = 
                    EMEEP_GetNumberFieldValue(EMEEP_banksParams[nBankNumber].nLastValidRecordAddress);
                #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

                nSectorSize = W25Q_CAPACITY_SECTOR_BYTES;

                // Find blank memory for the next record OR the next sector boundary
                do
                {
                    // Detect sector boundary
                    if (0U == (nNextRecordAddress & (nSectorSize - 1U)))
                    {
                        bSectorBoundary = TRUE;
                    }
                    else
                    {
                        // Detect blank memory
                        W25Q_IsAreaBlank(nNextRecordAddress,
                                          EMEEP_banksStaticCfg[nBankNumber].nRecordSize,
                                          &bMemoryBlank);

                        if (FALSE == bMemoryBlank)
                        {
                            nNextRecordAddress = 
                                (uint32_t)EMEEP_GetNextRecordAddr(nBankNumber, nNextRecordAddress);
                        }
                    }
                }
                while ((nNextRecordAddress != nPreviousValidRecordAddress) &&
                       (FALSE == bMemoryBlank)                             &&
                       (FALSE == bSectorBoundary));

                if ((TRUE == bSectorBoundary) || (TRUE == bMemoryBlank))
                {
                    // Blank memory (or sector boundary) for the next record is found
                    EMEEP_banksParams[nBankNumber].nNextRecordAddress = nNextRecordAddress;
                }
            }
            else
            {
                // There is no valid records
                // Start from the beginning of the EMEEP memory
                EMEEP_banksParams[nBankNumber].bValidRecordFound = FALSE;
                EMEEP_banksParams[nBankNumber].nLastValidRecordAddress = (uint32_t)&EMEEP_newRecordsContent[nBankNumber];
                EMEEP_banksParams[nBankNumber].nNextRecordAddress = EMEEP_banksStaticCfg[nBankNumber].nStartAddress;
            }
        }
    }
    else
    {
        // There is no valid records
        // Start from the beginning of the EMEEP memory
        EMEEP_banksParams[nBankNumber].bValidRecordFound = FALSE;
        EMEEP_banksParams[nBankNumber].nLastValidRecordAddress = (uint32_t)&EMEEP_newRecordsContent[nBankNumber];
        EMEEP_banksParams[nBankNumber].nNextRecordAddress = EMEEP_banksStaticCfg[nBankNumber].nStartAddress;
    }
    
} // end of EMEEP_FindLastValidRecord()



//**************************************************************************************************
// @Function      EMEEP_FindPreviousValidRecord()
//--------------------------------------------------------------------------------------------------
// @Description   Finds previous valid record in the memory,
//                starting downwards from the specified record.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   Valid record is found: pointer to the previous valid record.
//                Valid record is NOT found: null pointer.
//--------------------------------------------------------------------------------------------------
// @Parameters    pFindFrom - pointer to the record to start search from
//**************************************************************************************************
static uint32_t EMEEP_FindPreviousValidRecord(const uint8_t  nBankNumber,
                                         const uint32_t nFindFrom,
                                         BOOLEAN* const pRecordFound)
{
    // Found record address
    uint32_t nFoundRecordAddress = NULL_PTR;
    // Current record address
    uint32_t nCurrentRecordAddress = nFindFrom;
    // Search status
    BOOLEAN bEndOfSearch = FALSE;

    do
    {
        // Go to the previous record
        nCurrentRecordAddress = EMEEP_GetPrevRecordAddr(nBankNumber, nCurrentRecordAddress);

        // Check for the last record
        if (nCurrentRecordAddress != nFindFrom)
        {
            // Search previous valid record
            if (TRUE == EMEEP_IsSignatureValid(nCurrentRecordAddress))
            {
                if (TRUE == EMEEP_IsChecksumValid(nBankNumber, nCurrentRecordAddress))
                {
                    // Previous valid record is found
                    nFoundRecordAddress = nCurrentRecordAddress;
                    bEndOfSearch = TRUE;
                    *pRecordFound = TRUE;
                }
            }
        }
        else
        {
            // "pFindFrom" record is reached
            // There is no previous valid records
            bEndOfSearch = TRUE;
            *pRecordFound = FALSE;
        }
    }
    while (FALSE == bEndOfSearch);

    return nFoundRecordAddress;

} // end of EMEEP_FindPreviousValidRecord()



//**************************************************************************************************
// @Function      EMEEP_GetNextRecordAddr()
//--------------------------------------------------------------------------------------------------
// @Description   Increments record address taking into account EMEEP memory margins.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   Incremented record address.
//--------------------------------------------------------------------------------------------------
// @Parameters    nBankNumber - record's memory bank number
//                nRecordAddress - record address to increment
//**************************************************************************************************
static uint32_t EMEEP_GetNextRecordAddr(const uint8_t  nBankNumber,
                                   const uint32_t nRecordAddress)
{
    uint32_t nNewRecordAddress = 0UL;

    if ((nRecordAddress + EMEEP_banksStaticCfg[nBankNumber].nRecordSize) 
        >= (EMEEP_banksStaticCfg[nBankNumber].nEndAddress + 1U))
    {
        nNewRecordAddress = EMEEP_banksStaticCfg[nBankNumber].nStartAddress;
    }
    else
    {
        nNewRecordAddress = nRecordAddress + EMEEP_banksStaticCfg[nBankNumber].nRecordSize;
    }

    return nNewRecordAddress;

} // end of EMEEP_GetNextRecordAddr()



//**************************************************************************************************
// @Function      EMEEP_GetPrevRecordAddr()
//--------------------------------------------------------------------------------------------------
// @Description   Decrements record address taking into account EMEEP memory margins.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   Decremented record address.
//--------------------------------------------------------------------------------------------------
// @Parameters    nBankNumber - record's memory bank number
//                nRecordAddress - record address to decrement
//**************************************************************************************************
static uint32_t EMEEP_GetPrevRecordAddr(const uint8_t  nBankNumber,
                                   const uint32_t nRecordAddress)
{
    uint32_t nNewRecordAddress = 0UL;

    if ((nRecordAddress - EMEEP_banksStaticCfg[nBankNumber].nRecordSize) < 
        EMEEP_banksStaticCfg[nBankNumber].nStartAddress)
    {
        nNewRecordAddress = EMEEP_banksStaticCfg[nBankNumber].nEndAddress + 1U -
            EMEEP_banksStaticCfg[nBankNumber].nRecordSize;
    }
    else
    {
        nNewRecordAddress = nRecordAddress - EMEEP_banksStaticCfg[nBankNumber].nRecordSize;
    }

    return nNewRecordAddress;

} // end of EMEEP_GetPrevRecordAddr()



//**************************************************************************************************
// @Function      EMEEP_IsSignatureValid()
//--------------------------------------------------------------------------------------------------
// @Description   Checks whether signature of the specified record is valid or not.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   EMEEP_SUCCESS - signature of the specified entry are valid.
//                EMEEP_INVALID_SIGNATURE - signature of the specified entry is not valid.
//--------------------------------------------------------------------------------------------------
// @Parameters    pRecord - pointer to the entry, which signature should be check.
//**************************************************************************************************
static BOOLEAN EMEEP_IsSignatureValid(const uint32_t nRecordAddress)
{
    BOOLEAN bFuncResult = FALSE;
    
    EMEEP_SIGNATURE_FIELD_TYPE nSignatureValue = 
        *(EMEEP_SIGNATURE_FIELD_TYPE*)(nRecordAddress + EMEEP_CHECKSUM_FIELD_SIZE);

    bFuncResult = (EMEEP_RECORD_SIGNATURE_VALUE == nSignatureValue) ?
                  TRUE : FALSE;

    return bFuncResult;

} // end of EMEEP_IsSignatureValid()



//**************************************************************************************************
// @Function      EMEEP_IsChecksumValid()
//--------------------------------------------------------------------------------------------------
// @Description   Checks whether checksum of the specified record is valid or not.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   Checksum validation result.
//--------------------------------------------------------------------------------------------------
// @Parameters    nBankNumber    - EMEEP bank number
//                nRecordAddress - address of the record which checksum need to be validated
//**************************************************************************************************
static BOOLEAN EMEEP_IsChecksumValid(const uint8_t  nBankNumber,
                                     const uint32_t nRecordAddress)
{
    BOOLEAN bChecksumValid = FALSE;

    // Current checksum value
    EMEEP_CHECKSUM_FIELD_TYPE nRecordChecksum =
        EMEEP_CalcChecksum((uint8_t*)EMEEP_GetNumberFieldAddress(nRecordAddress),
                           (EMEEP_banksStaticCfg[nBankNumber].nRecordDataSize + 
                            EMEEP_NUMBER_FIELD_SIZE));

    if (nRecordChecksum == EMEEP_GetChecksumFieldValue(nRecordAddress))
    {
        bChecksumValid = TRUE;
    }

    return bChecksumValid;

} // end of EMEEP_IsChecksumValid()



//**************************************************************************************************
// @Function      EMEEP_CalcChecksum()
//--------------------------------------------------------------------------------------------------
// @Description   Calculates 32-bit arithmetic checksum of the specified data array.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   32-bit arithmetic checksum of the specified data array.
//--------------------------------------------------------------------------------------------------
// @Parameters    pDataBuffer - pointer to the data array
//                nDataQty    - size of the specified array to be calculated
//**************************************************************************************************
static EMEEP_CHECKSUM_FIELD_TYPE EMEEP_CalcChecksum(const uint8_t* const pDataBuffer,
                                                    const uint32_t nDataQty)
{
    // Current checksum value
    EMEEP_CHECKSUM_FIELD_TYPE nCurrentChecksum = 0U;
    // Current byte number
    uint32_t nByteNumber = 0U;

    for (nByteNumber = 0U; nByteNumber < nDataQty; nByteNumber++)
    {
        nCurrentChecksum += pDataBuffer[nByteNumber];
    }

    return nCurrentChecksum;

} // end of EMEEP_CalcChecksum()



//**************************************************************************************************
// @Function      EMEEP_GetChecksumFieldAddress()
//--------------------------------------------------------------------------------------------------
// @Description   Returns "checksum" field address from the specified record address.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   "Checksum" field address.
//--------------------------------------------------------------------------------------------------
// @Parameters    nRecordAddress - record start address
//**************************************************************************************************
static uint32_t EMEEP_GetChecksumFieldAddress(const uint32_t nRecordAddress)
{
    return nRecordAddress;

} // end of EMEEP_GetChecksumFieldAddress()



//**************************************************************************************************
// @Function      EMEEP_GetChecksumFieldValue()
//--------------------------------------------------------------------------------------------------
// @Description   Returns "checksum" field value from the specified record address.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   "Checksum" field value.
//--------------------------------------------------------------------------------------------------
// @Parameters    nRecordAddress - record start address
//**************************************************************************************************
static EMEEP_CHECKSUM_FIELD_TYPE EMEEP_GetChecksumFieldValue(const uint32_t nRecordAddress)
{
    return *(EMEEP_CHECKSUM_FIELD_TYPE*)EMEEP_GetChecksumFieldAddress(nRecordAddress);

} // end of EMEEP_GetChecksumFieldValue()



//**************************************************************************************************
// @Function      EMEEP_GetDataFieldAddress()
//--------------------------------------------------------------------------------------------------
// @Description   Calculates "data" field address from the specified record address.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   "Data" field address.
//--------------------------------------------------------------------------------------------------
// @Parameters    nRecordAddress - record start address
//**************************************************************************************************
static uint32_t EMEEP_GetDataFieldAddress(const uint32_t nRecordAddress)
{
    return (nRecordAddress             + 
            EMEEP_HEADER_SIZE);

} // end of EMEEP_GetDataFieldAddress()



//**************************************************************************************************
// @Function      EMEEP_GetNumberFieldAddress()
//--------------------------------------------------------------------------------------------------
// @Description   Returns "number" field address from the specified record address.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   "Number" field address.
//--------------------------------------------------------------------------------------------------
// @Parameters    nRecordAddress - record start address
//**************************************************************************************************
static uint32_t EMEEP_GetNumberFieldAddress(const uint32_t nRecordAddress)
{
    return (nRecordAddress             +
            EMEEP_CHECKSUM_FIELD_SIZE  +
            EMEEP_SIGNATURE_FIELD_SIZE);

} // end of EMEEP_GetNumberFieldAddress()



//**************************************************************************************************
// @Function      EMEEP_GetNumberFieldValue()
//--------------------------------------------------------------------------------------------------
// @Description   Returns "number" field value from the specified record address.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   "Number" field value.
//--------------------------------------------------------------------------------------------------
// @Parameters    nRecordAddress - record start address
//**************************************************************************************************
static uint32_t EMEEP_GetNumberFieldValue(const uint32_t nRecordAddress)
{
    return *(EMEEP_NUMBER_FIELD_TYPE*)EMEEP_GetNumberFieldAddress(nRecordAddress);

} // end of EMEEP_GetNumberFieldValue()



//**************************************************************************************************
// @Function      EMEEP_FlashCallback()
//--------------------------------------------------------------------------------------------------
// @Description   Calls on every end-of-job flash driver event.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    nEventID   - identifier of the flash driver's event
//                nJobResult - flash job result
//**************************************************************************************************
static void EMEEP_FlashCallback(const uint8_t nEventID,
                                const uint8_t nJobResult)
{
    switch (nEventID)
    {
        case EMEEP_READ_EVENT_ID:
            if (MEM_JOB_RESULT_OK == nJobResult)
            {
                #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                EMEEP_diagCounters[EMEEP_nCurrentBankNumber].nFlashReadSuccessCnt ++;
                EMEEP_diagCounters[EMEEP_nCurrentBankNumber].nBytesReadSuccessful +=
                    EMEEP_diagCounters[EMEEP_nCurrentBankNumber].nBytesToRead;
                #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

                EMEEP_nJobResult  = MEM_JOB_RESULT_OK;
            }
            else
            {
                // Update memory status
                EMEEP_nMemoryStatusMask |= MEM_STATUS_READ_ERR_MASK;
                EMEEP_nJobResult = MEM_JOB_RESULT_NOT_OK;
            }

            // Job complete
            EMEEP_nCurrentJob = EMEEP_JOB_IDLE;

            if (NULL_PTR != EMEEP_pLoadCallback)
            {
                // Notify user
                (EMEEP_pLoadCallback)(EMEEP_nLoadCallbackEventID, EMEEP_nJobResult);
            }
            break;

        case EMEEP_ERASE_EVENT_ID:
            if (MEM_JOB_RESULT_OK == nJobResult)
            {
                #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                uint32_t nSectorSize = 0UL;
                EMEEP_diagCounters[EMEEP_nCurrentBankNumber].nFlashEraseSuccessCnt ++;
                nSectorSize = FLASH_GetSectorSize(EMEEP_banksParams[EMEEP_nCurrentBankNumber].nNextRecordAddress);
                EMEEP_diagCounters[EMEEP_nCurrentBankNumber].nBytesErasedSuccessful += nSectorSize;
                #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

                #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                EMEEP_diagCounters[EMEEP_nCurrentBankNumber].nFlashWriteAttemptCnt ++;
                #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
            }
            else
            {
                // Job failed
                EMEEP_nJobResult = MEM_JOB_RESULT_NOT_OK;
                EMEEP_nCurrentJob = EMEEP_JOB_IDLE;

                // Update memory status
                EMEEP_nMemoryStatusMask |= MEM_STATUS_ERASE_ERR_MASK;

                if (NULL_PTR != EMEEP_pStoreCallback)
                {
                    // Notify user
                    (EMEEP_pStoreCallback)(EMEEP_nStoreCallbackEventID, EMEEP_nJobResult);
                }
            }
            break;

        case EMEEP_WRITE_EVENT_ID:
            if (MEM_JOB_RESULT_OK == nJobResult)
            {
                uint8_t nMemoryWearingLevel = 0U;
                uint32_t nEraseWriteCycles = 0UL;

                #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                EMEEP_diagCounters[EMEEP_nCurrentBankNumber].nFlashWriteSuccessCnt ++;
                EMEEP_diagCounters[EMEEP_nCurrentBankNumber].nBytesWrittenSuccessful += 
                    EMEEP_banksStaticCfg[EMEEP_nCurrentBankNumber].nRecordSize;
                #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)

                // Record successfully written
                EMEEP_banksParams[EMEEP_nCurrentBankNumber].bValidRecordFound = TRUE;

                EMEEP_banksParams[EMEEP_nCurrentBankNumber].nLastValidRecordAddress =
                    EMEEP_banksParams[EMEEP_nCurrentBankNumber].nNextRecordAddress;
                EMEEP_nJobResult = MEM_JOB_RESULT_OK;

                #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                EMEEP_diagCounters[EMEEP_nCurrentBankNumber].nLastValidRecordNumber =
                    EMEEP_GetNumberFieldValue(EMEEP_banksParams[EMEEP_nCurrentBankNumber].nLastValidRecordAddress);
                #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                
                nEraseWriteCycles = EMEEP_GetNumberFieldValue(EMEEP_banksParams[EMEEP_nCurrentBankNumber].nLastValidRecordAddress) /
                                    EMEEP_banksStaticCfg[EMEEP_nCurrentBankNumber].nMemoryCapacity;
                nMemoryWearingLevel =
                    (uint8_t)((nEraseWriteCycles * EMEEP_MAX_PERCENTS) / 
                         EMEEP_banksStaticCfg[EMEEP_nCurrentBankNumber].nMemoryEndurance);
                
                if (nMemoryWearingLevel >= EMEEP_MAX_PERCENTS)
                {
                    // Update memory status
                    EMEEP_nMemoryStatusMask |= MEM_STATUS_WEAR_THRS_2_ERR_MASK;
                }
                else if (nMemoryWearingLevel >= EMEEP_WEAR_LEVEL_THRS_1)
                {
                    // Update memory status
                    EMEEP_nMemoryStatusMask |= MEM_STATUS_WEAR_THRS_1_WARN_MASK;
                }
                else
                {
                     DoNothing();
                }

                #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
                EMEEP_diagCounters[EMEEP_nCurrentBankNumber].nMemoryWearingLevel = 
                    nMemoryWearingLevel;
                #endif // #if (ON == EMEEP_INTERNAL_DIAGNOSTICS)
            }
            else
            {
                // Update memory status
                EMEEP_nMemoryStatusMask |= MEM_STATUS_WRITE_ERR_MASK;
                EMEEP_nJobResult = MEM_JOB_RESULT_NOT_OK;
            }

            // Calculate address for the next record
            EMEEP_banksParams[EMEEP_nCurrentBankNumber].nNextRecordAddress = 
                EMEEP_GetNextRecordAddr(EMEEP_nCurrentBankNumber,
                                        EMEEP_banksParams[EMEEP_nCurrentBankNumber].nLastValidRecordAddress);

            // Job complete
            EMEEP_nCurrentJob = EMEEP_JOB_IDLE;

            if (NULL_PTR != EMEEP_pStoreCallback)
            {
                // Notify user
                (EMEEP_pStoreCallback)(EMEEP_nStoreCallbackEventID, EMEEP_nJobResult);
            }
            break;

        default:
            // Should not get here, 'cause there is no other events
            DoNothing();
            break;
    }

} // end of EMEEP_FlashCallback()



//****************************************** end of file *******************************************
