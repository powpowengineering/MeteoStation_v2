//**************************************************************************************************
// @Module        DS18B20
// @Filename      ds18b20_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the DS18B20 module.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************

#ifndef DS18B20_CFG_H
#define DS18B20_CFG_H

#include "Init.h"

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

// User specify pointer delay function
#define DS18B20_Delay                           (INIT_Delay)



#endif // #ifndef DS18B20_CFG_H

//****************************************** end of file *******************************************
