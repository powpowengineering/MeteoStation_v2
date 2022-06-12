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
// @Function      InitGpioSpi1OutBoard()
//--------------------------------------------------------------------------------------------------
// @Description   Init Spi1 on pin PB3,PB4,PB5
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void InitGpioSpi1OutBoard(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_3;// PB3 SPI1_SCK
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);

    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_4;// PB4 SPI1_MISO
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    //GPIO_PinAFConfig(GPIOB, GPIO_Pin_4, GPIO_AF_SPI1);

    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_5;// PB5 SPI1_MOSI
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
}
// end of InitGpioSpi1OutBoard()



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
    /*Enable Periph*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(    RCC_APB2Periph_TIM9, ENABLE);
    RCC_APB2PeriphClockCmd(    RCC_APB2Periph_ADC1, ENABLE); //activated ADC
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    RCC_APB1PeriphClockCmd(    RCC_APB1Periph_USART3, ENABLE);
    RCC_APB1PeriphClockCmd(    RCC_APB1Periph_I2C1, ENABLE);


    /*GPIO Init*/
    // test pin
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    //initialize pin A2 for anemometer
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_2;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN; //because signal is analogue
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz; //speed for digital
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    // test pin
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);


    /*USART1 TX*/
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);

    /*USART1 RX*/
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    /*USART3 TX*/
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);

    /*USART3 RX*/
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);



    // Init Gpio SPI1
    //InitGpioSpi1OutBoard();

    /*Init SPI1*/
 /*   SPI_InitTypeDef SPI_InitStruct;
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStruct);

    SPI_Cmd(SPI1, ENABLE);
*/

    /*Init TIM6 for OneWire delay*/
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Period = INIT_PERIOD_TIMER_DELAY;
    TIM_TimeBaseInitStruct.TIM_Prescaler = INIT_PSC_TIMER_DELAY;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(INIT_TIMER_DELAY, &TIM_TimeBaseInitStruct);
    //ADC set
    ADC_InitTypeDef ADC_InitStruct;
    /* Reset ADC init structure parameters values */
    ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;
    ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStruct.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_InitStruct);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_24Cycles);

    ADC_Cmd(ADC1, ENABLE);

    I2C_InitTypeDef I2C_InitStruct;

/*---------------- Reset I2C init structure parameters values ----------------*/
        /* initialize the I2C_ClockSpeed member */
        I2C_InitStruct.I2C_ClockSpeed = 5000;
        /* Initialize the I2C_Mode member */
        I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
        /* Initialize the I2C_DutyCycle member */
        I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
        /* Initialize the I2C_OwnAddress1 member */
        I2C_InitStruct.I2C_OwnAddress1 = 0;
        /* Initialize the I2C_Ack member */
        I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
        /* Initialize the I2C_AcknowledgedAddress member */
        I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
        I2C_Init(I2C1, &I2C_InitStruct);


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
            INIT_TIMER_DELAY->SR = (uint16_t) ~TIM_FLAG_Update;
            /* Clear counter */
            INIT_TIMER_DELAY->CNT = 0;
            /* Enable the TIM Counter */
            INIT_TIMER_DELAY->CR1 |= TIM_CR1_CEN;
            // wait
            while ((INIT_TIMER_DELAY->SR & TIM_FLAG_Update) != TIM_FLAG_Update);
        }
    }

    /* Disable the TIM Counter */
    INIT_TIMER_DELAY->CR1 &= (uint16_t) (~((uint16_t) TIM_CR1_CEN));
    /* Clear the flags */
    INIT_TIMER_DELAY->SR = (uint16_t) ~TIM_FLAG_Update;
    // Set auto-reload register
    INIT_TIMER_DELAY->ARR = us - (nPeriods * INIT_PERIOD_TIMER_DELAY);
    /* Clear counter */
    INIT_TIMER_DELAY->CNT = 0;
    /* Enable the TIM Counter */
    INIT_TIMER_DELAY->CR1 |= TIM_CR1_CEN;
    // wait
    while ((INIT_TIMER_DELAY->SR & TIM_FLAG_Update) != TIM_FLAG_Update);

}
// end of INIT_Delay()

//****************************************** end of file *******************************************