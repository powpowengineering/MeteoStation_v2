//**************************************************************************************************
// @Module        AM2305
// @Filename      am2305_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the AM2305 module.
//                
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef AM2305_CFG_H
#define AM2305_CFG_H



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

// The user specify port GPIOx with Tim channel
#define AM2305_GPIO_PORT                     GPIOB
// User specify pin with Tim channel
#define AM2305_PIN                           (14U)
// Specify PinSource
#define AM2305_PinSource                     GPIO_PinSource14
// Specify GPIO_AF
#define AM2305_GPIO_AF                       GPIO_AF_TIM9


// The user specifies a timer with a capture function
#define AM2305_TIMER                               TIM9
// User specify prescaler
#define AM2305_TIMER_PSC                          (31U)
// User specify period for one wire delay timer
// Valid value: [0,0xFFFF]
#define AM2305_TIMER_PERIOD                       (0xFFFFU)


#endif // #ifndef AM2305_CFG_H

//****************************************** end of file *******************************************
