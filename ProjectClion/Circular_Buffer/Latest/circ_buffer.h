//**************************************************************************************************
// @Module        Circular buffer
// @Filename      circ_buffer.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the Circular buffer module.
//--------------------------------------------------------------------------------------------------
// @Version       1.0.2
//--------------------------------------------------------------------------------------------------
// @Date          23.03.2016
//--------------------------------------------------------------------------------------------------
// @History       Version  Author    Comment
//                1.0.0    ASD       First release.
// 14.06.2011     1.0.1    ASD       Adapted for a lot of MISRA-C 2004 rules.
// 23.03.2016     1.0.2    ASD       Formatting improved.
//**************************************************************************************************

#ifndef CIRC_BUFFER_H
#define CIRC_BUFFER_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

#include "stm32l4xx_hal.h"

// Get data types
#include "compiler.h"

// Get generic definitions
#include "general.h"



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// Type of circular buffer size
typedef uint16_t CIRCBUF_SZ;

// Type of the structure of a circular buffer
typedef struct stCIRCBUF_struct
{
    // Pointer to data of the buffer
    const void* pData;
    // Item size in bytes
    uint8_t itemSize;
    // Size of the buffer in items
    CIRCBUF_SZ size;
    // Head of the buffer, where data is prepended
    CIRCBUF_SZ head;
    // Tail of the buffer, where data is removed from
    CIRCBUF_SZ tail;
} stCIRCBUF;

// Errors returned by circular buffer API
typedef enum
{
    // No error occurred
    CIRCBUF_NO_ERR,
    // Buffer is empty
    CIRCBUF_EMPTY,
    // Buffer overflow
    CIRCBUF_OVERFLOW,
    // Size of the buffer is too small
    CIRCBUF_SIZE_TOO_SMALL,
    // Buffer does not exist
    CIRCBUF_NOT_EXIST,
    // Data of the buffer do not exist
    CIRCBUF_DATA_NOT_EXIST
} enCIRCBUF_ERR;



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// None



//**************************************************************************************************
// Declarations of global (public) variables
//**************************************************************************************************

// None



//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

// Initializes a circular buffer
extern enCIRCBUF_ERR CIRCBUF_Init(stCIRCBUF* const pBuf,
                                  const void* const pData,
                                  const CIRCBUF_SZ size);

// Puts one item into a circular buffer
extern enCIRCBUF_ERR CIRCBUF_PutData(const void* const pData,
                                     stCIRCBUF* const pBuf);

// Gets and removes one item from a circular buffer
extern enCIRCBUF_ERR CIRCBUF_GetData(const void* const pData,
                                     stCIRCBUF* const pBuf);

// Cleans a circular buffer
extern enCIRCBUF_ERR CIRCBUF_Purge(stCIRCBUF* const pBuf);

// Calculates the number of items in a circular buffer
extern CIRCBUF_SZ CIRCBUF_GetNumberOfItems(const stCIRCBUF* const pBuf);

// Calculates the free number of elements in a circular buffer
extern CIRCBUF_SZ CIRCBUF_GetFreeSize(const stCIRCBUF* const pBuf);



#endif // #ifndef CIRC_BUFFER_H

//****************************************** end of file *******************************************
