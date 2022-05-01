//**************************************************************************************************
// @Module        DS18B20
// @Filename      ds18b20.c
//--------------------------------------------------------------------------------------------------
// @Platform      stm32
//--------------------------------------------------------------------------------------------------
// @Compatible    stm32
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the oneWire functionality.
//
//
//                Abbreviations:
//                  None.
//
//
//                Global (public) functions:

//
//                Local (private) functions:

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
#include "ds18b20.h"
// Add 1-wire interface
#include "OneWire.h"


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

// ROM function Commands
// Read ROM
#define DS18B20_READ_ROM_CMD                (0x33U)
// Match ROM
#define DS18B20_MATCH_ROM_CMD               (0x55U)
// Skip ROM
#define DS18B20_SKIP_ROM_CMD                (0xCCU)
// Search ROM
#define DS18B20_SEARCH_ROM_CMD              (0xF0U)
// Alarm search
#define DS18B20_ALARM_SEARCH_CMD            (0xECU)

// Memory command functions
// Write scratchpad
#define DS18B20_WRITE_SCRATCHPAD            (0x4EU)
// Read scratchpad
#define DS18B20_READ_SCRATCHPAD             (0xBEU)
// Copy scratchpad
#define DS18B20_COPY_SCRATCHPAD             (0x48U)
// Convert T
#define DS18B20_CONVERT_T                   (0x44U)
// Recall E2
#define DS18B20_RECALL_E2                   (0xB8U)
// Read power supply
#define DS18B20_READ_POWER_SUPPLY           (0xB4U)

#define DS18B20_CONVERSION_TIME_US          (1000000U)
// Size scratchpad
#define DS18B20_SCRATCHPAD_SIZE             (9U)
// Size scratchpad allow writing
#define DS18B20_SCRATCHPAD_WRITE_SIZE       (3U)
// Addr scratchpad TH
#define DS18B20_SCRATCHPAD_TH_ADDR          (2U)
// Addr scratchpad TL
#define DS18B20_SCRATCHPAD_TL_ADDR          (3U)
// Addr scratchpad CONFIG
#define DS18B20_SCRATCHPAD_TL_CONFIG        (4U)
// number bits temperature data
#define DS18B20_NUM_BITS_TEMPERATURE        (11U)
// number bytes ID
#define DS18B20_SIZE_ID_BYTES               (8U)

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// None.


//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************
// Match ROM function
static STD_RESULT DS18B20_MatchID(const uint8_t nCh, uint64_t ID);
// Skip ROM function
static void DS18B20_SkipID(const uint8_t nCh);
// Read scratchpad
static void DS18B20_ReadScratchPad(const uint8_t nCh, uint8_t* data);
// Start convert T
static STD_RESULT DS18B20_ConvertT(const uint8_t nCh);
// Get temperature value from scratchpad
static float DS18B20_GetTemFromScratchpad(const uint8_t* scratchpad);
// Calculate CRC
static uint8_t DS18B20_CalculateCRC(uint8_t* data, uint8_t len);
// Write scratchpad
static void DS18B20_WriteScratchPad(const uint8_t nCh, uint8_t TL, uint8_t TH, uint8_t resolution);

//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************


//**************************************************************************************************
// @Function      DS18B20_GetID()
//--------------------------------------------------------------------------------------------------
// @Description   Get ID DS18B20 sensor
//--------------------------------------------------------------------------------------------------
// @Notes         Only one sensor can be on the bus to correct get ID
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK - crc correct and sensor presence
//                RESULT_NOT_OK - crc doesn't correct or sensor doesn't presence
//--------------------------------------------------------------------------------------------------
// @Parameters    ID - pointer data to store ID
//                nCh - channel One Wire
//**************************************************************************************************
STD_RESULT DS18B20_GetID(const uint8_t nCh, uint64_t *const ID)
{
    STD_RESULT result = RESULT_NOT_OK;
    uint8_t crc=0;

    // Reset 1-wire
    enONE_WIRE_PRESENCE status;
    if (RESULT_OK == ONE_WIRE_reset(nCh,&status))
    {
        if (ONE_WIRE_PRESENCE == status)
        {
            // Send command READ ROM
            ONE_WIRE_writeByte(nCh,DS18B20_READ_ROM_CMD);
            // Read 8 bytes
            for (int i=0;i<DS18B20_SIZE_ID_BYTES;i++)
            {
                uint8_t byteVal=0;
                ONE_WIRE_readByte(nCh,&byteVal);
                *ID |= ((uint64_t)byteVal) << i*8;
            }
            crc = DS18B20_CalculateCRC((uint8_t*)ID, DS18B20_SIZE_ID_BYTES-1);
            if (crc == (uint8_t)(*ID >> (DS18B20_SIZE_ID_BYTES*8-8)))
            {
                result = RESULT_OK;
            }
            else
            {
                result = RESULT_NOT_OK;
            }
        }
        else
        {
            result = RESULT_NOT_OK;
        }
    }
    else
    {
        result = RESULT_NOT_OK;
    }

    return result;
}
//end of DS18B20_GetID()



