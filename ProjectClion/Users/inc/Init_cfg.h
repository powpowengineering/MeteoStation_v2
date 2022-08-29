//**************************************************************************************************
// @Module        INIT
// @Filename      init_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the INIT module.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************

#ifndef INIT_CFG_H
#define INIT_CFG_H



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

// Configure UART for TLM
#define INIT_TLM_USART_NUM                      USART1
#define INIT_TLM_USART_TX_PIN                   GPIO_PIN_6
#define INIT_TLM_USART_TX_PORT                  GPIOB
#define INIT_TLM_USART_RX_PIN                   GPIO_PIN_7
#define INIT_TLM_USART_RX_PORT                  GPIOB
#define INIT_TLM_USART_TX_AF                    GPIO_AF7_USART1
#define INIT_TLM_USART_RX_AF                    GPIO_AF7_USART1

// Configure UART for TERMINAL
#define INIT_TERMINAL_USART_NUM                 USART2
#define INIT_TERMINAL_USART_TX_PIN              GPIO_PIN_2
#define INIT_TERMINAL_USART_TX_PORT             GPIOA
#define INIT_TERMINAL_USART_RX_PIN              GPIO_PIN_3
#define INIT_TERMINAL_USART_RX_PORT             GPIOA
#define INIT_TERMINAL_USART_TX_AF               GPIO_AF7_USART2
#define INIT_TERMINAL_USART_RX_AF               GPIO_AF7_USART2
#define INIT_TERMINAL_DTR_PORT                  GPIOA
#define INIT_TERMINAL_DTR_PIN                   GPIO_PIN_4

// Configure UART for GSM
#define INIT_GSM_USART_NUM                      USART3
#define INIT_GSM_USART_TX_PIN                   GPIO_PIN_10
#define INIT_GSM_USART_TX_PORT                  GPIOB
#define INIT_GSM_USART_RX_PIN                   GPIO_PIN_11
#define INIT_GSM_USART_RX_PORT                  GPIOB
#define INIT_GSM_USART_TX_AF                    GPIO_AF7_USART3
#define INIT_GSM_USART_RX_AF                    GPIO_AF7_USART3

// Configure I2C1 for BMP280
#define INIT_BMP280_I2C_NUM                      I2C1
#define INIT_BMP280_I2C_SCL_PIN                  GPIO_PIN_8
#define INIT_BMP280_I2C_SCL_PORT                 GPIOB
#define INIT_BMP280_I2C_SDA_PIN                  GPIO_PIN_9
#define INIT_BMP280_I2C_SDA_PORT                 GPIOB
#define INIT_BMP280_I2C_SCL_AF                   GPIO_AF4_I2C1
#define INIT_BMP280_I2C_SDA_AF                   GPIO_AF4_I2C1
#define INIT_BMP280_I2C_OWNADDRESS1              (0U)
#define INIT_BMP280_I2C_OWNADDRESS2              (0U)
#define INIT_BMP280_I2C_DEV_ADR                  (0x76U)
#define INIT_BMP280_I2C_TIMING                   (0xF0001818U)

// Configure analog pin for Battery
#define INIT_BAT_PORT                            GPIOB
#define INIT_BAT_PIN                             GPIO_PIN_0
#define INIT_BAT_AN_CH                           ADC_CHANNEL_15
#define INIT_BAT_RANK                            ADC_INJECTED_RANK_1

// Configure analog pin for anemometr
#define INIT_ANEMOMETER_PORT                     GPIOB
#define INIT_ANEMOMETER_PIN                      GPIO_PIN_1
#define INIT_ANEMOMETER_AN_CH                    ADC_CHANNEL_16
#define INIT_ANEMOMETER_RANK                     ADC_INJECTED_RANK_2

// Configure DC/DC control GSM pin
#define INIT_DC_GSM_PORT                         GPIOC
#define INIT_DC_GSM_PIN                          GPIO_PIN_12

// Configure ADC
#define INIT_ADC_NUM                             ADC1

// Configure RTC
#define INIT_RTC_ASYNCHPREDIV                   (127U)
#define INIT_RTC_SYNCHPREDIV                    (255U)
#define INIT_RTC_OUTPUT                         (RTC_OUTPUT_ALARMA)
#define INIT_RTC_OUTPUT_REMAP                   (RTC_OUTPUT_REMAP_NONE)
#define INIT_RTC_OUTPUT_POLARITY                (RTC_OUTPUT_POLARITY_HIGH)
#define INIT_RTC_OUTPUT_TYPE                    (RTC_OUTPUT_TYPE_PUSHPULL)
#define INIT_RTC_TIME_HOUR_DEF                  (12U)
#define INIT_RTC_TIME_MINUTES_DEF               (0U)
#define INIT_RTC_TIME_SECONDS_DEF               (0U)
#define INIT_RTC_TIMEFORMAT                     (RTC_HOURFORMAT12_PM)

// Configure LED2 pin PA5
#define INIT_LED2_PORT                          GPIOA
#define INIT_LED2_PIN                           GPIO_PIN_5



//  Configure for one wire delay
#define INIT_TIMER_DELAY                          TIM6
// User specify prescaler for one wire delay timer
// Valid value: [0,0xFF]
#define INIT_PSC_TIMER_DELAY                      (79U)
// User specify period for one wire delay timer
// Valid value: [0,0xFF]
#define INIT_PERIOD_TIMER_DELAY                   (0xFFFFU)

#endif // #ifndef INIT_CFG_H

//****************************************** end of file *******************************************
