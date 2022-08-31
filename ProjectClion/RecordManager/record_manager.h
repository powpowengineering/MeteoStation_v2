//**************************************************************************************************
// @Module        RECORD_MAN
// @Filename      record_manager.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the RECORD_MAN interface.
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef RECORD_MAN_H
#define RECORD_MAN_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Get HAL interface
#include "stm32l4xx_hal.h"

#include "compiler.h"

// Get general types
#include "general_types.h"

// Get module cfg
#include "record_manager_cfg.h"

// Get RTOS interface
#include "cmsis_os.h"



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// Max record package in bytes
#define RECORD_MAN_MAX_SIZE_RECORD                 (100U)

// Virtual address
#define RECORD_MAN_VIR_ADR32_LAST_RECORD             (0U)
#define RECORD_MAN_VIR_ADR32_NEXT_RECORD             (4U)

// Size record adr
#define RECORD_MAN_SIZE_VIR_ADR                      (4U)



//**************************************************************************************************
// Declarations of global (public) variables
//**************************************************************************************************

// Mutex to get resource
extern xSemaphoreHandle RECORD_MAN_xMutex;



//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

// Init record manager
extern void RECORD_MAN_Init(void);

// Store record
extern STD_RESULT RECORD_MAN_Store(const uint8_t *pData,
                                   uint32_t nDataQty,
                                    uint32_t* pQtyRecord);

// Load record
extern STD_RESULT RECORD_MAN_Load(uint32_t nNumberRecord,
                                  uint8_t *pRecord,
                                  uint32_t* nQtyBytes);

// Get number of records in flash
extern STD_RESULT RECORD_MAN_GetNumberOfRecords(uint32_t* nNumberOfRecords);

// Get available memory
extern STD_RESULT RECORD_MAN_GetAvailableMem(uint32_t *pQtyByte);



#endif // #ifndef RECORD_MAN_H



//****************************************** end of file *******************************************