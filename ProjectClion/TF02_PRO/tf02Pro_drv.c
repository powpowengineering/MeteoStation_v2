//**************************************************************************************************
// @Module        TF02_PRO
// @Filename      tF02Pro_drv.c
//--------------------------------------------------------------------------------------------------
// @Platform      stm32
//--------------------------------------------------------------------------------------------------
// @Compatible    stm32
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
#include "tf02Pro_drv.h"

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

typedef enum TF02_PRO_FRAME_enum
{
    TF02_PRO_FRAME_FULL=0,
    TF02_PRO_FRAME_NOT_FULL,
    TF02_PRO_FRAME_CHECKSUM_ER
}TF02_PRO_FRAME;


//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

#define TF02_PRO_UART_NUMBER        USART3

#define LIDAR_HEADER_BYTE           (0x59U)

#define LIDAR_FRAME_SIZE            (9U)

#define SIZE_BUF_LIDAR_DATA         (64U)



#define TF02_PRO_UNRELIABLE_LEVEL   (60U)

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

//static uint8_t TF02_PRO_lidarData[SIZE_BUF_LIDAR_DATA];

static uint32_t index=0;


//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

static TF02_PRO_FRAME TF02_PRO_ParsingDataLidar(uint8_t ch,
                                                TF02_PRO_MEASURED_DATA *const data);



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      TF02_PRO_GetMeasuredData()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
STD_RESULT TF02_PRO_GetMeasuredData(TF02_PRO_MEASURED_DATA *const data)
{
    STD_RESULT result = RESULT_OK;

    uint32_t nNumberMes = 0;

    uint8_t ch = 0;

    TF02_PRO_MEASURED_DATA dataSum = {0, 0 ,0};

    while (TF02_PRO_QTY_MES > nNumberMes)
    {
        if (SET == USART_GetFlagStatus(TF02_PRO_UART_NUMBER, USART_FLAG_RXNE))
        {
            ch = USART_ReceiveData(TF02_PRO_UART_NUMBER);

            if (TF02_PRO_FRAME_FULL == TF02_PRO_ParsingDataLidar(ch, data))
            {
                if (TF02_PRO_UNRELIABLE_LEVEL < data->nStrength)
                {
                    dataSum.nDistance += data->nDistance;
                    dataSum.nStrength += data->nStrength;
                    dataSum.nTemperature += data->nTemperature;
                    nNumberMes++;
                }
            }
        }
    }

    data->nDistance = dataSum.nDistance / TF02_PRO_QTY_MES;
    data->nStrength = dataSum.nStrength / TF02_PRO_QTY_MES;
    data->nTemperature = dataSum.nTemperature / TF02_PRO_QTY_MES;

    return result;
} // end of TF02_PRO_GetMeasuredData()


//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      TF02_PRO_ParsingDataLidar()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static TF02_PRO_FRAME TF02_PRO_ParsingDataLidar(uint8_t ch,
                                                TF02_PRO_MEASURED_DATA *const data)
{
    TF02_PRO_FRAME result = TF02_PRO_FRAME_NOT_FULL;

    static LIDAR_FRAME LIDAR_Frame = LIDAR_HEADER_LOW;

    static uint8_t nChecksum = 0;

    switch(LIDAR_Frame)
    {
        case LIDAR_HEADER_LOW:
            if (LIDAR_HEADER_BYTE == ch)
            {
                LIDAR_Frame = LIDAR_HEADER_HIGH;
                result = TF02_PRO_FRAME_NOT_FULL;
                nChecksum = ch;
            }
            break;
        case LIDAR_HEADER_HIGH:
            if (LIDAR_HEADER_BYTE == ch)
            {
                LIDAR_Frame = LIDAR_DISTANCE_LOW;
                nChecksum += ch;
            }
            else
            {
                LIDAR_Frame = LIDAR_HEADER_LOW;
            }
            break;
        case LIDAR_DISTANCE_LOW:
            data->nDistance = ch;
            LIDAR_Frame = LIDAR_DISTANCE_HIGH;
            nChecksum += ch;
            break;
        case LIDAR_DISTANCE_HIGH:
            data->nDistance |= (uint16_t)ch << 8;
            LIDAR_Frame = LIDAR_STRENGTH_LOW;
            nChecksum += ch;
            break;
        case LIDAR_STRENGTH_LOW:
            data->nStrength = ch;
            LIDAR_Frame = LIDAR_STRENGTH_HIGH;
            nChecksum += ch;
            break;
        case LIDAR_STRENGTH_HIGH:
            data->nStrength |= (uint16_t)ch << 8;
            LIDAR_Frame = LIDAR_TEMPERATURE_LOW;
            nChecksum += ch;
            break;
        case LIDAR_TEMPERATURE_LOW:
            data->nTemperature = ch;
            LIDAR_Frame = LIDAR_TEMPERATURE_HIGH;
            nChecksum += ch;
            break;
        case LIDAR_TEMPERATURE_HIGH:
            data->nTemperature |= (uint16_t)ch << 8;
            LIDAR_Frame = LIDAR_CHECKSUM;
            nChecksum += ch;
            break;
        case LIDAR_CHECKSUM:
            LIDAR_Frame = LIDAR_HEADER_LOW;
            if (nChecksum == ch)
            {
                result = TF02_PRO_FRAME_FULL;
            }
            else
            {
                result = TF02_PRO_FRAME_CHECKSUM_ER;
            }
            break;
        default:
            LIDAR_Frame = LIDAR_HEADER_LOW;
    }

    return result;
} // end of TF02_PRO_ParsingDataLidar()


//****************************************** end of file *******************************************