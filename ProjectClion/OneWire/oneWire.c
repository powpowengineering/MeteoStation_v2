//**************************************************************************************************
// @Module        ONE_WIRE
// @Filename      oneWire.c
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
//                  ONE_WIRE_init()
//                  ONE_WIRE_reset()
//                  ONE_WIRE_readBit()
//                  ONE_WIRE_writeBit()
//                  ONE_WIRE_readByte()
//                  ONE_WIRE_writeByte()
//
//                Local (private) functions:
//                  ONE_WIRE_GpioInit()
//                  ONE_WIRE_DQLow()
//                  ONE_WIRE_DQInput()
//                  ONE_WIRE_DQGetValue()
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

// time of RESET PULSE in us
#define ONE_WIRE_RESET_PULSE_TIME_US                      (600U)
// time for PRESENCE PULSE in us
#define ONE_WIRE_PRESENCE_PULSE_TIME_US                   (100U)
// time for detect time slot in us
#define ONE_WIRE_DETECT_TIME_US                           (ONE_WIRE_RESET_PULSE_TIME_US+60U+240U+180U)
// master sample time
#define ONE_WIRE_MASTER_SAMPLE_TIME_US                    (13U)
// read time slot
#define ONE_WIRE_READ_TIME_SLOT_US                        (60U)
// master write time
#define ONE_WIRE_WRITE_SAMPLE_TIME_US                     (60U)
// rest time slot
#define ONE_WIRE_REST_TIME_SLOT_US                        (2U)
// channel number
#define ONE_WIRE_CH0                                      (0U)
#define ONE_WIRE_CH1                                      (1U)

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// None.


//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// Init gpio
static void ONE_WIRE_GpioInit(void);
// Set low level on DQ pin
static STD_RESULT ONE_WIRE_DQLow(uint8_t nCh);
// DQ configuration as input
static STD_RESULT ONE_WIRE_DQInput(uint8_t nCh);
// Get value on DQ input
static STD_RESULT ONE_WIRE_DQGetValue(uint8_t nCh, uint8_t *const bitStatus);


//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      ONE_WIRE_init()
//--------------------------------------------------------------------------------------------------
// @Description   Init one wire interface
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void ONE_WIRE_init(void)
{
    // Gpio init
    ONE_WIRE_GpioInit();

}// end of ONE_WIRE_init()