//**************************************************************************************************
// @Function      DS18B20_GetTemperature()
//--------------------------------------------------------------------------------------------------
// @Description   Read scratchpad
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    *t - Pointer data to store temperature
//                ID - ID of sensor
//**************************************************************************************************
STD_RESULT DS18B20_GetTemperature(uint8_t nCh, const uint64_t *const ID, float *const t )
{
    STD_RESULT result = RESULT_NOT_OK;
    uint8_t scratchpad[DS18B20_SCRATCHPAD_SIZE];
    uint8_t crc = 0;

    // Detect sensor/sensors
    enONE_WIRE_PRESENCE status;
    if (RESULT_OK == ONE_WIRE_reset(nCh,&status))
    {
        if (ONE_WIRE_PRESENCE == status)
        {
            // Match ROM ID
            DS18B20_MatchID(nCh,*ID);
            // Convert T
            if (RESULT_OK == DS18B20_ConvertT(nCh))
            {
                // Detect sensor/sensors
                ONE_WIRE_reset(nCh,&status);
                if (ONE_WIRE_PRESENCE == status)
                {
                    // Match ROM ID
                    DS18B20_MatchID(nCh,*ID);
                    // read scratchpad
                    DS18B20_ReadScratchPad(nCh,scratchpad);
                    // calculate crc
                    crc = DS18B20_CalculateCRC(scratchpad, DS18B20_SCRATCHPAD_SIZE-1);
                    if (crc == scratchpad[8])
                    {
                        // Get temperature
                        *t = DS18B20_GetTemFromScratchpad(scratchpad);
                        result = RESULT_OK;
                    }
                    else
                    {
                        result = RESULT_NOT_OK;
                    }
                }
                else
                {
                    result = RESULT_NOT_OK;
                }
            }
            else
            {
                result = RESULT_NOT_OK;
            }
        }
        else
        {
            result = RESULT_NOT_OK;
        }
    }
    else
    {
        result = RESULT_NOT_OK;
    }

    return result;
}
// end of DS18B20_GetTemperature



//**************************************************************************************************
// @Function      DS18B20_SetResolution()
//--------------------------------------------------------------------------------------------------
// @Description   Set resolution
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    resolution - pointer data resolution
//**************************************************************************************************
STD_RESULT DS18B20_SetResolution(uint8_t nCh, const uint8_t* ID,const uint8_t* resolution)
{
    STD_RESULT result = RESULT_NOT_OK;
    uint8_t scratchPad[DS18B20_SCRATCHPAD_SIZE];
    enONE_WIRE_PRESENCE status;
    if (RESULT_OK == ONE_WIRE_reset(nCh,&status))
    {
        if (ONE_WIRE_PRESENCE == status)
        {
            // Match ID
            DS18B20_MatchID(nCh,*ID);
            // Read TH and TL
            DS18B20_ReadScratchPad(nCh,scratchPad);
            ONE_WIRE_reset(nCh,&status);
            if (ONE_WIRE_PRESENCE == status)
            {
                // Match ID
                DS18B20_MatchID(nCh,*ID);
                // Set new resolution
                DS18B20_WriteScratchPad(nCh, scratchPad[DS18B20_SCRATCHPAD_TL_ADDR],scratchPad[DS18B20_SCRATCHPAD_TH_ADDR], *resolution);
                result = RESULT_OK;
            }
            else
            {
                result = RESULT_NOT_OK;
            }
        }
        else
        {
            result = RESULT_NOT_OK;
        }
    }
    else
    {
        result = RESULT_NOT_OK;
    }


    return result;
}
// end of DS18B20_SetResolution()

//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      DS18B20_MatchID()
//--------------------------------------------------------------------------------------------------
// @Description   Match ID. The user must reset before.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   Return 1 if ID matched, otherwise 0.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT DS18B20_MatchID(const uint8_t nCh, uint64_t ID)
{
    STD_RESULT result = RESULT_OK;
    uint8_t temp=0;

    // Send command MATCH ROM

    if (RESULT_OK == ONE_WIRE_writeByte(nCh,DS18B20_MATCH_ROM_CMD))
    {
        for(int i=0;i<64;i++)
        {
            temp = ID >> i;
            temp &= 0x01;
            if (RESULT_NOT_OK == ONE_WIRE_writeBit(nCh,temp))
            {
                result = RESULT_NOT_OK;
                break;
            }
        }
    }
    else
    {
        result = RESULT_NOT_OK;
    }

    return result;
}
//end of DS18B20_MatchID()



//**************************************************************************************************
// @Function      DS18B20_SkipID()
//--------------------------------------------------------------------------------------------------
// @Description   Skip ROM function.The user must reset before.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void DS18B20_SkipID(const uint8_t nCh)
{
    // Send command SKIP ROM
    ONE_WIRE_writeByte(nCh,DS18B20_SKIP_ROM_CMD);
}
//end of DS18B20_SkipID()



