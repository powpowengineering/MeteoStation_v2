//**************************************************************************************************
// @Filename      compiler.h
//--------------------------------------------------------------------------------------------------
// @Description   Contains compiler specific (non-ANSI) keywords.
//                Compiler: GCC C Compiler for ARM
//--------------------------------------------------------------------------------------------------
// @Version       2.1.0
//--------------------------------------------------------------------------------------------------
// @Date          05.12.2017
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// 06.06.2013     1.0.0    TVV         First release
// 14.10.2015     2.0.0    ASD         Total cleaning and refactoring
// 15.12.2016     2.0.1    POA         CODE_SEG constant has been added.
// 30.05.2017     2.0.2    SHA         INTERRUPT_ENABLE/DISABLE constants has been added.
// 29.06.2017     2.0.3    SHA         NULL_PTR_FUNC has been changed.
// 05.12.2017     2.1.0    SRM         A new memory placement macros.
//**************************************************************************************************

#ifndef COMPILER_H
#define COMPILER_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Include platform specific types
#include "platform.h"



//**************************************************************************************************
// Special compiler keywords
//**************************************************************************************************

// Null pointer
#define NULL_PTR_FUNC       ((void*)(0))

// Compiler keywords for alignment
#define PACKED              __attribute__ ((packed))
#define UNALIGNED           // TBD

// Compiler keywords for inlining functions bodies
#define INLINE_FUN          inline __attribute__((always_inline))
#define NOINLINE_FUN        // TBD

// Compiler keywords for programming interrupt routines
#define INTERRUPT           __attribute__ ((interrupt))

// Compiler keyword for placing an object at an absolute address (global variable address modifier)
// Usage example: unsigned char My_Array[3] AT_ADDR(0x0810C000) = {0x12, 0x15, 0x78};
#define AT_ADDR(address)    __attribute__ ((section (".address_"#address)))

// Pointer qualifiers
#define NEAR                // TBD
#define FAR                 // TBD
#define XHUGE               // TBD
#define DRV_MEM_NEAR        // TBD
#define MEM_TYPE_FAST       // TBD



//**************************************************************************************************
// Compiler keywords for the pragma preprocessing directives
//**************************************************************************************************

// Macro to threat pragma parameters as a string literal
#define PRAGMA(x)  _Pragma(#x)

// Pragma keywords for the segment naming
#define NEAR_SEG                                        near
#define FAR_SEG                                         huge
#define HUGE_SEG                                        huge
#define NOCLEAR_SEG                                     noclear
#define CODE_SEG                                        code



#define SEGMENT_BEGIN(type, modifier, name)                 SEGMENT_BEGIN_##modifier(type, modifier, name)
#define SEGMENT_BEGIN_NEAR_SEG(type, modifier, name)        __attribute__ ((section (#name)))
#define SEGMENT_BEGIN_FAR_SEG(type, modifier, name)         __attribute__ ((section (#name)))
#define SEGMENT_BEGIN_HUGE_SEG(type, modifier, name)        __attribute__ ((section (#name)))
#define SEGMENT_BEGIN_NOCLEAR_SEG(type, modifier, name)     __attribute__ ((section (".noinit")))
#define SEGMENT_BEGIN_RAM_DATA_SEG(type, modifier, name)    __attribute__ ((section (#name)))
#define SEGMENT_BEGIN_ROM_DATA_SEG(type, modifier, name)    __attribute__ ((section (#name)))
#define SEGMENT_BEGIN_RAM_CODE_SEG(type, modifier, name)    __attribute__ ((section (#name)))
#define SEGMENT_BEGIN_ROM_CODE_SEG(type, modifier, name)    __attribute__ ((section (#name)))



