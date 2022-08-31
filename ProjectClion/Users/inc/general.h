//**************************************************************************************************
// @Module        General definitions
// @Filename      general.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the General definitions module.
//--------------------------------------------------------------------------------------------------
// @Version       3.0.2
//--------------------------------------------------------------------------------------------------
// @Date          09.07.2015
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// 16.12.2014     3.0.0    SRM         Third release.
// 31.03.2015     3.0.1    POA         Redefine MEMTYPE as __near.
// 09.07.2015     3.0.2    ASD         Corrected "const" keyword usage.
// 02.11.2016     3.0.3    POA         U32 bitfield definition was deleted.
//**************************************************************************************************

#ifndef GENERAL_H
#define GENERAL_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Get data types
#include "compiler.h"

// Get generic types definitions
#include "general_types.h"

// Get generic macros definitions
#include "general_macros.h"



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// Structured WORD type
typedef volatile union STRUCTURED_WORD_union
{
    struct
    {
        U16 bit00 : 1;
        U16 bit01 : 1;
        U16 bit02 : 1;
        U16 bit03 : 1;
        U16 bit04 : 1;
        U16 bit05 : 1;
        U16 bit06 : 1;
        U16 bit07 : 1;
        U16 bit08 : 1;
        U16 bit09 : 1;
        U16 bit10 : 1;
        U16 bit11 : 1;
        U16 bit12 : 1;
        U16 bit13 : 1;
        U16 bit14 : 1;
        U16 bit15 : 1;
    } bit;
    struct
    {
        U8 byte0;
        U8 byte1;
    } byte;
    U16 word;
} STRUCTURED_WORD_type;

// Structured DWORD type
typedef volatile union STRUCTURED_DWORD_union
{
    struct
    {
        U8 byte0;
        U8 byte1;
        U8 byte2;
        U8 byte3;
    } byte;
    struct
    {
        U16 word0;
        U16 word1;
    } word;
    U32 doubleword;
} STRUCTURED_DWORD_type;

// Void function pointer
typedef void (*FAR _FUNC_)(void);

// Function pointer type to fill tables (etc.) with function pointers
typedef NEAR void (* ptrFUNCTION_NEAR)(void);
typedef FAR  void (* ptrFUNCTION_FAR) (void);



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// Null pointer
#define P_NULL      ((void*)(0))

// Memory type for global data
#define MEMTYPE     MEM_TYPE_FAST



//**************************************************************************************************
// Declarations of global (public) variables
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

#define API_CAN_Merge_Buf  API_Buffer_Merge16

extern U16 API_Buffer_Merge16(const U32 arrAddress,
                              const U16 plc,
                              const U32 insArrAddress,
                              const U16 insArrsize);

extern U8 API_Buffer_Merge_Inver(U32 arrAddress,
                                 U8 plc,
                                 U32 insArrAddress,
                                 U8 insArrsize);

extern U8 API_ReadDataByAddr(U32 arrAddress,
                             U8 plc,
                             U32 insArrAddress,
                             U16 insArrsize);



//**************************************************************************************************
// Declarations of interrupt service routines
//**************************************************************************************************

// None.



#endif  // #ifndef GENERAL_H

//****************************************** end of file *******************************************
