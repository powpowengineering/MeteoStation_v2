//**************************************************************************************************
// @Module        USART
// @Filename      usart_drv.c
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
//                  USART_init()
//
//                Local (private) functions:

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
#include "usart_drv.h"



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

// pointer function
typedef void (*pf)(void);

// Setting Usart
typedef struct USART_SETTINGS_str
{
    uint8_t Enable;
    USART_TypeDef* channel;
    pf GpioTX;
    pf GpioRX;
    uint32_t PinTX;
    uint32_t PinRX;
    uint32_t BaudRate;
    uint8_t PartyBit;
    uint8_t SizeBits;
    uint8_t StopBits;
}USART_SETTINGS;

#define USART_SETTINGS_CHANNEL(ch){ \
    USART_CH_##ch,                  \
    USART_ALIAS_STD_LIB_CH_##ch     \
    USART_GPIO_TX_PIN_CH_##ch,      \
    USART_GPIO_RX_PIN_CH_##ch,      \
    USART_TX_PIN_CH_##ch,           \
    USART_RX_PIN_CH_##ch,           \
    USART_BAUDRATE_CH_##ch,         \
    USART_PARTY_BIT_CH_##ch,        \
    USART_DATA_SIZE_BITS_CH_##ch,   \
    USART_NUM_STOP_BITS_CH_##ch     \
}

//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

// Number settings
#define USART_NUM_SETTINGS              (10U)
// timeout
#define USART_TIMEOUT_TX                (0xffffU)
// An array to setting usart channels
const static USART_SETTINGS USART_Settings[USART_NUMBER_CHANNELS][USART_NUM_SETTINGS] =
{
    USART_SETTINGS_CHANNEL(0),
    USART_SETTINGS_CHANNEL(1),
    USART_SETTINGS_CHANNEL(2),
    USART_SETTINGS_CHANNEL(3),
    USART_SETTINGS_CHANNEL(4),
    USART_SETTINGS_CHANNEL(5),
    USART_SETTINGS_CHANNEL(6),
    USART_SETTINGS_CHANNEL(7)
};

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
// @Function      USART_init()
//--------------------------------------------------------------------------------------------------
// @Description   Init USART channels
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void USART_init(void)
{
    for (uint8_t i=0;i<USART_NUMBER_CHANNELS;i++)
    {
        if (ON == USART_Settings[i]->Enable)
        {
            USART_InitTypeDef USART_InitStruct;
            USART_InitStruct.USART_BaudRate = USART_Settings[i]->BaudRate;
            USART_InitStruct.USART_WordLength = USART_Settings[i]->SizeBits;
            USART_InitStruct.USART_StopBits = USART_Settings[i]->StopBits;
            USART_InitStruct.USART_Parity = USART_Settings[i]->PartyBit;
            USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
            USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
            USART_Init(USART_Settings[i]->channel, &USART_InitStruct);
        }
    }
}// end of USART_init()



//**************************************************************************************************
// @Function      USART_PutChar()
//--------------------------------------------------------------------------------------------------
// @Description   Put character
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void USART_PutChar(const uint8_t channel, const uint8_t character)
{

    uint32_t timeout = USART_TIMEOUT_TX;
    while( timeout != 0)
    {
        if (SET == USART_GetFlagStatus(USART_Settings[channel]->channel, USART_IT_TC))
        {
            USART_SendData(USART_Settings[channel]->channel, character);
            break;
        }
        timeout--;
    }

}// end of USART_PutChar()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

// None.


//****************************************** end of file *******************************************

