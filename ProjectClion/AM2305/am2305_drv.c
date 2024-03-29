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

// Get HAL TIM LL
#include "stm32l4xx_ll_tim.h"



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

typedef enum AM2305_SIGNAL_STATUS_enum
{
    AM2305_SIGNAL_STATUS_START=0,
    AM2305_SIGNAL_STATUS_GO,
    AM2305_SIGNAL_STATUS_RESPONSE_LOW,
    AM2305_SIGNAL_STATUS_RESPONSE_HIGH,
    AM2305_SIGNAL_STATUS_SIGNAL_0_1_LOW,
    AM2305_SIGNAL_STATUS_SIGNAL_0_HIGH,
    AM2305_SIGNAL_STATUS_SIGNAL_1_HIGH,
    AM2305_SIGNAL_STATUS_END
}AM2305_SIGNAL_STATUS;


//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

// Timings
// Host the start signal down time
#define AM2305_TIME_T_BE_US                (1000U)
// Host the start signal down time
#define AM2305_TIME_T_GO_US                (30U)
// Response to low time min
#define AM2305_TIME_T_REL_MIN_US           (75U)
// Response to low time
#define AM2305_TIME_T_REL_US               (80U)
// Response to low time max
#define AM2305_TIME_T_REL_MAX_US           (85U)
// In response to high time min
#define AM2305_TIME_T_REH_MIN_US           (75U)
// In response to high time
#define AM2305_TIME_T_REH_US               (80U)
// In response to high time max
#define AM2305_TIME_T_REH_MAX_US           (85U)
// Signal "0", "1" low time
#define AM2305_TIME_T_LOW_US               (50U)
// Signal "0", "1" low time max
#define AM2305_TIME_T_LOW_MAX_US           (70U)
// Signal "0", "1" low time min
#define AM2305_TIME_T_LOW_MIN_US           (30U)
// Signal "0" high time
#define AM2305_TIME_T_H0_US                (26U)
// Signal "0" high time MIN
#define AM2305_TIME_T_H0_MIN_US            (22U)
// Signal "0" high time MAX
#define AM2305_TIME_T_H0_MAX_US            (30U)
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
// Quantity measurements
#define AM2305_QTY_MEAS             (100U)
// General time measure
#define AM2305_TIME_MEAS_US         (6000U)


//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// Pointer of delay function.
void (*pAM2305_Delay)(uint32_t us);

// Tim handler
static TIM_HandleTypeDef    AM2305_TimDelayHandle;



//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// Set low level on DQ pin
static void AM2305_DQLow(void);

// DQ configuration as input
static void AM2305_DQInput(void);

// Get DQ value
static GPIO_PinState AM2305_DQGetValue(void);

// Delay function
static void AM2305_Delay(uint32_t microseconds);

