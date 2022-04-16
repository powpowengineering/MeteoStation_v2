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
#define ONE_WIRE_TIME_RESET_PULSE_US                      (480U)
// time for PRESENCE PULSE in us
#define ONE_WIRE_TIME_PRESENCE_PULSE_US                   (100U)
// time for detect time slot in us
#define ONE_WIRE_TIME_DETECT_US                           (480U+60U+240U)
// master sample time
#define ONE_WIRE_TIME_MASTER_SAMPLE_TIME                  (13U)
// master write time
#define ONE_WIRE_TIME_WRITE_SAMPLE                        (30U)
// rest time slot
#define ONE_WIRE_TIME_REST_TIME_SLOT                      (2U)

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
static void ONE_WIRE_DQLow(void);
// DQ configuration as input
static void ONE_WIRE_DQInput(void);
// Get value on DQ input
static uint8_t ONE_WIRE_DQGetValue(void);


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
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
enONE_WIRE_PRESENCE ONE_WIRE_reset(void)
{
    enONE_WIRE_PRESENCE result = ONE_WIRE_NOT_PRESENCE;

    //pull DQ line low
    ONE_WIRE_DQLow();
    // leave it low for 480us
    ONE_WIRE_Delay(ONE_WIRE_TIME_RESET_PULSE_US);
    // allow line to return high
    ONE_WIRE_DQInput();
    // wait for presence
    ONE_WIRE_Delay(ONE_WIRE_TIME_PRESENCE_PULSE_US);
    // get presence signal
    if (ONE_WIRE_DQGetValue() == 0)
    {
        result = ONE_WIRE_NOT_PRESENCE;
    }
    else
    {
        result = ONE_WIRE_PRESENCE;
    }
    // wait for end of timeslot
    ONE_WIRE_Delay(ONE_WIRE_TIME_DETECT_US-ONE_WIRE_TIME_RESET_PULSE_US-ONE_WIRE_TIME_PRESENCE_PULSE_US);

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
// @ReturnValue   Bit value: 0,1
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
 uint8_t ONE_WIRE_readBit(void)
{
    // pull DQ low to start timeslot
    ONE_WIRE_DQLow();
    // wait before leave pin
    //ONE_WIRE_Delay(1);
    // allow line to return high
    ONE_WIRE_DQInput();
    // wait master sample time
    ONE_WIRE_Delay(ONE_WIRE_TIME_MASTER_SAMPLE_TIME-1);
    // read DQ
    return ONE_WIRE_DQGetValue();
}
//end of ONE_WIRE_readBit()



//**************************************************************************************************
// @Function      ONE_WIRE_writeBit()
//--------------------------------------------------------------------------------------------------
// @Description   Write bit
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   Bit value: 0,1
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void ONE_WIRE_writeBit(uint8_t bitVal)
{
    // pull DQ low to start timeslot
    ONE_WIRE_DQLow();

    if (1 == bitVal)
    {
        // allow line to return high
        ONE_WIRE_DQInput();
    }
    // wait master write time
    ONE_WIRE_Delay(ONE_WIRE_TIME_WRITE_SAMPLE);
    // allow line to return high
    ONE_WIRE_DQInput();
}
//end of ONE_WIRE_writeBit()



//**************************************************************************************************
// @Function      ONE_WIRE_readByte()
//--------------------------------------------------------------------------------------------------
// @Description   Read byte
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   Return byte read in one wire
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
uint8_t ONE_WIRE_readByte(void)
{
    uint8_t result = 0;

    for (int i=0;i<8;i++)
    {
        if (ONE_WIRE_readBit() == 1)
        {
            result |= 0x01 << i;
        }
        // wait for rest of timeslot
        ONE_WIRE_Delay(ONE_WIRE_TIME_REST_TIME_SLOT);
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
void ONE_WIRE_writeByte(uint8_t byteVal)
{
    uint8_t temp = 0;
    for(int i=0;i<8;i++)
    {
        temp = byteVal >> i;
        temp &= 0x01;
        ONE_WIRE_writeBit(temp);
        // wait for rest of timeslot
        ONE_WIRE_Delay(ONE_WIRE_TIME_REST_TIME_SLOT);
    }
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
    GPIO_InitStruct.GPIO_Pin  = (1<<ONE_WIRE_PIN);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(ONE_WIRE_GPIO_PORT, &GPIO_InitStruct);

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
static void ONE_WIRE_DQLow(void)
{
    // Config DQ as OUT
    ONE_WIRE_GPIO_PORT->MODER |= GPIO_MODER_MODER0_0 << (ONE_WIRE_PIN*2);
    // set DQ low
    ONE_WIRE_GPIO_PORT->BSRRH = GPIO_Pin_10;
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
static void ONE_WIRE_DQInput(void)
{
    // Config DQ as INPUT
    ONE_WIRE_GPIO_PORT->MODER &= ~(GPIO_MODER_MODER0 << (ONE_WIRE_PIN*2));
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
static uint8_t ONE_WIRE_DQGetValue(void)
{
    // Get value
    return GPIO_ReadInputDataBit(ONE_WIRE_GPIO_PORT, ONE_WIRE_PIN);
}// end of ONE_WIRE_DQGetValue()

//****************************************** end of file *******************************************

