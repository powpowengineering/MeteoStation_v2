//**************************************************************************************************
// @Module        RECORD_MAN
// @Filename      record_manager_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the RECORD_MAN module.
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef RECORD_MAN_CFG_H
#define RECORD_MAN_CFG_H

//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

typedef struct RECORD_MAN_TYPE_RECORD_struct
{
    uint32_t nUnixTime;
    float fTemperature;
    float fHumidity;
    float fPressure;
    float fWindSpeed;
    float fBatteryVoltage;
}RECORD_MAN_TYPE_RECORD;



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// Specify sizeof item
#define RECORD_MAN_SIZEOF_ITEM                  (4U)

// Specify number of record's items
#define RECORD_MAN_NUM_ITEMS                    (6U)

// Specify size of record in bytes + checksum
#define RECORD_MAN_SIZE_OF_RECORD_BYTES         (RECORD_MAN_SIZEOF_ITEM * (RECORD_MAN_NUM_ITEMS + 1U))

// Specify storage mode of record
// valid value: RECORD_MAN_MODE_STORAGE_FIXED, RECORD_MAN_MODE_STORAGE_VARIABLE
#define RECORD_MAN_MODE_STORAGE                 (RECORD_MAN_MODE_STORAGE_FIXED)

#endif // #ifndef RECORD_MAN_CFG_H

//****************************************** end of file *******************************************
