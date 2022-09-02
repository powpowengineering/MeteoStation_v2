//**************************************************************************************************
// @Module        TASK_READ_SENSORS_CFG
// @Filename      task_read_sensors_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the TASK_READ_SENSORS module.
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef TASK_READ_SENSORS_CFG_H
#define TASK_READ_SENSORS_CFG_H



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// BPM280 device address
#define TASK_READ_SEN_BMP280_ADR                (0x76U)

// Prm vTaskSensorsRead
#define TASK_SEN_R_STACK_DEPTH          (256U)
#define TASK_SEN_R_PARAMETERS           (NULL)
#define TASK_SEN_R_PRIORITY             (1U)


#endif // #ifndef TASK_READ_SENSORS_CFG_H

//****************************************** end of file *******************************************