//**************************************************************************************************
// @Function      DS18B20_ReadScratchPad()
//--------------------------------------------------------------------------------------------------
// @Description   Read scratchpad
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    Pointer data to store 8 bytes
//**************************************************************************************************
static void DS18B20_ReadScratchPad(uint8_t nCh, uint8_t* data)
{
    // Send command READ SCRATCHPAD
    ONE_WIRE_writeByte(nCh, DS18B20_READ_SCRATCHPAD);
    for(int i=0;i<DS18B20_SCRATCHPAD_SIZE;i++)
    {
        uint8_t byteVal=0;
        ONE_WIRE_readByte(nCh,&byteVal);
        *data = byteVal;
        data++;
    }
}
// end of DS18B20_ReadScratchPad()



//**************************************************************************************************
// @Function      DS18B20_WriteScratchPad()
//--------------------------------------------------------------------------------------------------
// @Description   Read scratchpad
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    TL - threshold high temperature
//                TL - threshold low temperature
//**************************************************************************************************
static void DS18B20_WriteScratchPad(const uint8_t nCh, uint8_t TL,uint8_t TH, uint8_t resolution)
{
    // Send command WRITE SCRATCHPAD
    ONE_WIRE_writeByte(nCh,DS18B20_WRITE_SCRATCHPAD);
    // write TL
    ONE_WIRE_writeByte(nCh,TL);
    // write TH
    ONE_WIRE_writeByte(nCh,TH);
    // write config
    ONE_WIRE_writeByte(nCh,resolution);
}
// end of DS18B20_WriteScratchPad()



//**************************************************************************************************
// @Function      DS18B20_ConvertT()
//--------------------------------------------------------------------------------------------------
// @Description   Start conversion temperature.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT DS18B20_ConvertT(const uint8_t nCh)
{
    STD_RESULT result = RESULT_NOT_OK;
    // Send command Convert T
    if (RESULT_OK == ONE_WIRE_writeByte(nCh,DS18B20_CONVERT_T))
    {
        // Delay conversion
        DS18B20_Delay(DS18B20_CONVERSION_TIME_US);
        uint8_t bitVal = 0;
        ONE_WIRE_readBit(nCh,&bitVal);
        if (LOGIC_1 == bitVal)
        {
            result = RESULT_OK;
        }
        else
        {
            result = RESULT_NOT_OK;
        }
    }
    else
    {
        result = RESULT_NOT_OK;
    }

    return result;
}
//end of DS18B20_ConvertT()



//**************************************************************************************************
// @Function      DS18B20_GetTemFromScratchpad()
//--------------------------------------------------------------------------------------------------
// @Description   Start conversion temperature.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static float DS18B20_GetTemFromScratchpad( const uint8_t* scratchpad)
{
    uint16_t tempScratchPad = 0;
    float temperature = 0.0f;

    tempScratchPad = *scratchpad;
    scratchpad++;
    tempScratchPad |= ((uint16_t)*scratchpad) << 8;

    temperature += ((tempScratchPad & 0x01) == 0x01) ? 1.0f/16.0f : 0.0f;
    temperature += ((tempScratchPad & 0x02) == 0x02) ? 1.0f/8.0f : 0.0f;
    temperature += ((tempScratchPad & 0x04) == 0x04) ? 1.0f/4.0f : 0.0f;
    temperature += ((tempScratchPad & 0x08) == 0x08) ? 1.0f/2.0f : 0.0f;
    temperature += ((tempScratchPad & 0x10) == 0x10) ? 1.0f : 0.0f;
    temperature += ((tempScratchPad & 0x20) == 0x20) ? 2.0f : 0.0f;
    temperature += ((tempScratchPad & 0x40) == 0x40) ? 4.0f : 0.0f;
    temperature += ((tempScratchPad & 0x80) == 0x80) ? 8.0f : 0.0f;
    temperature += ((tempScratchPad & 0x100) == 0x100) ? 16.0f : 0.0f;
    temperature += ((tempScratchPad & 0x200) == 0x200) ? 32.0f : 0.0f;
    temperature += ((tempScratchPad & 0x400) == 0x400) ?
                   (((tempScratchPad & 0x800) == 0x800) ? -64.0f : 64.0f) : 0.0f;

    return temperature;
}
// end of DS18B20_GetTemFromScratchpad()



//**************************************************************************************************
// @Function      DS18B20_CalculateCRC()
//--------------------------------------------------------------------------------------------------
// @Description   Calculate CRC
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters   data - pointer data to calculate CRC
//               len - length data
//**************************************************************************************************
static uint8_t DS18B20_CalculateCRC(uint8_t* data, uint8_t len)
{
    uint8_t crc = 0;
    while(len !=0)
    {
        uint8_t i = 8;
        uint8_t b = *data;
        do {
            if ((b ^ crc) & 0x01) {
                crc = ((crc ^ 0x18) >> 1) | 0x80;
            } else {
                crc >>= 1;
            }
            b >>= 1;
        } while (--i);
        len--;
        data++;
    }
    return crc;
}
// end of DS18B20_CalculateCRC()


//****************************************** end of file *******************************************

