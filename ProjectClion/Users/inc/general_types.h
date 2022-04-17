//**************************************************************************************************
// @Module        General types
// @Filename      general_types.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the General types module.
//--------------------------------------------------------------------------------------------------
// @Version       3.5.1
//--------------------------------------------------------------------------------------------------
// @Date          23.06.2021
//--------------------------------------------------------------------------------------------------

#ifndef GENERAL_TYPES_H
#define GENERAL_TYPES_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// True/False definition
#ifndef FALSE
#define FALSE               (0U)
#endif
#ifndef TRUE
#define TRUE                (1U)
#endif

// On/Off definition
#ifndef OFF
#define OFF                 (0U)
#endif
#ifndef ON
#define ON                  (1U)
#endif

// Logical signal state definitions
#ifndef LOGIC_0
#define LOGIC_0             (0U)
#endif
#ifndef LOGIC_1
#define LOGIC_1             (1U)
#endif


// Standard result type
typedef enum STD_RESULT_enum
{
    RESULT_OK     = (uint8_t)0U, // Function succeeded
    RESULT_NOT_OK = (uint8_t)1U  // Function not succeeded

} STD_RESULT;



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



#endif // #ifndef GENERAL_TYPES_H

//****************************************** end of file *******************************************
