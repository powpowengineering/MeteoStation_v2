//**************************************************************************************************
// @Module        EEPROM Emulation
// @Filename      eeprom_emulation.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the EEPROM Emulation module.
//--------------------------------------------------------------------------------------------------
// @Version       
//--------------------------------------------------------------------------------------------------
// @Date          
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment

//**************************************************************************************************

#ifndef EEPROM_EMULATION_H
#define EEPROM_EMULATION_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Get data types
#include "compiler.h"

// Get generic definitions
#include "general.h"

// Get configuration of the program module
#include "eeprom_emulation_cfg.h"

// Get memory types
#include "memory_types.h"



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// None.



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

// Initializes SW and HW resources of the program module
extern void EMEEP_Init(void);

// Deinitializes SW and HW resources of the program module
extern void EMEEP_DeInit(void);

// Sets "end of job" callback function pointer(s) for the specified job type
// (Load / Store)
extern STD_RESULT EMEEP_SetJobCallback(const U8 nJobType,
                                       const U8 nEventID,
                                       const MEM_END_OF_JOB_CALLBACK pCallback);

// Loads the last valid data image from the emulated EEPROM memory to the specified buffer
extern STD_RESULT EMEEP_Load(const U32 nSourceAddress,
                             U8* const pDataBuffer,
                             const U32 nDataQty);

// Stores a new data image to the emulated EEPROM memory from the specified buffer
extern STD_RESULT EMEEP_Store(const U32 nTargetAddress,
                              const U8* const pDataBuffer,
                              const U32 nDataQty);

// Returns the last emulated EEPROM job result
extern U8 EMEEP_GetJobResult(void);

// Returns memory status mask
extern U32 EMEEP_GetMemoryStatus(void);

// Sets a new memory status mask
extern void EMEEP_SetMemoryStatus(const U32 nStatusMask);



#endif // #ifndef EEPROM_EMULATION_H

//****************************************** end of file *******************************************
