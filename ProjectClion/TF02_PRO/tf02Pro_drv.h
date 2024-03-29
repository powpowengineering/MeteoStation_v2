//**************************************************************************************************
// @Module        TF02_PRO
// @Filename      tF02Pro_drv.h
//--------------------------------------------------------------------------------------------------
// @Description   Interface of the TF02_PRO interface.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef TF02_PRO_H
#define TF02_PRO_H


//**************************************************************************************************
// Project Includes
//**************************************************************************************************

#include "stm32l1xx.h"

#include "general_types.h"

#include "tf02Pro_drv_cfg.h"

//**************************************************************************************************
// Declarations of global (public) data types
//**************************************************************************************************

// Type of measured data.
typedef struct TF02_PRO_MeasuredData_str
{
    uint16_t nDistance;
    uint16_t nStrength;
    uint16_t nTemperature;
}TF02_PRO_MEASURED_DATA;

typedef enum LIDAR_FRAME_enum
{
    LIDAR_HEADER_LOW,
    LIDAR_HEADER_HIGH,
    LIDAR_DISTANCE_LOW,
    LIDAR_DISTANCE_HIGH,
    LIDAR_STRENGTH_LOW,
    LIDAR_STRENGTH_HIGH,
    LIDAR_TEMPERATURE_LOW,
    LIDAR_TEMPERATURE_HIGH,
    LIDAR_CHECKSUM
}LIDAR_FRAME;

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

extern STD_RESULT TF02_PRO_GetMeasuredData(TF02_PRO_MEASURED_DATA *const data);

#endif // #ifndef TF02_PRO

//****************************************** end of file *******************************************