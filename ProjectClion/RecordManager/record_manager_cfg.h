//**************************************************************************************************
// @Module        RECORD_MAN
// @Filename      record_manager_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the RECORD_MAN module.
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef RECORD_MAN_CFG_H
#define RECORD_MAN_CFG_H



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

// Specify size of record in bytes
#define RECORD_MAN_SIZE_OF_RECORD_BYTES         (4U * 7U)

// Specify storage mode of record
// valid value: RECORD_MAN_MODE_STORAGE_FIXED, RECORD_MAN_MODE_STORAGE_VARIABLE
#define RECORD_MAN_MODE_STORAGE                 (RECORD_MAN_MODE_STORAGE_FIXED)

#endif // #ifndef RECORD_MAN_CFG_H

//****************************************** end of file *******************************************