// capture time.
static STD_RESULT AM2305_CaptureTime(uint32_t *const microseconds);



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
    __HAL_RCC_TIM15_CLK_ENABLE();

    // Gpio init
    GPIO_InitTypeDef GPIO_InitStruct;
    // Config DQ as OUT open drain
    GPIO_InitStruct.Pin  = AM2305_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = AM2305_GPIO_AF;
    HAL_GPIO_Init(AM2305_GPIO_PORT, &GPIO_InitStruct);

    // High DQ line
    AM2305_DQInput();

    // Init AM2305_TIMER
    AM2305_TimDelayHandle.Instance = AM2305_TIMER;

    AM2305_TimDelayHandle.Init.Period            = AM2305_TIMER_PERIOD;
    AM2305_TimDelayHandle.Init.Prescaler         = AM2305_TIMER_PSC;
    AM2305_TimDelayHandle.Init.ClockDivision     = 0;
    AM2305_TimDelayHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    AM2305_TimDelayHandle.Init.RepetitionCounter = 0;
    HAL_TIM_Base_Init(&AM2305_TimDelayHandle);

    AM2305_TimDelayHandle.Channel = HAL_TIM_ACTIVE_CHANNEL_1;

    HAL_TIM_IC_Init(&AM2305_TimDelayHandle);

    TIM_IC_InitTypeDef sConfig;
    sConfig.ICFilter = 0x00;
    sConfig.ICPolarity = TIM_ICPOLARITY_BOTHEDGE;
    sConfig.ICPrescaler = TIM_ICPSC_DIV1;
    sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;

    HAL_TIM_IC_ConfigChannel(&AM2305_TimDelayHandle,
                             &sConfig,
                             TIM_CHANNEL_1);
    // Enable capture
    HAL_TIM_IC_Start(&AM2305_TimDelayHandle, TIM_CHANNEL_1);

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
    AM2305_SIGNAL_STATUS AM2305_SignalStatus = AM2305_SIGNAL_STATUS_RESPONSE_LOW;
    uint8_t data[AM2305_QTY_DATA_BITS/8];
    uint8_t cntBit=0;
    uint8_t cntByte=0;
    uint32_t time=0;
    uint32_t timesOld=0;
    uint32_t cnt=0;
    uint16_t measurements[AM2305_QTY_MEAS];
    uint8_t bit=0;


    // Start communicate. Getting response from sensor
    // Start signal
    AM2305_DQLow();
    
    // Host the start signal down time
    AM2305_Delay(AM2305_TIME_T_BE_US);
    
    // High DQ
    AM2305_DQInput();
    AM2305_TIMER->SR = (uint16_t)~(TIM_FLAG_CC1|TIM_FLAG_UPDATE);
    AM2305_TIMER->CNT = 0;

    while(AM2305_TIMER->CNT < AM2305_TIME_MEAS_US)
    {
        result = AM2305_CaptureTime(&time);
        if (RESULT_OK == result)
        {
            //AM2305_DQGetValue();
            measurements[cnt] = time - timesOld;
            timesOld = time;
            if (cnt == (AM2305_QTY_MEAS-1))
                break;
            cnt++;
        }
        else
        {
            break;
        }
    }
    // End communicate.

    // Start parsing received data.
    if (0 != cnt)
    {
        // Clear buf
        for (int i=0; i < AM2305_QTY_DATA_BITS/8; i++)
        {
            data[i] = 0;
        }
        for(int i = 0; i<AM2305_QTY_MEAS;i++)
        {
            switch(AM2305_SignalStatus)
            {
                case AM2305_SIGNAL_STATUS_RESPONSE_LOW :
                    if ((measurements[i] >= AM2305_TIME_T_REL_MIN_US) && \
                        (measurements[i] <= AM2305_TIME_T_REL_MAX_US))
                    {
                        AM2305_SignalStatus = AM2305_SIGNAL_STATUS_RESPONSE_HIGH;
                    }
                    break;
                case AM2305_SIGNAL_STATUS_RESPONSE_HIGH :
                    if ((measurements[i] >= AM2305_TIME_T_REH_MIN_US) && \
                        (measurements[i] <= AM2305_TIME_T_REH_MAX_US))
                    {
                        AM2305_SignalStatus = AM2305_SIGNAL_STATUS_SIGNAL_0_1_LOW;
                    }
                    break;
                case AM2305_SIGNAL_STATUS_SIGNAL_0_1_LOW :
                    if ((measurements[i] >= AM2305_TIME_T_LOW_MIN_US) && \
                        (measurements[i] <= AM2305_TIME_T_LOW_MAX_US))
                    {
                        AM2305_SignalStatus = AM2305_SIGNAL_STATUS_SIGNAL_0_HIGH;
                    }
                    break;
                case AM2305_SIGNAL_STATUS_SIGNAL_0_HIGH :
                case AM2305_SIGNAL_STATUS_SIGNAL_1_HIGH :
                    if ((measurements[i] >= AM2305_TIME_T_H0_MIN_US) && \
                        (measurements[i] <= AM2305_TIME_T_H0_MAX_US))
                    {
                        bit = 0;
                    }
                    else if ((measurements[i] >= AM2305_TIME_T_H1_MIN_US) && \
                             (measurements[i] <= AM2305_TIME_T_H1_MAX_US))
                    {
                        bit = 1;
                    }
                    AM2305_SignalStatus = AM2305_SIGNAL_STATUS_SIGNAL_0_1_LOW;
                    if (cntBit == 8U)
                    {
                        cntByte++;
                        cntBit=0;
                        data[cntByte] = 0;
                    }
                    data[cntByte] |= bit << (7U-cntBit);
                    if ((AM2305_QTY_DATA_BITS/8 == cntByte) && (7U == cntBit))
                    {
                        AM2305_SignalStatus = AM2305_SIGNAL_STATUS_END;
                    }
                    else
                    {
                        cntBit++;
                    }
                    break;
                case AM2305_SIGNAL_STATUS_END :
                    i = AM2305_QTY_MEAS;
                    break;
                default:
                    AM2305_SignalStatus = AM2305_SIGNAL_STATUS_SIGNAL_0_1_LOW;
                    break;
            }
        }

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

            temp = (((uint16_t)data[2])<<8) | data[3];
            // if temperature < 0
            if ((temp & (1<<15)) == 1<<15)
            {
                *temperature = -(float)(temp)/10;
            }
            else
            {
                *temperature = (float)(temp)/10;
            }
            result = RESULT_OK;
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
    // Gpio init
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // Config DQ as OUT open drain
    GPIO_InitStruct.Pin  = AM2305_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(AM2305_GPIO_PORT, &GPIO_InitStruct);

    // set DQ low
    HAL_GPIO_WritePin(AM2305_GPIO_PORT, AM2305_PIN, GPIO_PIN_RESET);
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
    // Gpio init
    GPIO_InitTypeDef GPIO_InitStruct;

    // Config DQ as Input AF
    GPIO_InitStruct.Pin  = AM2305_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = AM2305_GPIO_AF;
    HAL_GPIO_Init(AM2305_GPIO_PORT, &GPIO_InitStruct);
    
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
static GPIO_PinState AM2305_DQGetValue(void)
{
    uint8_t result = 0;
    //set PA1
    *(uint32_t*)0x40020018 = 1<<1;
    // Get value
    if ((AM2305_GPIO_PORT->IDR & (1<<AM2305_PIN)) != (uint32_t)GPIO_PIN_RESET)
    {
        result = (uint8_t)GPIO_PIN_SET;
    }
    else
    {
        result = (uint8_t)GPIO_PIN_RESET;
    }
    //reset PA1
    *(uint32_t*)0x40020018 = ((1<<1)<<16);
    return result;
}// end of AM2305_DQGetValue()



//**************************************************************************************************
// @Function      AM2305_Delay()
//--------------------------------------------------------------------------------------------------
// @Description   Delay function.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void AM2305_Delay(uint32_t microseconds)
{
    uint32_t nPeriods = 0;

    if (microseconds > AM2305_TIMER_PERIOD)
    {
        nPeriods = microseconds / (uint32_t)AM2305_TIMER_PERIOD;
        // Set auto-reload register
        AM2305_TIMER->ARR = AM2305_TIMER_PERIOD;

        for (int i = 0; i < nPeriods; i++)
        {
            /* Disable the TIM Counter */
            AM2305_TIMER->CR1 &= (uint16_t) (~((uint16_t) TIM_CR1_CEN));
            /* Clear the flags */
            AM2305_TIMER->SR = (uint16_t) ~TIM_FLAG_UPDATE;
            /* Clear counter */
            AM2305_TIMER->CNT = 0;
            /* Enable the TIM Counter */
            AM2305_TIMER->CR1 |= TIM_CR1_CEN;
            // wait
            while ((AM2305_TIMER->SR & TIM_FLAG_UPDATE) != TIM_FLAG_UPDATE);
        }
    }

    /* Disable the TIM Counter */
    AM2305_TIMER->CR1 &= (uint16_t) (~((uint16_t) TIM_CR1_CEN));
    /* Clear the flags */
    AM2305_TIMER->SR = (uint16_t) ~TIM_FLAG_UPDATE;
    // Set auto-reload register
    AM2305_TIMER->ARR = microseconds - (nPeriods * AM2305_TIMER_PERIOD);
    /* Clear counter */
    AM2305_TIMER->CNT = 0;
    /* Enable the TIM Counter */
    AM2305_TIMER->CR1 |= TIM_CR1_CEN;
    // wait
    while ((AM2305_TIMER->SR & TIM_FLAG_UPDATE) != TIM_FLAG_UPDATE);
}// end of AM2305_Delay()



//**************************************************************************************************
// @Function      AM2305_CaptureTime()
//--------------------------------------------------------------------------------------------------
// @Description   Capture time.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT AM2305_CaptureTime(uint32_t *const microseconds)
{
    STD_RESULT result = RESULT_NOT_OK;

    AM2305_TIMER->ARR = AM2305_TIMER_PERIOD;
    uint32_t flag_CC1=0;
    uint32_t flag_Update=0;

    while((RESET == flag_CC1) && (RESET == flag_Update))
    {
        flag_CC1 = LL_TIM_IsActiveFlag_CC1(AM2305_TIMER);
        flag_Update = LL_TIM_IsActiveFlag_UPDATE(AM2305_TIMER);
    }

    //AM2305_TIMER->CNT = 0;
    LL_TIM_ClearFlag_CC1(AM2305_TIMER);
    LL_TIM_ClearFlag_UPDATE(AM2305_TIMER);

    if (SET == flag_CC1)
    {
        *microseconds = AM2305_TIMER->CCR1;
        result = RESULT_OK;
    }
    else if (SET == flag_Update)
    {
        result = RESULT_NOT_OK;
    }

    return result;
}// end of AM2305_CaptureTime
//****************************************** end of file *******************************************
