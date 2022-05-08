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

// User specify port GPIOx
// Valid value: GPIOA,GPIOB,GPIOC
#define AM2305_GPIO_PORT                     (GPIOB)
// User specify pin
// Valid value: [0,15]
#define AM2305_PIN                           (12U)

// User specify pointer delay function
#define AM2305_Delay                         INIT_Delay


#endif // #ifndef AM2305_CFG_H

//****************************************** end of file *******************************************
