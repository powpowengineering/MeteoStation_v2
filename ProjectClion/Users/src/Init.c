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

#include "time_drv.h"

#include "stm32l4xx_ll_dma.h"

#include "stm32l4xx_ll_usart.h"



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

// ADC Handler
ADC_HandleTypeDef ADC_Handle;

#define TERMINAL_RX_BUF_SIZE            (512U)
uint8_t TERMINAL_BufRx[TERMINAL_RX_BUF_SIZE];


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

// DMA hanle for UART TERMINAL
static DMA_HandleTypeDef DMA_Handle;



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
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_TIM6_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();


    // Configure the GPIO_LED pin
    GPIO_InitStruct.Pin   = INIT_LED2_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(INIT_LED2_PORT, &GPIO_InitStruct);

    // Configure DC GSM Pin
    GPIO_InitStruct.Pin   = INIT_DC_GSM_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(INIT_DC_GSM_PORT, &GPIO_InitStruct);

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

    // Initialize pin PB1 for BATTERY
    GPIO_InitStruct.Pin        = INIT_BAT_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull       = GPIO_NOPULL;
    HAL_GPIO_Init(INIT_BAT_PORT, &GPIO_InitStruct);

    // Initialize pin PB0 for ANEMOMETER
    GPIO_InitStruct.Pin        = INIT_ANEMOMETER_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull       = GPIO_NOPULL;
    HAL_GPIO_Init(INIT_ANEMOMETER_PORT, &GPIO_InitStruct);

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

    // Configure DMA for UART TERMINAL
    DMA_Handle.Instance = DMA1_Channel6;
    DMA_Handle.Init.Request = DMA_REQUEST_2;
    DMA_Handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    DMA_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
    DMA_Handle.Init.MemInc = DMA_MINC_ENABLE;
    DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    DMA_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    DMA_Handle.Init.Mode = DMA_CIRCULAR;
    DMA_Handle.Init.Priority = DMA_PRIORITY_LOW;
    DMA_Handle.DmaBaseAddress = DMA1;
    HAL_DMA_Init(&DMA_Handle);
    HAL_DMA_Start (&DMA_Handle,
                   LL_USART_DMA_GetRegAddr(INIT_TERMINAL_USART_NUM, LL_USART_DMA_REG_DATA_RECEIVE),
                   (uint32_t)TERMINAL_BufRx,
                   TERMINAL_RX_BUF_SIZE);

    // Configure UART for TERMINAL
    UartTERMINALHandle.Instance            = INIT_TERMINAL_USART_NUM;
    UartTERMINALHandle.Init.BaudRate       = 9600;
    UartTERMINALHandle.Init.WordLength     = USART_WORDLENGTH_8B;
    UartTERMINALHandle.Init.StopBits       = USART_STOPBITS_1;
    UartTERMINALHandle.Init.Parity         = USART_PARITY_NONE;
    UartTERMINALHandle.Init.Mode           = USART_MODE_TX_RX;

    HAL_UART_DeInit(&UartTERMINALHandle);
    // Init UART TERMINAL
    HAL_UART_Init(&UartTERMINALHandle);
//    LL_USART_EnableDMAReq_RX(INIT_TERMINAL_USART_NUM);

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

    //ADC set
    ADC_Handle.Instance = INIT_ADC_NUM;
    ADC_Handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    ADC_Handle.Init.Resolution = ADC_RESOLUTION_8B;
    ADC_Handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    ADC_Handle.Init.ScanConvMode = ADC_SCAN_ENABLE;
    ADC_Handle.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    ADC_Handle.Init.LowPowerAutoWait = DISABLE;
    ADC_Handle.Init.ContinuousConvMode = DISABLE;
    ADC_Handle.Init.NbrOfConversion = 0U;
    ADC_Handle.Init.DiscontinuousConvMode = DISABLE;
    ADC_Handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    ADC_Handle.Init.DMAContinuousRequests = DISABLE;
    ADC_Handle.Init.OversamplingMode = DISABLE;
    ADC_Handle.InjectionConfig.ChannelCount = 2U;
    ADC_Handle.InjectionConfig.ContextQueue = 1U;
    HAL_ADC_Init(&ADC_Handle);

    ADC_InjectionConfTypeDef ADC_InjectionConf;
    ADC_InjectionConf.InjectedChannel = INIT_BAT_AN_CH;
    ADC_InjectionConf.InjectedRank = INIT_BAT_RANK;
    ADC_InjectionConf.InjectedNbrOfConversion = 2;
    ADC_InjectionConf.InjectedSamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    ADC_InjectionConf.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
    ADC_InjectionConf.AutoInjectedConv = DISABLE;
    ADC_InjectionConf.InjectedDiscontinuousConvMode = DISABLE;
    ADC_InjectionConf.InjectedOffset = 0;
    HAL_ADCEx_InjectedConfigChannel(&ADC_Handle, &ADC_InjectionConf);

    ADC_InjectionConf.InjectedChannel = INIT_ANEMOMETER_AN_CH;
    ADC_InjectionConf.InjectedRank = INIT_ANEMOMETER_RANK;
    ADC_InjectionConf.InjectedNbrOfConversion = 2;
    ADC_InjectionConf.InjectedSamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    ADC_InjectionConf.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
    ADC_InjectionConf.AutoInjectedConv = DISABLE;
    ADC_InjectionConf.InjectedDiscontinuousConvMode = DISABLE;
    ADC_InjectionConf.InjectedOffset = 0;
    HAL_ADCEx_InjectedConfigChannel(&ADC_Handle, &ADC_InjectionConf);

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

    // Init RTC
    TIME_Init();
}
// end of Init()



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