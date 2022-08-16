//**************************************************************************************************
// @Module        ONE_WIRE
// @Filename      OneWire_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the ONE_WIRE module.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************

#ifndef ONE_WIRE_CFG_H
#define ONE_WIRE_CFG_H



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



// User specify port GPIOx
// Valid value: GPIOA,GPIOB,GPIOC
// Enable Ch0
#define ONE_WIRE_CH0_EN                         (ON)
#define ONE_WIRE_GPIO_PORT_CH0                  GPIOA
#define ONE_WIRE_PIN_CH0                        (GPIO_PIN_10)


#define ONE_WIRE_CH1_EN                         (OFF)
#define ONE_WIRE_GPIO_PORT_CH1                  (GPIOB)
#define ONE_WIRE_PIN_CH1                        (GPIO_PIN_3)

// User specify pointer delay function
#define ONE_WIRE_Delay                          (INIT_Delay)


#endif // #ifndef ONE_WIRE_CFG_H

//****************************************** end of file *******************************************