//**************************************************************************************************
// @Function      ONE_WIRE_reset()
//--------------------------------------------------------------------------------------------------
// @Description   Detect slaves
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK - function completed successfully
//                RESULT_NOT_OK - function didn't complete successfully
//--------------------------------------------------------------------------------------------------
// @Parameters    nCh - number of channel
//                status - one wire presence pulse or not
//**************************************************************************************************
STD_RESULT ONE_WIRE_reset(uint8_t nCh, enONE_WIRE_PRESENCE *const status)
{
    STD_RESULT result = RESULT_NOT_OK;

    //pull DQ line low
    if (RESULT_OK == ONE_WIRE_DQLow(nCh))
    {
        // leave it low for RESET_PULSE
        ONE_WIRE_Delay(ONE_WIRE_RESET_PULSE_TIME_US);
        // allow line to return high
        if (RESULT_OK == ONE_WIRE_DQInput(nCh))
        {
            // wait for presence
            ONE_WIRE_Delay(ONE_WIRE_PRESENCE_PULSE_TIME_US);
            // get presence signal
            uint8_t bitStatus = 0;
            if (RESULT_OK == ONE_WIRE_DQGetValue(nCh,&bitStatus))
            {
                if ( 0 == bitStatus)
                {
                    *status = ONE_WIRE_PRESENCE;
                }
                else
                {
                    *status = ONE_WIRE_NOT_PRESENCE;
                }
                // wait for end of timeslot
                ONE_WIRE_Delay(ONE_WIRE_DETECT_TIME_US - ONE_WIRE_RESET_PULSE_TIME_US - ONE_WIRE_PRESENCE_PULSE_TIME_US);
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
// end of ONE_WIRE_reset()



//**************************************************************************************************
// @Function      ONE_WIRE_readBit()
//--------------------------------------------------------------------------------------------------
// @Description   Read bit
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK - function completed successfully
//                RESULT_NOT_OK - function didn't complete successfully
//--------------------------------------------------------------------------------------------------
// @Parameters    nCh - number of channel
//                bitVal - read value
//**************************************************************************************************
STD_RESULT ONE_WIRE_readBit(uint8_t nCh, uint8_t *const bitVal)
{
    STD_RESULT result = RESULT_NOT_OK;

    // pull DQ low to start timeslot
    if (RESULT_OK == ONE_WIRE_DQLow(nCh))
    {
        // wait before leave pin
        //ONE_WIRE_Delay(1);
        // allow line to return high
        if (RESULT_OK == ONE_WIRE_DQInput(nCh))
        {
            // wait master sample time
            ONE_WIRE_Delay(ONE_WIRE_MASTER_SAMPLE_TIME_US - 1);
            // read DQ
            if (RESULT_OK == ONE_WIRE_DQGetValue(nCh, bitVal))
            {
                result = RESULT_OK;
            }
            else
            {
                result = RESULT_NOT_OK;
            }
            // wait end of read time slot
            ONE_WIRE_Delay(ONE_WIRE_READ_TIME_SLOT_US - ONE_WIRE_MASTER_SAMPLE_TIME_US - 1);
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
//end of ONE_WIRE_readBit()



//**************************************************************************************************
// @Function      ONE_WIRE_writeBit()
//--------------------------------------------------------------------------------------------------
// @Description   Write bit
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK - function completed successfully
//                RESULT_NOT_OK - function didn't complete successfully
//--------------------------------------------------------------------------------------------------
// @Parameters    nCh - number of channel
//                bitVal - value being written
//**************************************************************************************************
STD_RESULT ONE_WIRE_writeBit(uint8_t nCh, uint8_t bitVal)
{
    STD_RESULT result = RESULT_NOT_OK;

    // pull DQ low to start timeslot
    if (RESULT_OK == ONE_WIRE_DQLow(nCh))
    {
        if (1 == bitVal)
        {
            // allow line to return high
            if (RESULT_OK == ONE_WIRE_DQInput(nCh))
            {
                result = RESULT_OK;
            }
            else
            {
                result = RESULT_NOT_OK;
            }
        }
        // wait master write time
        ONE_WIRE_Delay(ONE_WIRE_WRITE_SAMPLE_TIME_US);
        // allow line to return high
        if (RESULT_OK == ONE_WIRE_DQInput(nCh))
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
//end of ONE_WIRE_writeBit()



//**************************************************************************************************
// @Function      ONE_WIRE_readByte()
//--------------------------------------------------------------------------------------------------
// @Description   Read byte
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK - function completed successfully
//                RESULT_NOT_OK - function didn't complete successfully
//--------------------------------------------------------------------------------------------------
// @Parameters    nCh - number of channel
//                bitVal - value being read
//**************************************************************************************************
STD_RESULT ONE_WIRE_readByte(uint8_t nCh, uint8_t *const byteVal)
{
    STD_RESULT result = RESULT_OK;

    for (int i=0;i<8;i++)
    {
        uint8_t bitVal=0;
        if (RESULT_OK == ONE_WIRE_readBit(nCh,&bitVal))
        {
            *byteVal |= 0x01 << i;
        }
        else
        {
            result = RESULT_NOT_OK;
            break;
        }
        // wait for rest of timeslot
        ONE_WIRE_Delay(ONE_WIRE_REST_TIME_SLOT_US);
    }

    return result;
}
//end of ONE_WIRE_readByte()


//**************************************************************************************************
// @Function      ONE_WIRE_writeByte()
//--------------------------------------------------------------------------------------------------
// @Description   Write byte
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
STD_RESULT ONE_WIRE_writeByte(uint8_t nCh, uint8_t byteVal)
{
    STD_RESULT result = RESULT_OK;

    uint8_t temp = 0;

    if (ONE_WIRE_CH0 == nCh)
    {
        for(int i=0;i<8;i++)
        {
            temp = byteVal >> i;
            temp &= 0x01;
            if (RESULT_OK == ONE_WIRE_writeBit(nCh,temp))
            {
                // wait for rest of timeslot
                ONE_WIRE_Delay(ONE_WIRE_REST_TIME_SLOT_US);
            }
            else
            {
                result = RESULT_NOT_OK;
                break;
            }
        }
    }
    else if (ONE_WIRE_CH1 == nCh)
    {
        for(int i=0;i<8;i++)
        {
            temp = byteVal >> i;
            temp &= 0x01;
            if (RESULT_OK == ONE_WIRE_writeBit(nCh,temp))
            {
                // wait for rest of timeslot
                ONE_WIRE_Delay(ONE_WIRE_REST_TIME_SLOT_US);
            }
            else
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
//end of ONE_WIRE_writeByte()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      ONE_WIRE_GpioInit()
//--------------------------------------------------------------------------------------------------
// @Description   Init gpio for one wire
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void ONE_WIRE_GpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin  = (1<<ONE_WIRE_PIN_CH0);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(ONE_WIRE_GPIO_PORT_CH0, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin  = (1<<ONE_WIRE_PIN_CH1);
    GPIO_Init(ONE_WIRE_GPIO_PORT_CH1, &GPIO_InitStruct);

}// end of ONE_WIRE_GpioInit()



//**************************************************************************************************
// @Function      ONE_WIRE_DQLow()
//--------------------------------------------------------------------------------------------------
// @Description   Set low level on DQ pin
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT ONE_WIRE_DQLow(uint8_t nCh)
{
    STD_RESULT result = RESULT_NOT_OK;

    if (ONE_WIRE_CH0 == nCh)
    {
        // Config DQ as OUT
        ONE_WIRE_GPIO_PORT_CH0->MODER |= GPIO_MODER_MODER0_0 << (ONE_WIRE_PIN_CH0*2);
        // set DQ low
        ONE_WIRE_GPIO_PORT_CH0->BSRRH = GPIO_Pin_10;
        result = RESULT_OK;
    }
    else if (ONE_WIRE_CH1 == nCh)
    {
        // Config DQ as OUT
        ONE_WIRE_GPIO_PORT_CH1->MODER |= GPIO_MODER_MODER0_0 << (ONE_WIRE_PIN_CH1*2);
        // set DQ low
        ONE_WIRE_GPIO_PORT_CH1->BSRRH = GPIO_Pin_10;
        result = RESULT_OK;
    }

    return result;
}// end of ONE_WIRE_DQLow()



//**************************************************************************************************
// @Function      ONE_WIRE_DQInput()
//--------------------------------------------------------------------------------------------------
// @Description   DQ configuration as input
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT ONE_WIRE_DQInput(uint8_t nCh)
{
    STD_RESULT result = RESULT_NOT_OK;

    if (ONE_WIRE_CH0 == nCh)
    {
        // Config DQ as INPUT
        ONE_WIRE_GPIO_PORT_CH0->MODER &= ~(GPIO_MODER_MODER0 << (ONE_WIRE_PIN_CH0 * 2));
        result = RESULT_OK;
    }
    else if (ONE_WIRE_CH1 == nCh)
    {
        // Config DQ as INPUT
        ONE_WIRE_GPIO_PORT_CH1->MODER &= ~(GPIO_MODER_MODER0 << (ONE_WIRE_PIN_CH1 * 2));
        result = RESULT_OK;
    }

    return result;
}// end of ONE_WIRE_DQInput()



//**************************************************************************************************
// @Function      ONE_WIRE_DQGetValue()
//--------------------------------------------------------------------------------------------------
// @Description   Get value on DQ input
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT ONE_WIRE_DQGetValue(uint8_t nCh, uint8_t *const bitStatus)
{
    STD_RESULT result = RESULT_NOT_OK;

    if (ONE_WIRE_CH0 == nCh)
    {
        // Get value
        if ((ONE_WIRE_GPIO_PORT_CH0->IDR & (1<<ONE_WIRE_PIN_CH0)) != (uint32_t)Bit_RESET)
        {
            *bitStatus = (uint8_t)Bit_SET;
        }
        else
        {
            *bitStatus = (uint8_t)Bit_RESET;
        }
        result = RESULT_OK;
    }
    else if (ONE_WIRE_CH1 == nCh)
    {
        // Get value
        if ((ONE_WIRE_GPIO_PORT_CH1->IDR & (1<<ONE_WIRE_PIN_CH1)) != (uint32_t)Bit_RESET)
        {
            *bitStatus = (uint8_t)Bit_SET;
        }
        else
        {
            *bitStatus = (uint8_t)Bit_RESET;
        }
        result = RESULT_OK;
    }

    return result;
}// end of ONE_WIRE_DQGetValue()

//****************************************** end of file *******************************************