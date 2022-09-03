//**************************************************************************************************
// @Module        TIME
// @Filename      time_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the MODULE module.
//                [Description...]
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************

#ifndef TIME_CFG_H
#define TIME_CFG_H



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// Configure RTC
#define TIME_RTC_ASYNCHPREDIV                   (127U)
#define TIME_RTC_SYNCHPREDIV                    (255U)
#define TIME_RTC_OUTPUT                         (RTC_OUTPUT_ALARMA)
#define TIME_RTC_OUTPUT_REMAP                   (RTC_OUTPUT_REMAP_NONE)
#define TIME_RTC_OUTPUT_POLARITY                (RTC_OUTPUT_POLARITY_HIGH)
#define TIME_RTC_OUTPUT_TYPE                    (RTC_OUTPUT_TYPE_PUSHPULL)
#define TIME_RTC_TIME_HOUR_DEF                  (12U)
#define TIME_RTC_TIME_MINUTES_DEF               (0U)
#define TIME_RTC_TIME_SECONDS_DEF               (0U)
#define TIME_RTC_TIMEFORMAT                     (RTC_HOURFORMAT12_AM)

// Alarm for sensors
#define TIME_ALARM_SENS_NUM                    (RTC_ALARM_A)
#define TIME_ALARM_SENS_HOURS                  (TIME_RTC_TIME_HOUR_DEF)
#define TIME_ALARM_SENS_MINUTES                (TIME_RTC_TIME_MINUTES_DEF)
#define TIME_ALARM_SENS_SECONDS                (TIME_RTC_TIME_SECONDS_DEF)
#define TIME_ALARM_SENS_MASK                   (RTC_ALARMMASK_MINUTES | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_DATEWEEKDAY)

// Alarm for GSM
#define TIME_ALARM_GSM_NUM                    (RTC_ALARM_B)
#define TIME_ALARM_GSM_HOURS                  (TIME_RTC_TIME_HOUR_DEF)
#define TIME_ALARM_GSM_MINUTES                (TIME_RTC_TIME_MINUTES_DEF)
#define TIME_ALARM_GSM_SECONDS                (TIME_RTC_TIME_SECONDS_DEF)
#define TIME_ALARM_GSM_MASK                   (RTC_ALARMMASK_HOURS | RTC_ALARMMASK_DATEWEEKDAY)



#endif // #ifndef TIME_CFG_H

//****************************************** end of file *******************************************
