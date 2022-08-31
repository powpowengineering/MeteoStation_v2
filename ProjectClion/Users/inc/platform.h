// *****************************************************************************
// @Module  platform.h
// @Author  MSEpro Code Generator v.3.0.4916.32096 built at 17.06.2013 17:49:52
// @Description Contains platform-specific types definitions: e200z0
// *****************************************************************************

#ifndef PLATFORM_H
#define PLATFORM_H



// *****************************************************************************
// Processor architecture definitions
// *****************************************************************************

// Processor word size
#define CPU_WORD_SIZE  (32U)

// Processor byte order
#define CPU_FIRST_HIGH_BYTE  (0U) // Big-endian (motorola) format
#define CPU_FIRST_LOW_BYTE   (1U) // Little-endian (intel) format
#define CPU_BYTE_ORDER       (CPU_FIRST_LOW_BYTE)



// *****************************************************************************
// Data types definitions
// *****************************************************************************

typedef unsigned char   BOOLEAN;    // for using with TRUE/FALSE

typedef signed char     S8;         // 1 byte signed
typedef unsigned char   U8;         // 1 byte unsigned

typedef signed short    S16;        // 2 byte signed
typedef unsigned short  U16;        // 2 byte unsigned

typedef signed int      SWORD;      // 4 byte signed
typedef unsigned int    UWORD;      // 4 byte unsigned

typedef signed long     S32;        // 4 byte signed
typedef unsigned long   U32;        // 4 byte unsigned

typedef float           FLOAT32;    // 32 bit
typedef float           F32;        // 32 bit



#endif // PLATFORM_H

