//**************************************************************************************************
// @Module        INIT
// @Filename      Init.c
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
//
//
//                Local (private) functions:
//
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
#include "Init.h"

#include "stm32l4xx_ll_tim.h"

//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None.


//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// TLM handler
USART_HandleTypeDef UsartTLMHandle;

// TERMINAL UART handler
UART_HandleTypeDef UartTERMINALHandle;

// GSM UART handler
UART_HandleTypeDef UartGSMHandler;

// BMP280 I2C handler
I2C_HandleTypeDef I2CBMP280Handler;

// Tme delay handler
TIM_HandleTypeDef    TimDelayHandle;



//**************************************************************************************************
// Declarations of local (private) data types
//**************************************************************************************************

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

// None.



//**************************************************************************************************
// @Function      Init()
//--------------------------------------------------------------------------------------------------
// @Description   Init MCU
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_TIM6_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    // Configure the GPIO_LED pin
    GPIO_InitStruct.Pin   = GPIO_PIN_5;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configure the TX pin UART for TLM
    GPIO_InitStruct.Pin        = INIT_TLM_USART_TX_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLUP;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = INIT_TLM_USART_TX_AF;
    HAL_GPIO_Init(INIT_TLM_USART_TX_PORT, &GPIO_InitStruct);

    // Configure the RX pin UART for TLM
    GPIO_InitStruct.Pin        = INIT_TERMINAL_USART_RX_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLUP;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = INIT_TERMINAL_USART_RX_AF;
    HAL_GPIO_Init(INIT_TERMINAL_USART_RX_PORT, &GPIO_InitStruct);

    // Configure the TX pin UART for TERMINAL
    GPIO_InitStruct.Pin        = INIT_TERMINAL_USART_TX_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLUP;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = INIT_TERMINAL_USART_TX_AF;
    HAL_GPIO_Init(INIT_TERMINAL_USART_TX_PORT, &GPIO_InitStruct);

    // Configure the RX pin UART for TERMINAL
    GPIO_InitStruct.Pin        = INIT_TERMINAL_USART_RX_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLUP;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = INIT_TERMINAL_USART_RX_AF;
    HAL_GPIO_Init(INIT_TERMINAL_USART_RX_PORT, &GPIO_InitStruct);

    // Configure the RX pin UART for TERMINAL DTR
    GPIO_InitStruct.Pin        = INIT_TERMINAL_DTR_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLUP;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(INIT_TERMINAL_DTR_PORT, &GPIO_InitStruct);

    // Configure the TX pin UART for GSM
    GPIO_InitStruct.Pin        = INIT_GSM_USART_TX_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLUP;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = INIT_GSM_USART_TX_AF;
    HAL_GPIO_Init(INIT_GSM_USART_TX_PORT, &GPIO_InitStruct);

    // Configure the RX pin UART for GSM
    GPIO_InitStruct.Pin        = INIT_GSM_USART_RX_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLUP;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = INIT_GSM_USART_RX_AF;
    HAL_GPIO_Init(INIT_GSM_USART_RX_PORT, &GPIO_InitStruct);

//    //initialize pin A2 for anemometer
//    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_2;
//    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN; //because signal is analogue
//    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz; //speed for digital
//    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
//    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configure the SCL pin I2C1 for BMP280
    GPIO_InitStruct.Pin  = INIT_BMP280_I2C_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = INIT_BMP280_I2C_SCL_AF;
    HAL_GPIO_Init(INIT_BMP280_I2C_SCL_PORT, &GPIO_InitStruct);

    // Configure the SDA pin I2C1 for BMP280
    GPIO_InitStruct.Pin  = INIT_BMP280_I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = INIT_BMP280_I2C_SDA_AF;
    HAL_GPIO_Init(INIT_BMP280_I2C_SDA_PORT, &GPIO_InitStruct);

    // Configure UART for TLM
    UsartTLMHandle.Instance            = INIT_TLM_USART_NUM;
    UsartTLMHandle.Init.BaudRate       = 9600;
    UsartTLMHandle.Init.WordLength     = USART_WORDLENGTH_8B;
    UsartTLMHandle.Init.StopBits       = USART_STOPBITS_1;
    UsartTLMHandle.Init.Parity         = USART_PARITY_NONE;
    UsartTLMHandle.Init.Mode           = USART_MODE_TX_RX;

    HAL_USART_DeInit(&UsartTLMHandle);
    // Init UART TLM
    HAL_USART_Init(&UsartTLMHandle);

    // Configure UART for TERMINAL
    UartTERMINALHandle.Instance            = INIT_TERMINAL_USART_NUM;
    UartTERMINALHandle.Init.BaudRate       = 9600;
    UartTERMINALHandle.Init.WordLength     = USART_WORDLENGTH_8B;
    UartTERMINALHandle.Init.StopBits       = USART_STOPBITS_1;
    UartTERMINALHandle.Init.Parity         = USART_PARITY_NONE;
    UartTERMINALHandle.Init.Mode           = USART_MODE_TX_RX;

    HAL_UART_DeInit(&UartTERMINALHandle);
    // Init UART TERMINAL
//    HAL_USART_Init(&UsartTERMINALHandle);
    HAL_UART_Init(&UartTERMINALHandle);

    // Configure UART for GSM
    UartGSMHandler.Instance            = INIT_GSM_USART_NUM;
    UartGSMHandler.Init.BaudRate       = 9600;
    UartGSMHandler.Init.WordLength     = USART_WORDLENGTH_8B;
    UartGSMHandler.Init.StopBits       = USART_STOPBITS_1;
    UartGSMHandler.Init.Parity         = USART_PARITY_NONE;
    UartGSMHandler.Init.Mode           = USART_MODE_TX_RX;

    HAL_UART_DeInit(&UartGSMHandler);
    // Init UART TERMINAL
