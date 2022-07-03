//**************************************************************************************************
// @Module        USART
// @Filename      Usart_drv_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the USART module.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************

#ifndef USART_DRV_CFG_H
#define USART_DRV_CFG_H


//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// Enable/disable the development error detection feature of the program module.
// Used only for debug purposes and should be disabled in the final release.
// Valid values: ON / OFF
#define MODULE_DEVELOPMENT_ERROR_DETECTION      (OFF)

// User can enable/disable the internal diagnostic of the program module.
// Used only for debug purposes and should be disabled in the final release.
// Valid values: ON / OFF
#define MODULE_INTERNAL_DIAGNOSTICS             (OFF)

// Amount channels
#define USART_NUMBER_CHANNELS                   (2U)

//
#define USART_CH_0                              (ON)
// alias std lib usart
#define USART_ALIAS_STD_LIB_CH_0                USART1
// GPIO TX PIN channel 0
#define USART_GPIO_TX_PIN_CH_0                  GPIOA
// GPIO RX PIN channel 0
#define USART_GPIO_RX_PIN_CH_0                  GPIOA
// TX PIN channel 0
#define USART_TX_PIN_CH_0                       (GPIO_Pin_9)
// RX PIN channel 0
#define USART_RX_PIN_CH_0                       (GPIO_Pin_10)
// Usart channel 0 baudrate
#define USART_BAUDRATE_CH_0                     (115200)
// Usart party bit
#define USART_PARTY_BIT_CH_0                    USART_Parity_No
// Usart size data bit
#define USART_DATA_SIZE_BITS_CH_0               (8U)
// Usart stop bits
#define USART_NUM_STOP_BITS_CH_0                (1U)
// DMA TX enable
#define USART_DMA_TX_EN_0                       (OFF)
// DMA RX enable
#define USART_DMA_RX_EN_0                       (OFF)
//// Enable control bit Transmit Data Register Empty
//#define USART_TXEIE_EN_0                        (OFF)
//// CTS flag
//#define USART_CTSIE_EN_0                        (OFF)
//// Transmission Complete
//#define USART_TCIE_EN_0                         (OFF)
//// Received Data Ready to be Read
//// Overrun Error Detected
//#define USART_RXNEIE_EN_0                       (OFF)
//// Idle Line Detected
//#define USART_IDLEIE_EN_0                       (OFF)
//// Parity Error
//#define USART_PEIE_EN_0                         (OFF)
//// Break Flag
//#define USART_ELBDIE_EN_0                       (OFF)
//// Noise Flag, Overrun error and Framing Error in multibuffer
////communication
//#define USART_EIE_EN_0                          (OFF)



#define USART_CH_1                              (ON)
#define USART_ALIAS_STD_LIB_CH_1                USART3
#define USART_GPIO_TX_PIN_CH_1                  GPIOB
#define USART_GPIO_RX_PIN_CH_1                  GPIOB
#define USART_TX_PIN_CH_1                       (GPIO_Pin_10)
#define USART_RX_PIN_CH_1                       (GPIO_Pin_11)
#define USART_BAUDRATE_CH_1                     (115200)
#define USART_PARTY_BIT_CH_1                    USART_Parity_No
#define USART_DATA_SIZE_BITS_CH_1               (8U)
#define USART_NUM_STOP_BITS_CH_1                (1U)
#define USART_DMA_TX_EN_1                       (ON)
#define USART_DMA_RX_EN_1                       (ON)
//#define USART_TXEIE_EN_1                        (OFF)
//#define USART_CTSIE_EN_1                        (OFF)
//#define USART_TCIE_EN_1                         (OFF)
//#define USART_RXNEIE_EN_1                       (OFF)
//#define USART_IDLEIE_EN_1                       (ON)
//#define USART_PEIE_EN_1                         (OFF)
//#define USART_ELBDIE_EN_1                       (OFF)
//#define USART_EIE_EN_1                          (OFF)




#endif // #ifndef USART_DRV_CFG_H

//****************************************** end of file *******************************************
