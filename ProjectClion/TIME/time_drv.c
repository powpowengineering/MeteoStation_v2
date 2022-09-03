//**************************************************************************************************
// @Module        TIME
// @Filename      time_drv.c
//--------------------------------------------------------------------------------------------------
// @Platform      PLATFORM_NAME
//--------------------------------------------------------------------------------------------------
// @Compatible    COMPATIBLE_PROCESSOR_MODULE
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the MODULE functionality.
//                [Description...]
//
//                Abbreviations:
//                  ABBR0 -
//                  ABBR1 -
//
//                Global (public) functions:
//                  MODULE_functionZero()
//                  MODULE_functionOne()
//
//                Local (private) functions:
//                  MODULE_functionTwo()
//                  MODULE_functionThree()
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Native header
#include "time_drv.h"

#include "Init.h"

#include "printf.h"

// Get hal_ll rtc
#include "stm32l4xx_ll_rtc.h"



//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of local (private) data types
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

#define _TBIAS_DAYS		((70 * (U32)365) + 17)
#define _TBIAS_SECS		(_TBIAS_DAYS * (U32)86400)
#define	_TBIAS_YEAR		1900
#define MONTAB(year)		((((year) & 03) || ((year) == 0)) ? mos : lmos)
#define	Daysto32(year, mon)	(((year - 1) / 4) + MONTAB(year)[mon])


const U16	lmos[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
const U16	mos[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};



//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// Current time
static RTC_TimeTypeDef sTimeCurrent;

// Current date
static RTC_DateTypeDef sDateCurrent;

// RTC handle
static RTC_HandleTypeDef RTC_Handle;



//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// None.



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      TIME_Init()
//--------------------------------------------------------------------------------------------------
// @Description   [description...]
//--------------------------------------------------------------------------------------------------
// @Notes
//--------------------------------------------------------------------------------------------------
// @ReturnValue   returnValue - [description...]
//--------------------------------------------------------------------------------------------------
// @Parameters    parameterZero - [description...]
//**************************************************************************************************
void TIME_Init(void)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    RTC_AlarmTypeDef sAlarm;

    RTC_Handle.Instance = RTC;

    // Check cause reset
    /* Check if the system was resumed not from Standby mode */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == RESET)
    {
        // Do delay to have ability connect debugger
        HAL_GPIO_WritePin(INIT_LED2_PORT, INIT_LED2_PIN, GPIO_PIN_SET);
        INIT_Delay(3000000);
        HAL_GPIO_WritePin(INIT_LED2_PORT, INIT_LED2_PIN, GPIO_PIN_RESET);

        // Config RTC
        RTC_Handle.Init.HourFormat = RTC_HOURFORMAT_12;
        RTC_Handle.Init.AsynchPrediv = TIME_RTC_ASYNCHPREDIV;
        RTC_Handle.Init.SynchPrediv = TIME_RTC_SYNCHPREDIV;
        RTC_Handle.Init.OutPut = TIME_RTC_OUTPUT;
        RTC_Handle.Init.OutPutRemap = TIME_RTC_OUTPUT_REMAP;
        RTC_Handle.Init.OutPutPolarity = TIME_RTC_OUTPUT_POLARITY;
        RTC_Handle.Init.OutPutType = TIME_RTC_OUTPUT_TYPE;
        HAL_RTC_Init(&RTC_Handle);

        // Set default date
        sDate.Date = 2;
        sDate.Month = RTC_MONTH_SEPTEMBER;
        sDate.WeekDay = RTC_WEEKDAY_FRIDAY;
        sDate.Year = 22;
        HAL_RTC_SetDate(&RTC_Handle, &sDate, RTC_FORMAT_BIN);

        // Set default time
        sTime.Hours = TIME_RTC_TIME_HOUR_DEF;
        sTime.Minutes = TIME_RTC_TIME_MINUTES_DEF;
        sTime.Seconds = TIME_RTC_TIME_SECONDS_DEF;
        sTime.TimeFormat = TIME_RTC_TIMEFORMAT;
        HAL_RTC_SetTime(&RTC_Handle, &sTime, RTC_FORMAT_BIN);

        // Set alarm for sensors
        sAlarm.AlarmTime.Hours = sTime.Hours + TIME_ALARM_SENS_HOURS;
        sAlarm.AlarmTime.Minutes = sTime.Minutes + TIME_ALARM_SENS_MINUTES;
        sAlarm.AlarmTime.Seconds = sTime.Seconds + TIME_ALARM_SENS_SECONDS;
        sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
        sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
        sAlarm.AlarmMask = TIME_ALARM_SENS_MASK;
        sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
        sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
        sAlarm.AlarmDateWeekDay = 1;
        sAlarm.Alarm = TIME_ALARM_SENS_NUM;
        HAL_RTC_SetAlarm_IT(&RTC_Handle, &sAlarm, FORMAT_BIN);

        // Set alarm fo GSM
        sAlarm.AlarmTime.Hours = sTime.Hours + TIME_ALARM_GSM_HOURS;
        sAlarm.AlarmTime.Minutes = sTime.Minutes + TIME_ALARM_GSM_MINUTES;
        sAlarm.AlarmTime.Seconds = sTime.Seconds + TIME_ALARM_GSM_SECONDS;
        sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
        sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
        sAlarm.AlarmMask = TIME_ALARM_GSM_MASK;
        sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
        sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
        sAlarm.AlarmDateWeekDay = 1;
        sAlarm.Alarm = TIME_ALARM_GSM_NUM;
        HAL_RTC_SetAlarm_IT(&RTC_Handle, &sAlarm, FORMAT_BIN);

        __HAL_RTC_WRITEPROTECTION_DISABLE(&RTC_Handle);
        LL_RTC_DisableIT_TAMP(RTC_Handle.Instance);
        LL_RTC_DisableIT_TAMP1(RTC_Handle.Instance);
        LL_RTC_DisableIT_TAMP2(RTC_Handle.Instance);
        LL_RTC_DisableIT_TAMP3(RTC_Handle.Instance);
        LL_RTC_DisableIT_TS(RTC_Handle.Instance);
        LL_RTC_DisableIT_WUT(RTC_Handle.Instance);
        __HAL_RTC_WRITEPROTECTION_ENABLE(&RTC_Handle);
    }
} // end of TIME_Init()