#define SEGMENT_END(type, modifier, name)                   SEGMENT_END_##modifier(type, modifier, name)
#define SEGMENT_END_NEAR_SEG(type, modifier, name)
#define SEGMENT_END_FAR_SEG(type, modifier, name)
#define SEGMENT_END_HUGE_SEG(type, modifier, name)
#define SEGMENT_END_NOCLEAR_SEG(type, modifier, name)
#define SEGMENT_END_RAM_DATA_SEG(type, modifier, name)
#define SEGMENT_END_ROM_DATA_SEG(type, modifier, name)
#define SEGMENT_END_RAM_CODE_SEG(type, modifier, name)
#define SEGMENT_END_ROM_CODE_SEG(type, modifier, name)



//**************************************************************************************************
// Compiler intrinsic (built-in) functions
//**************************************************************************************************

// Function-Like prototype: __near void* volatile GET_USER_SP(void);
// Returns the value of the user stack pointer
#define GET_USER_SP()                       // TBD

// Function-Like prototype: void volatile SET_USER_SP(__near void* value_ptr);
// Sets the value of the user stack pointer to value.
#define SET_USER_SP(value_ptr)              // TBD

// Function-Like prototype: __codeptr volatile GET_RETURN_ADDR(void);
// Returns the return address of a function
// (used by the compiler for profiling when you compile with the option --profile)
#define GET_RETURN_ADDR()                   // TBD

// Function-Like prototype: UWORD ROL(UWORD operand, UWORD count);
// Rotates "operand" left "count" times
#define ROL(operand, count)                 // TBD

// Function-Like prototype: UWORD ROR(UWORD operand, UWORD count);
// Rotates "operand" right "count" times
#define ROR(operand, count)                 // TBD

// Function-Like prototype: BIT TEST_CLEAR(BIT semaphore);
// Reads and clears "semaphore". Returns "0" if semaphore was not cleared, "1" otherwise
#define TEST_CLEAR(semaphore)               // TBD

// Function-Like prototype: BIT TEST_SET(BIT semaphore);
// Reads and sets "semaphore". Returns "0" if semaphore was not set, "1" otherwise
#define TEST_SET(semaphore)                 // TBD

// Function-Like prototype: void BIT_FLD(volatile UWORD UNALIGNED* operand_ptr, UWORD mask, UWORD value);
// Assigns the constant "value" to the bit-field indicated by the constant "mask" of the
// bit-addressable "operand"
#define BIT_FLD(operand_ptr, mask, value)   // TBD

// Function-Like prototype: BIT GET_BIT(operand, bit_offset);
// Returns the bit at "bit_offset" of the bit-addressable "operand" for usage in bit expressions
#define GET_BIT(operand, bit_offset)        ((operand) & (1U << (bit_offset)))

// Function-Like prototype: void PUT_BIT(BIT value, operand, bit_offset);
// Assigns "value" to the bit at "bit_offset" of the bit-addressable "operand"
#define PUT_BIT(value, operand, bit_offset) // TBD

// Function-Like prototype: void SW_INTERRUPT(int_no);
// Executes the C166/ST10 software interrupt specified by the interrupt number "int_no" via
// the software trap (TRAP) instruction
#define SW_INTERRUPT(int_no)                // TBD

// Function-Like prototype: void IDLE(void);
// Enters into the idle mode, when the CPU is powered down while the peripherals remain running
#define IDLE()                              // TBD

// Function-Like prototype: void __nop( void );
// Generates a NOP instruction
#define NOP()                               __asm__("nop");  // TBD

// Function-Like prototype: void __enwdt( void );
// Enables the watchdog timer
#define ENABLE_WDT()                        // TBD

// Function-Like prototype: void __diswdt( void );
// Disables the watchdog timer
#define DISABLE_WDT()                       // TBD

// Function-Like prototype: void __srvwdt( void );
// Services the watchdog timer
#define SERVICE_WDT()                       // TBD

// Function-Like prototype: void INTERRUPT_ENABLE(void);
//  Interrupt enable
#define INTERRUPT_ENABLE()                  __asm__("cpsie i");

// Function-Like prototype: void INTERRUPT_DISABLE(void);
//  Interrupt disable
#define INTERRUPT_DISABLE()                 __asm__("cpsid i");



// Include general definitions
#include "general.h"



#endif  // #ifndef COMPILER_H

//****************************************** end of file *******************************************
