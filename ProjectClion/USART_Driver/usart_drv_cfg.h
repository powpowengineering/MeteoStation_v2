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
#define USART_NUMBER_CHANNELS                   (1U)
// number ch 0
#define USART_CH_0                              (USART3)
// GPIO TX PIN channel 0
#define USART_GPIO_TX_PIN_CH_0                  (GPIOB)
// GPIO RX PIN channel 0
#define USART_GPIO_RX_PIN_CH_0                  (GPIOB)
// TX PIN channel 0
#define USART_TX_PIN_CH_0                       (GPIO_PIN_10)
// RX PIN channel 0
#define USART_RX_PIN_CH_0                       (GPIO_PIN_11)

#endif // #ifndef USART_DRV_CFG_H

//****************************************** end of file *******************************************