//**************************************************************************************************
// @Function      HAL_RTC_MspInit()
//--------------------------------------------------------------------------------------------------
// @Description   [description...]
//--------------------------------------------------------------------------------------------------
// @Notes
//--------------------------------------------------------------------------------------------------
// @ReturnValue   returnValue - [description...]
//--------------------------------------------------------------------------------------------------
// @Parameters    parameterZero - [description...]
//**************************************************************************************************
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /*##-1- Enables the PWR Clock and Enables access to the backup domain ######*/
    /* To enable access on RTC registers */
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();


    /*##-2- Configure LSE/LSI as RTC clock source ###############################*/
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        //Error_Handler();
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        //Error_Handler();
    }
    /* Configures the External Low Speed oscillator (LSE) drive capability */
//    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);

    /*##-2- Enable the RTC & BKP peripheral Clock ##############################*/
    /* Enable RTC Clock */
    __HAL_RCC_RTC_ENABLE();

    /* To disable access on RTC registers */
//    HAL_PWR_DisableBkUpAccess();

} // end of HAL_RTC_MspInit()



//**************************************************************************************************
// @Function      MODULE_FunctionZero()
//--------------------------------------------------------------------------------------------------
// @Description   [description...]
//--------------------------------------------------------------------------------------------------
// @Notes
//--------------------------------------------------------------------------------------------------
// @ReturnValue   returnValue - [description...]
//--------------------------------------------------------------------------------------------------
// @Parameters    parameterZero - [description...]
//**************************************************************************************************
U32 TIME_GetUnixTimestamp(void)
{
    // Get current date
    HAL_RTC_GetDate(&RTC_Handle, &sDateCurrent, RTC_FORMAT_BIN);

    // Get current time
    HAL_RTC_GetTime(&RTC_Handle, &sTimeCurrent, RTC_FORMAT_BIN);

    // convert time structure to scalar time
    U32 days;
    U32 secs;
    U32 mon, year;

    /* Calculate number of days. */
    mon = sDateCurrent.Month - 1;
    year = sDateCurrent.Year - _TBIAS_YEAR;
    days = Daysto32(year, mon) - 1;
    days += 365 * year;
    days += sDateCurrent.Date;
    days -= _TBIAS_DAYS;

    // Calculate number of seconds.
    secs = 3600 * sTimeCurrent.Hours;
    secs += 60 * sTimeCurrent.Minutes;
    secs += sTimeCurrent.Seconds;

    secs += (days * (U32) 86400);

    return (secs);
} // end of TIME_GetUnixTimestamp()



