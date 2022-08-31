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
// @History       Version  Author      Comment
// 16.12.2014     3.0.0    SRM         Third release.
// 13.01.2015     3.1.0    SRM         DEV_ERROR enumeration has been expanded.
// 03.03.2015     3.2.0    SRM         IO_STATUS enumeration has been added.
// 22.04.2015     3.3.0    SRM         RT_ERROR enumeration has been expanded.
// 02.06.2015     3.3.1    SHA         Added IO_STATUS_FAULT_UNKNOWN error to IO_STATUS.
// 18.05.2016     3.4.0    SRM         RT_ERROR_BUSY (runtime error type) constant has been added.
// 20.01.2020     3.5.0    SRM         Physical values types has been added.
// 23.06.2021     3.5.1    AVV         Added some more physical values types.
//**************************************************************************************************

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

// Development error definitions
typedef enum DEV_ERROR_enum
{
    DEV_ERROR_NOT_INIT      = (U8)0,   // Program module is not initialized
    DEV_ERROR_ALREADY_INIT  = (U8)1,   // Program module already initialized
    DEV_ERROR_PARAM_0       = (U8)2,   // Parameter number 0 is out of range
    DEV_ERROR_PARAM_1       = (U8)3,   // Parameter number 1 is out of range
    DEV_ERROR_PARAM_2       = (U8)4,   // Parameter number 2 is out of range
    DEV_ERROR_PARAM_3       = (U8)5,   // Parameter number 3 is out of range
    DEV_ERROR_PARAM_4       = (U8)6,   // Parameter number 4 is out of range
    DEV_ERROR_PARAM_5       = (U8)7,   // Parameter number 5 is out of range
    DEV_ERROR_PARAM_6       = (U8)8,   // Parameter number 6 is out of range
    DEV_ERROR_PARAM_7       = (U8)9,   // Parameter number 7 is out of range
    DEV_ERROR_PARAM_8       = (U8)10,  // Parameter number 8 is out of range
    DEV_ERROR_PARAM_9       = (U8)11,  // Parameter number 9 is out of range
    DEV_ERROR_FUNC_NOT_SUPP = (U8)12,  // Function is not supported

    DEV_ERROR_LAST          = (U8)128 // The last standard development error definition

} DEV_ERROR;

// Runtime error definitions
typedef enum RT_ERROR_enum
{
    RT_ERROR_NULL_PTR           = (U8)0,  // Null pointer
    RT_ERROR_RX_FIFO_OVERFLOW   = (U8)1,  // RX FIFO overflow attempt occurred
    RT_ERROR_TX_FIFO_OVERFLOW   = (U8)2,  // TX FIFO overflow attempt occurred
    RT_ERROR_RX_OVERRUN         = (U8)3,  // RX buffer overrun occurred
    RT_ERROR_TX_OVERRUN         = (U8)4,  // TX buffer overrun occurred
    RT_ERROR_BUSY               = (U8)5,  // Resource is busy

    RT_ERROR_LAST               = (U8)128 // The last standard runtime error definition

} RT_ERROR;

// Standard result type
typedef enum STD_RESULT_enum
{
    RESULT_OK     = (U8)0U, // Function succeeded
    RESULT_NOT_OK = (U8)1U  // Function not succeeded

} STD_RESULT;

// Board channel status definitions
typedef enum IO_STATUS_enum
{
    IO_STATUS_NOT_APPLICABLE     = (U8)0,    // Channel status is not applicable
    IO_STATUS_NO_FAILURE         = (U8)1,    // Channel state is OK
    IO_STATUS_FAULT_UNKNOWN      = (U8)2,    // Channel state is unknown fault
    IO_STATUS_SHRT_CIRC_BAT      = (U8)3,    // Short circuit to bat
    IO_STATUS_OPEN_LOAD          = (U8)4,    // Open load
    IO_STATUS_SHRT_CIRC_GND      = (U8)5,    // Short circuit to ground in on
    IO_STATUS_OVER_VOLTAGE       = (U8)6,    // Over voltage
    IO_STATUS_UNDER_VOLTAGE      = (U8)7,    // Under voltage
    IO_STATUS_OVER_CURRENT       = (U8)8,    // Over current
    IO_STATUS_UNDER_CURRENT      = (U8)9,    // Under current
    IO_STATUS_OVER_TEMPERATURE   = (U8)10,   // Over temperature

    IO_STATUS_LAST               = (U8)128   // The last status definitions

} IO_STATUS;



// Physical values types

// Voltage
// Resolution: 1  millivolts / bit,   Offset: 0 volts,   Range: 0 - 65.535 volts
typedef U16 U16_PHYS_VOLT_1MV;
// Resolution: 10 millivolts / bit,   Offset: 0 volts,   Range: 0 - 655.35 volts
typedef U16 U16_PHYS_VOLT_10MV;
// Resolution: 1  millivolts / bit,   Offset: 0 volts,   Range: -32.767 - 32.768 volts
typedef S16 S16_PHYS_VOLT_1MV;

// Amperage
// Resolution: 1  milliamps / bit,    Offset: 0 amps,   Range: 0 - 65.535 amps
typedef U16 U16_PHYS_AMP_1MA;
// Resolution: 10 milliamps / bit,    Offset: 0 amps,   Range: 0 - 655.35 amps
typedef U16 U16_PHYS_AMP_10MA;
// Resolution: 1  milliamps / bit,    Offset: 0 amps,   Range: -32.767 - 32.768 amps
typedef S16 S16_PHYS_AMP_1MA;

// Temperature
// Resolution: 1 degrees / bit,    Offset: 0 deg,   Range: 0 - 255 deg
typedef U8  U8_PHYS_TEMP_1DEG;
// Resolution: 1 degrees / bit,    Offset: 0 deg,   Range: -127 - 128 deg
typedef S8  S8_PHYS_TEMP_1DEG;
// Resolution: 1 degrees / bit,    Offset: 0 deg,   Range: -32767 - 32768 deg
typedef S16 S16_PHYS_TEMP_1DEG;
// Resolution: 0.1 degrees / bit,  Offset: 0 deg,   Range: -3276.7 - 3276.8 deg
typedef S16 S16_PHYS_TEMP_01DEG;
// Resolution: 1 degrees / bit,    Offset: 0 deg,   Range: 0 - 65535 deg
typedef U16 U16_PHYS_TEMP_1DEG;
// Resolution: 0.1 degrees / bit,  Offset: 0 deg,   Range: 0 - 6553.5 deg
typedef U16 U16_PHYS_TEMP_01DEG;

// Pressure
// Resolution: 10 kpa / bit,       Offset: 0 kpa,   Range: 0 - 2550 kpa
typedef U8 U8_PHYS_PRESS_10KPA;
// Resolution: 1 kpa / bit,        Offset: 0 kpa,   Range: 0 - 65535 kpa
typedef U16 U16_PHYS_PRESS_1KPA;
// Resolution: 0.1 kpa / bit,      Offset: 0 kpa,   Range: 0 - 6553.5 kpa
typedef U16 U16_PHYS_PRESS_01KPA;

// Signal strength
// Resolution: 1 dBm / bit,        Offset: 0 dBm,   Range: -127 - 128 dBm
typedef S8 S8_PHYS_SS_1DBM;
// Resolution: 0.1 dBm / bit,      Offset: 0 dBm,   Range: -3276.7 - 3276.8 dBm
typedef S16 S16_PHYS_SS_01DBM;




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
