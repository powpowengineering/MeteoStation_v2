//**************************************************************************************************
// @Module        AM2305
// @Filename      am2305.c
//--------------------------------------------------------------------------------------------------
// @Platform      STM32
//--------------------------------------------------------------------------------------------------
// @Compatible    STM32L151
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the AM2305 functionality.
//                
//
//                Abbreviations:
//                  None.
//                  
//
//                Global (public) functions:
//                  AM2305_Init();
//                  AM2305_GetHumidityTemperature();
//
//                Local (private) functions:
//                  AM2305_DQLow();
//                  AM2305_DQInput();
//                  AM2305_DQGetValue();
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
#include "am2305_drv.h"

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
//**************************************************************************************************

// None.


//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

// Timings
// Host the start signal down time
#define AM2305_TIME_T_BE_US                (1000U)
// Host the start signal down time
#define AM2305_TIME_T_GO_US                (30U)
// Response to low time
#define AM2305_TIME_T_REL_US               (80U)
// In response to high time
#define AM2305_TIME_T_REH_US               (80U)
// Signal "0", "1" low time
#define AM2305_TIME_T_LOW_US               (50U)
// Signal "0", "1" low time max
#define AM2305_TIME_T_LOW_MAX_US           (55U)
// Signal "0" high time
#define AM2305_TIME_T_H0_US                (26U)
// Signal "1" high time
#define AM2305_TIME_T_H1_US                (70U)
// Signal "1" high time min
#define AM2305_TIME_T_H1_MIN_US            (68U)
// Signal "1" high time max
#define AM2305_TIME_T_H1_MAX_US            (75U)
// Sensor to release the bus time
#define AM2305_TIME_T_EN_US                (50U)
// quantity data bits
#define AM2305_QTY_DATA_BITS               (40U)
// Strobe duration
#define AM2305_STROBE_DURATION_US          (10U)
// quantity zeros strobes in data stream
#define AM2305_QTY_ZERO_BITS               (AM2305_TIME_T_LOW_US/AM2305_STROBE_DURATION_US)
// quantity one strobes in data. BIT_RESET_STATE
#define AM2305_QTY_ONE_STROBE_BIT_RESET_STATE   (AM2305_TIME_T_H0_US/AM2305_STROBE_DURATION_US)
// quantity MIN one strobes in data. BIT_SET_STATE
#define AM2305_QTY_MIN_ONE_STROBE_BIT_SET_STATE     (AM2305_TIME_T_H1_MIN_US/AM2305_STROBE_DURATION_US)
// quantity MAX one strobes in data. BIT_SET_STATE
#define AM2305_QTY_MAX_ONE_STROBE_BIT_SET_STATE     (AM2305_TIME_T_H1_MAX_US/AM2305_STROBE_DURATION_US)
// quantity MAX zero strobes in data.
#define AM2305_QTY_MAX_ZERO_STROBES_BIT         (AM2305_TIME_T_LOW_MAX_US/AM2305_STROBE_DURATION_US)

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// Pointer of delay function.
void (*pAM2305_Delay)(uint32_t us);


//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// Set low level on DQ pin
static void AM2305_DQLow(void);
// DQ configuration as input
static void AM2305_DQInput(void);
// Get DQ value
static BitAction AM2305_DQGetValue(void);


//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************


//**************************************************************************************************
// @Function      AM2305_Init()
//--------------------------------------------------------------------------------------------------
// @Description   Init GPIO, delay function.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void AM2305_Init(void)
{
    // gpio init
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin  = (1<<AM2305_PIN);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(AM2305_GPIO_PORT, &GPIO_InitStruct);
    // High DQ line
    AM2305_DQInput();
    // Init AM2305 delay function.
    pAM2305_Delay = AM2305_Delay;
}// end of AM2305_Init();


