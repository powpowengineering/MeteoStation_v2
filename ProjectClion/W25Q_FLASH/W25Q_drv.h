//**************************************************************************************************
// @Module        W25Q
// @Filename      W25Q_drv.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the W25Q interface.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef W25Q_H
#define W25Q_H


//**************************************************************************************************
// Project Includes
//**************************************************************************************************

#include "stm32l4xx_hal.h"

#include "compiler.h"

#include "general_types.h"

#include "W25Q_drv_cfg.h"

//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// Type memory blocks
typedef enum W25Q_TYPE_BLOCKS_enum
{
    W25Q_BLOCK_MEMORY_4KB = 0,
    W25Q_BLOCK_MEMORY_32KB,
    W25Q_BLOCK_MEMORY_64KB,
    W25Q_BLOCK_MEMORY_ALL
}W25Q_TYPE_BLOCKS;


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

// Init W25Q interface
extern void W25Q_Init(void);

// Read unique ID
extern STD_RESULT W25Q_ReadUniqueID(uint64_t* ID);

// Read Manufacture ID
extern STD_RESULT W25Q_ReadManufactureID(uint16_t* ID);

// Read data
extern STD_RESULT W25Q_ReadData(const uint32_t adr,uint8_t* data, const uint32_t len);

// Write data
extern STD_RESULT W25Q_WriteData(uint32_t adr,uint8_t* data, uint32_t len);

// Erase block 4KB,32KB,64KB or all memory.
extern STD_RESULT W25Q_EraseBlock(const uint32_t adr, W25Q_TYPE_BLOCKS typeBlock);

// Get Block/Sector Lock
extern STD_RESULT W25Q_GetLock(const uint32_t adr, uint8_t *const lock);

// Global Block/Sector Unlock
extern STD_RESULT W25Q_UnLockGlobal(void);

// Detect blank memory
extern STD_RESULT W25Q_IsAreaBlank(const uint32_t nAddress, const uint32_t nSize, BOOLEAN *const bResult);

// Power down
extern STD_RESULT W25Q_PowerDown(void);



#endif // #ifndef W25Q_H

//****************************************** end of file *******************************************