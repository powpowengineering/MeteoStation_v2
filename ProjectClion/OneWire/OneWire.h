//**************************************************************************************************
// @Module        ONE_WIRE
// @Filename      ModuleRAK811.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the one wire interface.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef ONE_WIRE_H
#define ONE_WIRE_H


//**************************************************************************************************
// Project Includes
//**************************************************************************************************

#include "stm32l4xx_hal.h"

#include "compiler.h"

#include "general_types.h"

#include "OneWire_cfg.h"



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// Presence slaves
typedef enum
{
    // presence
    ONE_WIRE_PRESENCE=0,
    // not presence
    ONE_WIRE_NOT_PRESENCE
}enONE_WIRE_PRESENCE;


//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// None.


//**************************************************************************************************
// Declarations of global (public) variables
//**************************************************************************************************

// None.


//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

// Init one wire
extern void ONE_WIRE_init(void);
// Reset
extern STD_RESULT ONE_WIRE_reset(uint8_t nCh, enONE_WIRE_PRESENCE *const status);
// Read bit
extern STD_RESULT ONE_WIRE_readBit(uint8_t nCh, uint8_t *const bitVal);
// Write bit
extern STD_RESULT ONE_WIRE_writeBit(uint8_t nCh, uint8_t bitVal);
// Read byte
extern STD_RESULT ONE_WIRE_readByte(uint8_t nCh, uint8_t *const byteVal);
// Write byte
extern STD_RESULT ONE_WIRE_writeByte(uint8_t nCh, uint8_t byteVal);
// Delay in us
extern void ONE_WIRE_Delay(uint32_t us);

#endif // #ifndef ONE_WIRE_H

//****************************************** end of file *******************************************