//    HAL_USART_Init(&UsartTERMINALHandle);
    HAL_UART_Init(&UartGSMHandler);


    TimDelayHandle.Instance = INIT_TIMER_DELAY;

    /* Initialize TIMx peripheral as follows:
         + Period = 10000 - 1
         + Prescaler = (SystemCoreClock/10000) - 1
         + ClockDivision = 0
         + Counter direction = Up
    */
    TimDelayHandle.Init.Period            = INIT_PERIOD_TIMER_DELAY;
    TimDelayHandle.Init.Prescaler         = INIT_PSC_TIMER_DELAY;
    TimDelayHandle.Init.ClockDivision     = 0;
    TimDelayHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimDelayHandle.Init.RepetitionCounter = 0;

    HAL_TIM_Base_Init(&TimDelayHandle);


//    /*Init TIM6 for OneWire delay*/
//    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
//    TIM_TimeBaseInitStruct.TIM_Period = INIT_PERIOD_TIMER_DELAY;
//    TIM_TimeBaseInitStruct.TIM_Prescaler = INIT_PSC_TIMER_DELAY;
//    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
//    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
//    TIM_TimeBaseInit(INIT_TIMER_DELAY, &TIM_TimeBaseInitStruct);
//    //ADC set
//    ADC_InitTypeDef ADC_InitStruct;
//    /* Reset ADC init structure parameters values */
//    ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
//    ADC_InitStruct.ADC_ScanConvMode = DISABLE;
//    ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
//    ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
//    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;
//    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
//    ADC_InitStruct.ADC_NbrOfConversion = 1;
//    ADC_Init(ADC1, &ADC_InitStruct);
//
//    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_24Cycles);
//
//    ADC_Cmd(ADC1, ENABLE);
//
    // Init I2C1 for BMP280
    I2CBMP280Handler.Instance = INIT_BMP280_I2C_NUM;
    I2CBMP280Handler.Init.Timing = INIT_BMP280_I2C_TIMING;
    I2CBMP280Handler.Init.OwnAddress1 = INIT_BMP280_I2C_OWNADDRESS1;
    I2CBMP280Handler.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    I2CBMP280Handler.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2CBMP280Handler.Init.OwnAddress2 = INIT_BMP280_I2C_OWNADDRESS2;
    I2CBMP280Handler.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    I2CBMP280Handler.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2CBMP280Handler.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    I2CBMP280Handler.Mode = HAL_I2C_MODE_MASTER;
    I2CBMP280Handler.Devaddress = INIT_BMP280_I2C_DEV_ADR;

    HAL_I2C_DeInit(&I2CBMP280Handler);
    HAL_I2C_Init(&I2CBMP280Handler);
}
// end of Init()


//**************************************************************************************************
// @Function      INIT_TerminalSend()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void INIT_TerminalSend(const char* data, int16_t size)
{
    while (size) {
        while ((INIT_TERMINAL_USART_NUM->ISR & USART_ISR_TXE) != USART_ISR_TXE);
        INIT_TERMINAL_USART_NUM->TDR = *data;
        ++data;
        --size;
    }
} // end of INIT_TerminalSend()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      INIT_Delay()
//--------------------------------------------------------------------------------------------------
// @Description   Delay function in us
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    us - microseconds,[0..2^32-1]
//**************************************************************************************************
void INIT_Delay(uint32_t us)
{
    uint32_t nPeriods = 0;

    if (us > INIT_PERIOD_TIMER_DELAY) {
        nPeriods = us / (uint32_t)INIT_PERIOD_TIMER_DELAY;
        // Set auto-reload register
        INIT_TIMER_DELAY->ARR = INIT_PERIOD_TIMER_DELAY;

        for (int i = 0; i < nPeriods; i++) {
            /* Disable the TIM Counter */
            INIT_TIMER_DELAY->CR1 &= (uint16_t) (~((uint16_t) TIM_CR1_CEN));
            /* Clear the flags */
            INIT_TIMER_DELAY->SR = (uint16_t) ~LL_TIM_SR_UIF;
            /* Clear counter */
            INIT_TIMER_DELAY->CNT = 0;
            /* Enable the TIM Counter */
            INIT_TIMER_DELAY->CR1 |= TIM_CR1_CEN;
            // wait
            while ((INIT_TIMER_DELAY->SR & LL_TIM_SR_UIF) != LL_TIM_SR_UIF);
        }
    }

    /* Disable the TIM Counter */
    INIT_TIMER_DELAY->CR1 &= (uint16_t) (~((uint16_t) TIM_CR1_CEN));
    /* Clear the flags */
    INIT_TIMER_DELAY->SR = (uint16_t) ~LL_TIM_SR_UIF;
    // Set auto-reload register
    INIT_TIMER_DELAY->ARR = us - (nPeriods * INIT_PERIOD_TIMER_DELAY);
    /* Clear counter */
    INIT_TIMER_DELAY->CNT = 0;
    /* Enable the TIM Counter */
    INIT_TIMER_DELAY->CR1 |= TIM_CR1_CEN;
    // wait
    while ((INIT_TIMER_DELAY->SR & LL_TIM_SR_UIF) != LL_TIM_SR_UIF);
}
// end of INIT_Delay()



//****************************************** end of file *******************************************