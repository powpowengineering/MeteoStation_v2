//**************************************************************************************************
// @Module        TASK_MQTT
// @Filename      task_mqtt_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the TASK_MQTT module.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef MQTT_CFG_H
#define MQTT_CFG_H



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

// Confugure UART for GSM module
#define INIT_GSM_USART_NUM                      USART2
#define INIT_GSM_USART_TX_PIN                   GPIO_PIN_2
#define INIT_GSM_USART_TX_PORT                  GPIOA
#define INIT_GSM_USART_RX_PIN                   GPIO_PIN_3
#define INIT_GSM_USART_RX_PORT                  GPIOA
#define INIT_GSM_USART_TX_AF                    GPIO_AF7_USART2
#define INIT_GSM_USART_RX_AF                    GPIO_AF7_USART2

#endif // #ifndef MQTT_CFG_H

//****************************************** end of file *******************************************