//**************************************************************************************************
// @Function      AM2305_GetHumidityTemperature()
//--------------------------------------------------------------------------------------------------
// @Description   Read humidity adn temperature.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   RESULT_OK - read OK
//                RESULT_NOT_OK - read Not Ok
//--------------------------------------------------------------------------------------------------
// @Parameters    humidity - get the humidity
//                temperature - get the temperature
//**************************************************************************************************
STD_RESULT AM2305_GetHumidityTemperature(float *const humidity,float *const temperature)
{
    STD_RESULT result = RESULT_OK;
    uint8_t data[AM2305_QTY_DATA_BITS/8];
    uint8_t cntBit=0;
    uint8_t cntByte=0;

    // Getting response from sensor
    AM2305_DQLow();
    // Host the start signal down time
    AM2305_Delay(AM2305_TIME_T_BE_US);
    // High DQ
    AM2305_DQInput();
    // Bus master has released time
    AM2305_Delay(AM2305_TIME_T_GO_US);
    // Master sample time
    AM2305_Delay(AM2305_TIME_T_REL_US/2);
    // Get DQ value
    if (Bit_RESET == AM2305_DQGetValue())
    {
        // continue wait response to low time
        AM2305_Delay(AM2305_TIME_T_REL_US/2);
        // In response to high time
        AM2305_Delay(AM2305_TIME_T_REH_US);

        for (int i=0,cntZero=0,cntOne=0; i < AM2305_QTY_DATA_BITS; )
        {
            AM2305_Delay(AM2305_STROBE_DURATION_US);
            if (Bit_RESET == AM2305_DQGetValue())
            {
                cntZero++;
                if ((0 < cntOne) && (AM2305_QTY_ONE_STROBE_BIT_RESET_STATE >= cntOne))
                {
                    if (cntBit == 8U)
                    {
                        cntByte++;
                        cntBit=0;
                        data[cntByte] = 0;
                    }
                    cntBit++;
                    cntOne=0;
                    i++;
                }
                else if ((AM2305_QTY_MIN_ONE_STROBE_BIT_SET_STATE <= cntOne) && \
                         (AM2305_QTY_MAX_ONE_STROBE_BIT_SET_STATE >= cntOne))
                {
                    if (cntBit == 8U)
                    {
                        cntByte++;
                        cntBit=0;
                        data[cntByte] = 0;
                    }
                    data[cntByte] |= 1 << cntBit;
                    cntBit++;
                    cntOne=0;
                    i++;
                }
                else if (AM2305_QTY_MAX_ZERO_STROBES_BIT < cntZero)
                {
                    result = RESULT_NOT_OK;
                    break;
                }
            }
            else
            {
                cntOne++;
                cntZero=0;
            }
        }
    }
    else
    {
        result = RESULT_NOT_OK;
    }

    // calculate temperature and humidity
    if (RESULT_OK == result)
    {
        // calculate party byte
        uint8_t sum=0;
        for(uint8_t i = 0; i < (AM2305_QTY_DATA_BITS/8)-1; i++)
        {
            sum += data[i];
        }
        if ( data[(AM2305_QTY_DATA_BITS/8)-1] == sum)
        {
            uint16_t temp = (((uint16_t)data[0])<<8) | data[1];
            *humidity = (float)(temp)/10;

            temp = (((uint16_t)data[3])<<8) | data[4];
            // if temperature < 0
            if ((temp & (1<<15)) == 1<<15)
            {
                *temperature = -(float)(temp)/10;
            }
            else
            {
                *temperature = (float)(temp)/10;
            }
        }
        else
        {
            result = RESULT_NOT_OK;
        }
    }

    return result;

}// end of AM2305_GetHumidityTemperature()

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
static void AM2305_DQLow(void)
{
    // Config DQ as OUT
    AM2305_GPIO_PORT->MODER |= GPIO_MODER_MODER0_0 << (AM2305_PIN*2);
    // set DQ low
    AM2305_GPIO_PORT->BSRRH = 1U<<AM2305_PIN;
}// end of AM2305_DQLow()



//**************************************************************************************************
// @Function      AM2305_DQInput()
//--------------------------------------------------------------------------------------------------
// @Description   DQ configuration as input
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void AM2305_DQInput(void)
{
    // Config DQ as INPUT
    AM2305_GPIO_PORT->MODER &= ~(GPIO_MODER_MODER0 << (AM2305_PIN * 2));
}// end of AM2305_DQInput()


//**************************************************************************************************
// @Function      AM2305_DQGetValue()
//--------------------------------------------------------------------------------------------------
// @Description   Get value on DQ input
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static BitAction AM2305_DQGetValue(void)
{
    uint8_t result = 0;

    // Get value
    if ((AM2305_GPIO_PORT->IDR & (1<<AM2305_PIN)) != (uint32_t)Bit_RESET)
    {
        result = (uint8_t)Bit_SET;
    }
    else
    {
        result = (uint8_t)Bit_RESET;
    }
    return result;
}// end of AM2305_DQGetValue()


//****************************************** end of file *******************************************
