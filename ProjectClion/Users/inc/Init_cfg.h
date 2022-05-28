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


// User specify Timer for one wire delay
#define INIT_TIMER_DELAY                          TIM6
// User specify prescaler for one wire delay timer
// Valid value: [0,0xFF]
#define INIT_PSC_TIMER_DELAY                      (31U)
// User specify period for one wire delay timer
// Valid value: [0,0xFF]
#define INIT_PERIOD_TIMER_DELAY                   (0xFFFFU)

#endif // #ifndef INIT_CFG_H

//****************************************** end of file *******************************************
