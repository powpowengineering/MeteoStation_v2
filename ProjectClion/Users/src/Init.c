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
// Get hal_ll rtc
#include "stm32l4xx_ll_rtc.h"
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

// RTC handle
RTC_HandleTypeDef RTC_Handle;



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

// Init RTC
static void INIT_RTC(void);



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
    INIT_RTC();
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



//**************************************************************************************************
// @Function      INIT_RTC()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void INIT_RTC(void)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    RTC_AlarmTypeDef sAlarm;

    RTC_Handle.Instance = RTC;

    // Check cause reset
    /* Check if the system was resumed not from Standby mode */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == RESET)
    {
        // Do delay to have ability connect debugger
        HAL_GPIO_WritePin(INIT_LED2_PORT, INIT_LED2_PIN, GPIO_PIN_SET);
        INIT_Delay(3000000);
        HAL_GPIO_WritePin(INIT_LED2_PORT, INIT_LED2_PIN, GPIO_PIN_RESET);

        // Config RTC
        RTC_Handle.Init.HourFormat = RTC_HOURFORMAT_24;
        RTC_Handle.Init.AsynchPrediv = INIT_RTC_ASYNCHPREDIV;
        RTC_Handle.Init.SynchPrediv = INIT_RTC_SYNCHPREDIV;
        RTC_Handle.Init.OutPut = INIT_RTC_OUTPUT;
        RTC_Handle.Init.OutPutRemap = INIT_RTC_OUTPUT_REMAP;
        RTC_Handle.Init.OutPutPolarity = INIT_RTC_OUTPUT_POLARITY;
        RTC_Handle.Init.OutPutType = INIT_RTC_OUTPUT_TYPE;
        HAL_RTC_Init(&RTC_Handle);

        sTime.Hours = INIT_RTC_TIME_HOUR_DEF;
        sTime.Minutes = INIT_RTC_TIME_MINUTES_DEF;
        sTime.Seconds = INIT_RTC_TIME_SECONDS_DEF;
        sTime.TimeFormat = INIT_RTC_TIMEFORMAT;
        HAL_RTC_SetTime(&RTC_Handle, &sTime, RTC_FORMAT_BIN);

        sDate.Date = 24;
        sDate.Month = RTC_MONTH_AUGUST;
        sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
        sDate.Year = 22;
        HAL_RTC_SetDate(&RTC_Handle, &sDate, RTC_FORMAT_BIN);

        // Set alarm fo sensors
        sAlarm.AlarmTime.Hours = sTime.Hours + INIT_ALARM_SENS_HOURS;
        sAlarm.AlarmTime.Minutes = sTime.Minutes + INIT_ALARM_SENS_MINUTES;
        sAlarm.AlarmTime.Seconds = sTime.Seconds + INIT_ALARM_SENS_SECONDS;
        sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
        sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
        sAlarm.AlarmMask = INIT_ALARM_SENS_MASK;
        sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
        sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
        sAlarm.AlarmDateWeekDay = 1;
        sAlarm.Alarm = INIT_ALARM_SENS_NUM;
        HAL_RTC_SetAlarm_IT(&RTC_Handle, &sAlarm, FORMAT_BIN);

        // Set alarm fo GSM
        sAlarm.AlarmTime.Hours = sTime.Hours + INIT_ALARM_GSM_HOURS;
        sAlarm.AlarmTime.Minutes = sTime.Minutes + INIT_ALARM_GSM_MINUTES;
        sAlarm.AlarmTime.Seconds = sTime.Seconds + INIT_ALARM_GSM_SECONDS;
        sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
        sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
        sAlarm.AlarmMask = INIT_ALARM_GSM_MASK;
        sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
        sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
        sAlarm.AlarmDateWeekDay = 1;
        sAlarm.Alarm = INIT_ALARM_GSM_NUM;
        HAL_RTC_SetAlarm_IT(&RTC_Handle, &sAlarm, FORMAT_BIN);

        __HAL_RTC_WRITEPROTECTION_DISABLE(&RTC_Handle);
        LL_RTC_DisableIT_TAMP(RTC_Handle.Instance);
        LL_RTC_DisableIT_TAMP1(RTC_Handle.Instance);
        LL_RTC_DisableIT_TAMP2(RTC_Handle.Instance);
        LL_RTC_DisableIT_TAMP3(RTC_Handle.Instance);
        LL_RTC_DisableIT_TS(RTC_Handle.Instance);
        LL_RTC_DisableIT_WUT(RTC_Handle.Instance);
        __HAL_RTC_WRITEPROTECTION_ENABLE(&RTC_Handle);
    }

} // end of INIT_RTC()


void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /*##-1- Enables the PWR Clock and Enables access to the backup domain ######*/
    /* To enable access on RTC registers */
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();


    /*##-2- Configure LSE/LSI as RTC clock source ###############################*/
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        //Error_Handler();
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        //Error_Handler();
    }
    /* Configures the External Low Speed oscillator (LSE) drive capability */
//    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);

    /*##-2- Enable the RTC & BKP peripheral Clock ##############################*/
    /* Enable RTC Clock */
    __HAL_RCC_RTC_ENABLE();

    /* Enable RTC APB clock  */
//    __HAL_RCC_RTCAPB_CLK_ENABLE();
}
//****************************************** end of file *******************************************