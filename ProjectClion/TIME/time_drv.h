//**************************************************************************************************
// @Module        TIME
// @Filename      time_drv.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the MODULE module.
//                [Description...]
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************

#ifndef TIME_H
#define TIME_H



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Get HAL interface
#include "stm32l4xx_hal.h"

// Get data types
#include "compiler.h"

// Get generic definitions
#include "general.h"

// Get configuration of the program module
#include "time_drv_cfg.h"



//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

typedef struct TIME_type_str
{       /* date and time components */
    U8	tm_sec;
    U8	tm_min;
    U8	tm_hour;
    U8	tm_mday;
    U8	tm_mon;
    U16	tm_year;
}TIME_type;



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// Alarm name
#define TIME_ALARM_SENS                  (RTC_ALARM_A)
#define TIME_ALARM_GSM                   (RTC_ALARM_B)



//**************************************************************************************************
// Declarations of global (public) variables
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of global (public) functions
//**************************************************************************************************

// Init time
extern void TIME_Init(void);

// Get Unix timestamp
extern U32 TIME_GetUnixTimestamp(void);

// Set alarm
extern void TIME_SetAlarm(const TIME_type time, U32 nAlarmName);

// Set date
extern void TIME_SetDate(TIME_type time);

// Set time
extern void TIME_SetTime(TIME_type time);

// Check alarm
extern BOOLEAN TIME_CheckAlarm(U32 nAlarmName);

// Show current time
extern void TIME_TimeShow(void);



#endif // #ifndef TIME_H

//****************************************** end of file *******************************************