//**************************************************************************************************
// @Function      TIME_SetAlarm()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void TIME_SetAlarm(const TIME_type time, uint32_t nAlarmName)
{
    RTC_TimeTypeDef sTimeCurrent;
    RTC_AlarmTypeDef sAlarm;

    /*##-1- Enables the PWR Clock and Enables access to the backup domain ######*/
    /* To enable access on RTC registers */
    HAL_PWR_EnableBkUpAccess();

    /* Get the RTC current Time */
    HAL_RTC_GetTime(&RTC_Handle, &sTimeCurrent, RTC_FORMAT_BIN);

    // Calculate next time alarm
    sAlarm.AlarmTime.Seconds = (sTimeCurrent.Seconds + time.tm_sec) % 59U;
    sAlarm.AlarmTime.Minutes = (sTimeCurrent.Minutes + time.tm_min) % 59U;
    sAlarm.AlarmTime.Hours = (sTimeCurrent.Hours + time.tm_hour) % 23U;

    sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;

    if (TIME_ALARM_SENS == nAlarmName)
    {
        sAlarm.AlarmMask = TIME_ALARM_SENS_MASK;
    }
    else if (TIME_ALARM_GSM == nAlarmName)
    {
        sAlarm.AlarmMask = TIME_ALARM_GSM_MASK;
    }

    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 1;
    sAlarm.Alarm = nAlarmName;

    HAL_RTC_SetAlarm_IT(&RTC_Handle, &sAlarm, FORMAT_BIN);

    __HAL_RTC_WRITEPROTECTION_DISABLE(&RTC_Handle);
    LL_RTC_DisableIT_TAMP(RTC_Handle.Instance);
    LL_RTC_DisableIT_TAMP1(RTC_Handle.Instance);
    LL_RTC_DisableIT_TAMP2(RTC_Handle.Instance);
    LL_RTC_DisableIT_TAMP3(RTC_Handle.Instance);
    LL_RTC_DisableIT_TS(RTC_Handle.Instance);
    LL_RTC_DisableIT_WUT(RTC_Handle.Instance);
    __HAL_RTC_WRITEPROTECTION_ENABLE(&RTC_Handle);

    /* To disable access on RTC registers */
    HAL_PWR_DisableBkUpAccess();

} // end of TIME_SetAlarm()



//**************************************************************************************************
// @Function      TIME_CheckAlarm()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
BOOLEAN TIME_CheckAlarm(U32 nAlarmName)
{
    BOOLEAN bResult = FALSE;

    if (TIME_ALARM_SENS == nAlarmName)
    {
        bResult = LL_RTC_IsActiveFlag_ALRA(RTC_Handle.Instance);
    }
    else if (TIME_ALARM_GSM == nAlarmName)
    {
        bResult = LL_RTC_IsActiveFlag_ALRB(RTC_Handle.Instance);
    }

    return bResult;
} // end of TIME_CheckAlarm()



//**************************************************************************************************
// @Function      TIME_TimeShow()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void TIME_TimeShow(void)
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;

    // Get the RTC current Time
    HAL_RTC_GetTime(&RTC_Handle, &stimestructureget, RTC_FORMAT_BIN);

    // Get the RTC current Date
    HAL_RTC_GetDate(&RTC_Handle, &sdatestructureget, RTC_FORMAT_BIN);

    // Display time Format : dd:mm:yy | hh:mm:ss
    printf("%02d:%02d:%02d | ",sdatestructureget.Date, sdatestructureget.Month, sdatestructureget.Year);
    printf("%02d:%02d:%02d\r\n",stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);

} // end of TIME_TimeShow()



//**************************************************************************************************
// @Function      TIME_SetDate()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void TIME_SetDate(TIME_type time)
{

    RTC_DateTypeDef sDate;

    /*##-1- Enables the PWR Clock and Enables access to the backup domain ######*/
    /* To enable access on RTC registers */
    HAL_PWR_EnableBkUpAccess();

    // Set default date
    sDate.Date = time.tm_mday;
    sDate.Month = time.tm_mon;
    sDate.Year = time.tm_year;
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    HAL_RTC_SetDate(&RTC_Handle, &sDate, RTC_FORMAT_BIN);

    // Disable BKUP access
    HAL_PWR_DisableBkUpAccess();
} // end of TIME_SetDate()



//**************************************************************************************************
// @Function      TIME_SetDate()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void TIME_SetTime(TIME_type time)
{
    RTC_TimeTypeDef sTime;

    /*##-1- Enables the PWR Clock and Enables access to the backup domain ######*/
    /* To enable access on RTC registers */
    HAL_PWR_EnableBkUpAccess();

    // Set default date
    sTime.Hours = time.tm_hour;
    sTime.Minutes = time.tm_min;
    sTime.Seconds = time.tm_sec;
    HAL_RTC_SetTime(&RTC_Handle, &sTime, RTC_FORMAT_BIN);

    HAL_RTC_DST_Sub1Hour(&RTC_Handle);

    // Disable BKUP access
    HAL_PWR_DisableBkUpAccess();
} // end of TIME_SetDate()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

// None.



//****************************************** end of file *******************************************
