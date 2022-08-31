//**************************************************************************************************
// @Module        General macros
// @Filename      general_macros.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the General macros module.
//--------------------------------------------------------------------------------------------------
// @Version       3.2.1
//--------------------------------------------------------------------------------------------------
// @Date          20.10.2016
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// 16.12.2014     3.0.0    SRM         Third release.
// 11.02.2014     3.0.1    POA         CONCATENATE macros was added.
// 02.12.2015     3.1.0    SRM         Find MIN/MAX value macros has been added.
// 22.07.2016     3.2.0    SIV         Added bit quantity macroses.
// 20.10.2016     3.2.1    SHA         Added ConvertToPtr_S16bit() and ConvertToPtr_S32bit
//**************************************************************************************************

#ifndef GENERAL_MACROS_H
#define GENERAL_MACROS_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// Maximum possible values each of the C data type
#define U8_MAX_VALUE            (0xFFU)
#define U16_MAX_VALUE           (0xFFFFU)
#define U32_MAX_VALUE           (0xFFFFFFFFUL)
#define U64_MAX_VALUE           (0xFFFFFFFFFFFFFFFFULL)

// Number of bits in each of the C data type
#define U8_BIT_QTY              (8U)
#define U16_BIT_QTY             (16U)
#define U32_BIT_QTY             (32U)
#define U64_BIT_QTY             (64U)

// Null pointer
#define NULL_PTR                (0U)

// Returns size of the specified array in elements
#define SIZE_OF_ARRAY(array)    (sizeof(array) / sizeof((array)[0U]))

// Concatenation of two macros
#define PASTER(x, y)            x ## y
#define CONCATENATE(x, y)       PASTER(x, y)

// Byte order definition
#define FIRST_HIGH_BYTE         (0U) // Big-endian
#define FIRST_LOW_BYTE          (1U) // Little-endian

// The function doing nothing definition
#define DoNothing()

// Convert input param "addr", that contains address, to arithmetic pointer
#define ConvertToPtr_8bit(addr)                 ((U8*)(addr))
#define ConvertToPtr_S8bit(addr)                ((S8*)(addr))
#define ConvertToPtr_16bit(addr)                ((U16*)(addr))
#define ConvertToPtr_S16bit(addr)               ((S16*)(addr))
#define ConvertToPtr_32bit(addr)                ((U32*)(addr))
#define ConvertToPtr_S32bit(addr)               ((S32*)(addr))

// These macros returns value from the specified address
#define GetDataFromAddress_8bit(Address,  Offset)   ((U8)*((U8* FAR)  ((U32)(Address) + (Offset))))
#define GetDataFromAddress_16bit(Address, Offset)   ((U16)*((U16* FAR)((U32)(Address) + (Offset))))
#define GetDataFromAddress_32bit(Address, Offset)   ((U32)*((U32* FAR)((U32)(Address) + (Offset))))

// NIBBLE <-> BYTE <-> WORD <-> DWORD <-> BYTE <-> NIBBLE macro conversion support
#define MAKEBYTE(lowNibble, highNibble) (((U8)(lowNibble) & 0x0FU) | (((U8)(highNibble) & 0x0FU) << 4U))
#define MAKEWORD(lowByte, highByte)     ((U16)(((U8)(lowByte)) | ((U16)((U8)(highByte))) << U8_BIT_QTY))
#define MAKELONG(lowWord, highWord)     ((U32)(((U16)((U16)(lowWord) & 0xFFFFU)) | ((U32)((U16)((U16)(highWord) & 0xFFFFU))) << U16_BIT_QTY))
#define LONIBBLE(b) (((U8)(b)) & 0x0FU)
#define HINIBBLE(b) ((((U8)(b)) & 0xF0U) >> 4U)
#define LOBYTE(w)   ((U8)((U16)(w) & 0x00FFU))
#define HIBYTE(w)   ((U8)((((U16)(w)) >> U8_BIT_QTY) & (U16)0x00FFU))
#define LOWORD(l)   ((U16)((U32)(l) & 0xFFFFU))
#define HIWORD(l)   ((U16)(((U32)(l) >> U16_BIT_QTY) & 0xFFFFU))

// Check, whether value is in the specified range.
// This inequality is UNstrict,
// i.e. LOW and HIGH values are possible (in the specified range).
#define IsValueInRange(LOW, Value, HIGH)     ((((LOW)  <   (Value))    \
                                            || ((LOW)  ==  (Value)))   \
                                            && ((HIGH) >=  (Value)))

// Converts (MHz - frequency) into (Hz - frequency)
#define MHZ_TO_HZ(MHz)        ((MHz) * (1000000UL))
#define HZ_TO_MHZ(Hz)         ((Hz)  / (1000000UL))

// Integer rounding support macro
#define ROUND_TO(nValue, nRoundTo)          (((nValue) / (nRoundTo)) * (nRoundTo))

// ASCII conversion support
// Convert HEX value to ASCII symbol: HEX = {0x0..0xF} -> {'0'..'F'}
#define BIN_TO_ASCII(digit)     (((digit) > 9U) ? ((digit) + 0x37U) : ((digit) + 0x30U))
// Convert ASCII symbol to its HEX value: CHAR = {'0'..'F'} -> {0x0..0xF}
#define ASCII_TO_BIN(symbol)    (((symbol) >= 0x41U) ? ((symbol) - 0x37U) : ((symbol) - 0x30U))

// Find maximum value macro
#define MAX(valueA, valueB)     (((valueA) > (valueB)) ? (valueA) : (valueB))
// Find minimum value macro
#define MIN(valueA, valueB)     (((valueA) < (valueB)) ? (valueA) : (valueB))



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

// None.



//**************************************************************************************************
// Declarations of interrupt service routines
//**************************************************************************************************

// None.



#endif // #ifndef GENERAL_MACROS_H

//****************************************** end of file *******************************************
