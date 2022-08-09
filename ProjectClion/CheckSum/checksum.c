//**************************************************************************************************
// @Module        CH_SUM
// @Filename      checksum.c
//--------------------------------------------------------------------------------------------------
// @Platform      stm32
//--------------------------------------------------------------------------------------------------
// @Compatible    stm32
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the CH_SUM functionality.
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
#include "checksum.h"

#include "Init.h"

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

// None.

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// None.


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
// @Function      CH_SUM_CalculateCRC8
//--------------------------------------------------------------------------------------------------
// @Description   Calculate CRC8.
//--------------------------------------------------------------------------------------------------
// @Notes         Poly  : 0x31    x^8 + x^5 + x^4 + 1
//--------------------------------------------------------------------------------------------------
// @ReturnValue   crc8
//--------------------------------------------------------------------------------------------------
// @Parameters    data - pointer to data to calculate crc8
//                len - length data
//**************************************************************************************************
uint8_t CH_SUM_CalculateCRC8(const uint8_t* data, uint32_t len)
{
    uint8_t crc = 0xFF;
    uint32_t i;

    if (0U != len)
    {
        while (len--)
        {
            crc ^= *data++;

            for (i = 0; i < 8; i++)
                crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
        }
    }


    return crc;
}// end of CH_SUM_CalculateCRC8



//**************************************************************************************************
// @Function      CH_SUM_CalculateCRC8WithBegin
//--------------------------------------------------------------------------------------------------
// @Description   Calculate CRC8 with begin value
//--------------------------------------------------------------------------------------------------
// @Notes         Poly  : 0x31    x^8 + x^5 + x^4 + 1
//--------------------------------------------------------------------------------------------------
// @ReturnValue   crc8
//--------------------------------------------------------------------------------------------------
// @Parameters    data - pointer to data to calculate crc8
//                len - length data
//**************************************************************************************************
uint8_t CH_SUM_CalculateCRC8WithBegin(const uint8_t* data,
                                      uint32_t len,
                                      uint8_t nCrcBegin)
{
    uint8_t crc = nCrcBegin;
    uint32_t i;

    if (0U != len)
    {
        while (len--)
        {
            crc ^= *data++;

            for (i = 0; i < 8; i++)
                crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
        }
    }

    return crc;
}// end of CH_SUM_CalculateCRC8WithBegin



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************


//****************************************** end of file *******************************************