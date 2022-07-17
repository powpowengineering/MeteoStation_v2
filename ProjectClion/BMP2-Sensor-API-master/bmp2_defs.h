/**
* Copyright (c) 2021 Bosch Sensortec GmbH. All rights reserved.
*
* BSD-3-Clause
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* @file       bmp2_defs.h
* @date       2021-05-21
* @version    v1.0.1
*
*/

/*! @file bmp2_defs.h
 * @brief Sensor driver for BMP2 sensor
 */

#ifndef _BMP2_DEFS_H
#define _BMP2_DEFS_H

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*! @name       Header includes             */
/****************************************************************/
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/kernel.h>
#else
#include <stdint.h>
#include <stddef.h>
#endif

/****************************************************************/
/*! @name       Common macros               */
/****************************************************************/
#ifdef __KERNEL__
#if !defined(UINT8_C) && !defined(INT8_C)
#define INT8_C(x)    S8_C(x)
#define UINT8_C(x)   U8_C(x)
#endif

#if !defined(UINT16_C) && !defined(INT16_C)
#define INT16_C(x)   S16_C(x)
#define UINT16_C(x)  U16_C(x)
#endif

#if !defined(INT32_C) && !defined(UINT32_C)
#define INT32_C(x)   S32_C(x)
#define UINT32_C(x)  U32_C(x)
#endif

#if !defined(INT64_C) && !defined(UINT64_C)
#define INT64_C(x)   S64_C(x)
#define UINT64_C(x)  U64_C(x)
#endif
#endif

/*! @name C standard macros */
#ifndef NULL
#ifdef __cplusplus
#define NULL         0
#else
#define NULL         ((void *) 0)
#endif
#endif

/******************************************************************************/
/*! @name        Compiler switch macros Definitions                */
/******************************************************************************/
#ifndef BMP2_64BIT_COMPENSATION /*< Check if 64bit (using BMP2_64BIT_COMPENSATION) is enabled */
#ifndef BMP2_32BIT_COMPENSATION /*< Check if 32bit (using BMP2_32BIT_COMPENSATION) is enabled */
#ifndef BMP2_DOUBLE_COMPENSATION /*< If any of the integer data types not enabled then enable
                                    * BMP2_DOUBLE_COMPENSATION */
#define BMP2_DOUBLE_COMPENSATION
#endif
#endif
#endif

/******************************************************************************/
/*! @name        General Macro Definitions                */
/******************************************************************************/

/*! @name Maximum write length to registers */
#define BMP2_MAX_LEN                                  UINT8_C(4)

/*! @name Return codes */
/*! @name Success code*/
#define BMP2_OK                                       INT8_C(0)

/*! @name Error codes */
#define BMP2_E_NULL_PTR                               INT8_C(-1)
#define BMP2_E_COM_FAIL                               INT8_C(-2)
#define BMP2_E_INVALID_LEN                            INT8_C(-3)
#define BMP2_E_DEV_NOT_FOUND                          INT8_C(-4)
#define BMP2_E_UNCOMP_TEMP_RANGE                      INT8_C(-5)
#define BMP2_E_UNCOMP_PRESS_RANGE                     INT8_C(-6)
#define BMP2_E_UNCOMP_TEMP_AND_PRESS_RANGE            INT8_C(-7)

/*! @name Warning codes */
#define BMP2_W_MIN_TEMP                               INT8_C(1)
#define BMP2_W_MAX_TEMP                               INT8_C(2)
#define BMP2_W_MIN_PRES                               INT8_C(3)
#define BMP2_W_MAX_PRES                               INT8_C(4)

/*! @name Chip ID */
#define BMP2_CHIP_ID                                  UINT8_C(0x58)

/*! @name I2C addresses */
#define BMP2_I2C_ADDR_PRIM                            UINT8_C(0x76)
#define BMP2_I2C_ADDR_SEC                             UINT8_C(0x77)

/*! @name Register addresses */
#define BMP2_REG_CHIP_ID                              UINT8_C(0xD0)
#define BMP2_REG_SOFT_RESET                           UINT8_C(0xE0)
#define BMP2_REG_STATUS                               UINT8_C(0xF3)
#define BMP2_REG_CTRL_MEAS                            UINT8_C(0xF4)
#define BMP2_REG_CONFIG                               UINT8_C(0xF5)
#define BMP2_REG_PRES_MSB                             UINT8_C(0xF7)
#define BMP2_REG_PRES_LSB                             UINT8_C(0xF8)
#define BMP2_REG_PRES_XLSB                            UINT8_C(0xF9)
#define BMP2_REG_TEMP_MSB                             UINT8_C(0xFA)
#define BMP2_REG_TEMP_LSB                             UINT8_C(0xFB)
#define BMP2_REG_TEMP_XLSB                            UINT8_C(0xFC)

/* Calibration parameter register addresses */
#define BMP2_REG_DIG_T1_LSB                           UINT8_C(0x88)
#define BMP2_REG_DIG_T1_MSB                           UINT8_C(0x89)
#define BMP2_REG_DIG_T2_LSB                           UINT8_C(0x8A)
#define BMP2_REG_DIG_T2_MSB                           UINT8_C(0x8B)
#define BMP2_REG_DIG_T3_LSB                           UINT8_C(0x8C)
#define BMP2_REG_DIG_T3_MSB                           UINT8_C(0x8D)
#define BMP2_REG_DIG_P1_LSB                           UINT8_C(0x8E)
#define BMP2_REG_DIG_P1_MSB                           UINT8_C(0x8F)
#define BMP2_REG_DIG_P2_LSB                           UINT8_C(0x90)
#define BMP2_REG_DIG_P2_MSB                           UINT8_C(0x91)
#define BMP2_REG_DIG_P3_LSB                           UINT8_C(0x92)
#define BMP2_REG_DIG_P3_MSB                           UINT8_C(0x93)
#define BMP2_REG_DIG_P4_LSB                           UINT8_C(0x94)
#define BMP2_REG_DIG_P4_MSB                           UINT8_C(0x95)
#define BMP2_REG_DIG_P5_LSB                           UINT8_C(0x96)
#define BMP2_REG_DIG_P5_MSB                           UINT8_C(0x97)
#define BMP2_REG_DIG_P6_LSB                           UINT8_C(0x98)
#define BMP2_REG_DIG_P6_MSB                           UINT8_C(0x99)
#define BMP2_REG_DIG_P7_LSB                           UINT8_C(0x9A)
#define BMP2_REG_DIG_P7_MSB                           UINT8_C(0x9B)
#define BMP2_REG_DIG_P8_LSB                           UINT8_C(0x9C)
#define BMP2_REG_DIG_P8_MSB                           UINT8_C(0x9D)
#define BMP2_REG_DIG_P9_LSB                           UINT8_C(0x9E)
#define BMP2_REG_DIG_P9_MSB                           UINT8_C(0x9F)

/*! @name Interface settings */
#define BMP2_SPI_RD_MASK                              UINT8_C(0x80)
#define BMP2_SPI_WR_MASK                              UINT8_C(0x7F)

/*! @name Delay definition */
#define BMP2_DELAY_US_STARTUP_TIME                    UINT8_C(2000)

/*! @name Power modes */
#define BMP2_POWERMODE_SLEEP                          UINT8_C(0x00)
#define BMP2_POWERMODE_FORCED                         UINT8_C(0x01)
#define BMP2_POWERMODE_NORMAL                         UINT8_C(0x03)

#define BMP2_POWERMODE_POS                            UINT8_C(0x00)
#define BMP2_POWERMODE_MSK                            UINT8_C(0x03)

/*! @name Soft reset command */
#define BMP2_SOFT_RESET_CMD                           UINT8_C(0xB6)

/*! @name Data length */
#define BMP2_P_T_LEN                                  UINT8_C(0X06)

/*! @name Temperature range values in integer(32bit, 64bit) and float */
#define BMP2_MIN_TEMP_INT                             INT32_C(-4000)
#define BMP2_MAX_TEMP_INT                             INT32_C(8500)
#define BMP2_MIN_TEMP_DOUBLE                          -40.0f
#define BMP2_MAX_TEMP_DOUBLE                          85.0f

/*! @name Pressure range values in integer and float */
#define BMP2_MIN_PRES_32INT                           UINT32_C(30000)
#define BMP2_MAX_PRES_32INT                           UINT32_C(110000)
#define BMP2_MIN_PRES_64INT                           UINT32_C(30000 * 256)
#define BMP2_MAX_PRES_64INT                           UINT32_C(110000 * 256)
#define BMP2_MIN_PRES_DOUBLE                          30000.0f
#define BMP2_MAX_PRES_DOUBLE                          110000.0f

/*! @name ODR options */
#define BMP2_ODR_0_5_MS                               UINT8_C(0x00)
#define BMP2_ODR_62_5_MS                              UINT8_C(0x01)
#define BMP2_ODR_125_MS                               UINT8_C(0x02)
#define BMP2_ODR_250_MS                               UINT8_C(0x03)
#define BMP2_ODR_500_MS                               UINT8_C(0x04)
#define BMP2_ODR_1000_MS                              UINT8_C(0x05)
#define BMP2_ODR_2000_MS                              UINT8_C(0x06)
#define BMP2_ODR_4000_MS                              UINT8_C(0x07)

/*! @name Over-sampling macros */
#define BMP2_OS_NONE                                  UINT8_C(0x00)
#define BMP2_OS_1X                                    UINT8_C(0x01)
#define BMP2_OS_2X                                    UINT8_C(0x02)
#define BMP2_OS_4X                                    UINT8_C(0x03)
#define BMP2_OS_8X                                    UINT8_C(0x04)
#define BMP2_OS_16X                                   UINT8_C(0x05)
#define BMP2_OS_TEMP_POS                              UINT8_C(0x05)
#define BMP2_OS_TEMP_MSK                              UINT8_C(0xE0)
#define BMP2_OS_PRES_POS                              UINT8_C(0x02)
#define BMP2_OS_PRES_MSK                              UINT8_C(0x1C)

/*! @name Over-sampling mode */
#define BMP2_OS_MODE_ULTRA_LOW_POWER                  UINT8_C(0x00)
#define BMP2_OS_MODE_LOW_POWER                        UINT8_C(0x01)
#define BMP2_OS_MODE_STANDARD_RESOLUTION              UINT8_C(0x02)
#define BMP2_OS_MODE_HIGH_RESOLUTION                  UINT8_C(0x03)
#define BMP2_OS_MODE_ULTRA_HIGH_RESOLUTION            UINT8_C(0x04)

/*! @name Filter coefficient macros */
#define BMP2_FILTER_OFF                               UINT8_C(0x00)
#define BMP2_FILTER_COEFF_2                           UINT8_C(0x01)
#define BMP2_FILTER_COEFF_4                           UINT8_C(0x02)
#define BMP2_FILTER_COEFF_8                           UINT8_C(0x03)
#define BMP2_FILTER_COEFF_16                          UINT8_C(0x04)

#define BMP2_FILTER_POS                               UINT8_C(0x02)
#define BMP2_FILTER_MSK                               UINT8_C(0x1C)

/*! @name SPI 3-Wire macros */
#define BMP2_SPI3_WIRE_ENABLE                         UINT8_C(0x01)
#define BMP2_SPI3_WIRE_DISABLE                        UINT8_C(0x00)

#define BMP2_SPI3_ENABLE_POS                          UINT8_C(0x00)
#define BMP2_SPI3_ENABLE_MSK                          UINT8_C(0x01)

/*! @name Measurement status macros */
#define BMP2_MEAS_DONE                                UINT8_C(0x00)
#define BMP2_MEAS_ONGOING                             UINT8_C(0x01)

#define BMP2_STATUS_MEAS_POS                          UINT8_C(0x03)
#define BMP2_STATUS_MEAS_MSK                          UINT8_C(0x08)

/*! @name Image update status macros */
#define BMP2_IM_UPDATE_DONE                           UINT8_C(0x00)
#define BMP2_IM_UPDATE_ONGOING                        UINT8_C(0x01)

#define BMP2_STATUS_IM_UPDATE_POS                     UINT8_C(0x00)
#define BMP2_STATUS_IM_UPDATE_MSK                     UINT8_C(0x01)

/*! @name Standby duration macros */
#define BMP2_STANDBY_DURN_POS                         UINT8_C(0x05)
#define BMP2_STANDBY_DURN_MSK                         UINT8_C(0xE0)

/*! @name Calibration parameters' relative position */
#define BMP2_DIG_T1_LSB_POS                           UINT8_C(0)
#define BMP2_DIG_T1_MSB_POS                           UINT8_C(1)
#define BMP2_DIG_T2_LSB_POS                           UINT8_C(2)
#define BMP2_DIG_T2_MSB_POS                           UINT8_C(3)
#define BMP2_DIG_T3_LSB_POS                           UINT8_C(4)
#define BMP2_DIG_T3_MSB_POS                           UINT8_C(5)
#define BMP2_DIG_P1_LSB_POS                           UINT8_C(6)
#define BMP2_DIG_P1_MSB_POS                           UINT8_C(7)
#define BMP2_DIG_P2_LSB_POS                           UINT8_C(8)
#define BMP2_DIG_P2_MSB_POS                           UINT8_C(9)
#define BMP2_DIG_P3_LSB_POS                           UINT8_C(10)
#define BMP2_DIG_P3_MSB_POS                           UINT8_C(11)
#define BMP2_DIG_P4_LSB_POS                           UINT8_C(12)
#define BMP2_DIG_P4_MSB_POS                           UINT8_C(13)
#define BMP2_DIG_P5_LSB_POS                           UINT8_C(14)
#define BMP2_DIG_P5_MSB_POS                           UINT8_C(15)
#define BMP2_DIG_P6_LSB_POS                           UINT8_C(16)
#define BMP2_DIG_P6_MSB_POS                           UINT8_C(17)
#define BMP2_DIG_P7_LSB_POS                           UINT8_C(18)
#define BMP2_DIG_P7_MSB_POS                           UINT8_C(19)
#define BMP2_DIG_P8_LSB_POS                           UINT8_C(20)
#define BMP2_DIG_P8_MSB_POS                           UINT8_C(21)
#define BMP2_DIG_P9_LSB_POS                           UINT8_C(22)
#define BMP2_DIG_P9_MSB_POS                           UINT8_C(23)
#define BMP2_DIG_P10_POS                              UINT8_C(24)
#define BMP2_CALIB_DATA_SIZE                          UINT8_C(25)

/*! @brief Macros holding the minimum and maximum for trimming values */
#define BMP2_ST_DIG_T1_MIN                            UINT16_C(19000)
#define BMP2_ST_DIG_T1_MAX                            UINT16_C(35000)
#define BMP2_ST_DIG_T2_MIN                            UINT16_C(22000)
#define BMP2_ST_DIG_T2_MAX                            UINT16_C(30000)
#define BMP2_ST_DIG_T3_MIN                            INT16_C(-3000)
#define BMP2_ST_DIG_T3_MAX                            INT16_C(-1000)
#define BMP2_ST_DIG_P1_MIN                            UINT16_C(30000)
#define BMP2_ST_DIG_P1_MAX                            UINT16_C(42000)
#define BMP2_ST_DIG_P2_MIN                            INT16_C(-12970)
#define BMP2_ST_DIG_P2_MAX                            INT16_C(-8000)
#define BMP2_ST_DIG_P3_MIN                            INT16_C(-5000)
#define BMP2_ST_DIG_P3_MAX                            UINT16_C(8000)
#define BMP2_ST_DIG_P4_MIN                            INT16_C(-10000)
#define BMP2_ST_DIG_P4_MAX                            UINT16_C(18000)
#define BMP2_ST_DIG_P5_MIN                            INT16_C(-500)
#define BMP2_ST_DIG_P5_MAX                            UINT16_C(1100)
#define BMP2_ST_DIG_P6_MIN                            INT16_C(-1000)
#define BMP2_ST_DIG_P6_MAX                            UINT16_C(1000)
#define BMP2_ST_DIG_P7_MIN                            INT16_C(-32768)
#define BMP2_ST_DIG_P7_MAX                            UINT16_C(32767)
#define BMP2_ST_DIG_P8_MIN                            INT16_C(-30000)
#define BMP2_ST_DIG_P8_MAX                            UINT16_C(10000)
#define BMP2_ST_DIG_P9_MIN                            INT16_C(-10000)
#define BMP2_ST_DIG_P9_MAX                            UINT16_C(30000)

/*! @brief Macros holding the minimum and maximum for trimming values */
/* 0x00000 is minimum output value */
#define BMP2_ST_ADC_T_MIN                             INT32_C(0x00000)

/* 0xFFFF0 is maximum 20-bit output value without over sampling */
#define BMP2_ST_ADC_T_MAX                             INT32_C(0xFFFF0)

/* 0x00000 is minimum output value */
#define BMP2_ST_ADC_P_MIN                             INT32_C(0x00000)

/* 0xFFFF0 is maximum 20-bit output value without over sampling */
#define BMP2_ST_ADC_P_MAX                             INT32_C(0xFFFF0)

/*! @brief Macros to read out API revision number */
/* Register holding custom trimming values */
#define BMP2_ST_TRIMCUSTOM_REG                        UINT8_C(0x87)
#define BMP2_ST_TRIMCUSTOM_REG_APIREV_POS             UINT8_C(1)
#define BMP2_ST_TRIMCUSTOM_REG_APIREV_MSK             UINT8_C(0x06)
#define BMP2_ST_TRIMCUSTOM_REG_APIREV_LEN             UINT8_C(2)
#define BMP2_ST_TRIMCUSTOM_REG_APIREV_REG             BMP2_ST_TRIMCUSTOM_REG

/* Highest API revision supported is revision 0. */
#define BMP2_ST_MAX_APIREVISION                       UINT8_C(0x00)

#define BMP2_MSBLSB_TO_U16(msb, lsb)                  (((uint16_t)msb << 8) | ((uint16_t)lsb))

/*! @name Bit-slicing macros */
#define BMP2_GET_BITS(reg_data, bitname)              ((reg_data & bitname##_MSK) \
                                                       >> bitname##_POS)
#define BMP2_SET_BITS(reg_data, bitname, val)         ((reg_data & \
                                                        ~bitname##_MSK) | ((val << bitname##_POS) & bitname##_MSK))
#define BMP2_SET_BITS_POS_0(reg_data, bitname, data)  ((reg_data & \
                                                        ~(bitname##_MSK)) | (data & bitname##_MSK))
#define BMP2_GET_BITS_POS_0(reg_data, bitname)        (reg_data & \
                                                       (bitname##_MSK))

/******************************************************************************/
/*! @name           Function Pointers                             */
/******************************************************************************/
#ifndef BMP2_INTF_RET_TYPE
#define BMP2_INTF_RET_TYPE                            int8_t
#endif

#ifndef BMP2_INTF_RET_SUCCESS
#define BMP2_INTF_RET_SUCCESS                         INT8_C(0)
#endif

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific read functions of the user
 *
 * @param[in]     reg_addr : 8bit register address of the sensor
 * @param[out]    reg_data : Data from the specified address
 * @param[in]     length   : Length of the reg_data array
 * @param[in,out] intf_ptr : Void pointer that can enable the linking of descriptors
 *                           for interface related callbacks
 * @retval 0 for Success
 * @retval Non-zero for Failure
 */
typedef BMP2_INTF_RET_TYPE (*bmp2_read_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr);

/*!
 * @brief Bus communication function pointer which should be mapped to
 * the platform specific read functions of the user
 *
 * @param[in]     reg_addr : 8bit register address of the sensor
 * @param[out]    reg_data : Data from the specified address
 * @param[in]     length   : Length of the reg_data array
 * @param[in,out] intf_ptr : Void pointer that can enable the linking of descriptors
 *                           for interface related callbacks
 * @retval 0 for Success
 * @retval Non-zero for Failure
 */
typedef BMP2_INTF_RET_TYPE (*bmp2_write_fptr_t)(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length,
                                                void *intf_ptr);

/*!
 * @brief Delay function pointer which should be mapped to
 * delay function of the user
 *
 * @param period - The time period in microseconds
 * @param[in,out] intf_ptr : Void pointer that can enable the linking of descriptors
 *                           for interface related callbacks
 */
typedef void (*bmp2_delay_us_fptr_t)(uint32_t period, void *intf_ptr);

/******************************************************************************/
/*!  @name         Enum Declarations                                  */
/******************************************************************************/

/*!
 * @brief Enum to define BMP2 sensor interfaces
 */
enum bmp2_intf {
    /*! SPI interface */
    BMP2_SPI_INTF,
    /*! I2C interface */
    BMP2_I2C_INTF
};

/******************************************************************************/
/*!  @name         Structure Declarations                             */
/******************************************************************************/

/*!
 * @brief Calibration parameters' structure
 */
struct bmp2_calib_param
{
    /*! Calibration parameter of temperature data */

    /*! Calibration t1 data */
    uint16_t dig_t1;

    /*! Calibration t2 data */
    int16_t dig_t2;

    /*! Calibration t3 data */
    int16_t dig_t3;

    /*! Calibration parameter of pressure data */

    /*! Calibration p1 data */
    uint16_t dig_p1;

    /*! Calibration p2 data */
    int16_t dig_p2;

    /*! Calibration p3 data */
    int16_t dig_p3;

    /*! Calibration p4 data */
    int16_t dig_p4;

    /*! Calibration p5 data */
    int16_t dig_p5;

    /*! Calibration p6 data */
    int16_t dig_p6;

    /*! Calibration p7 data */
    int16_t dig_p7;

    /*! Calibration p8 data */
    int16_t dig_p8;

    /*! Calibration p9 data */
    int16_t dig_p9;

    /*! Calibration p10 data */
    int8_t dig_p10;

    /*! Fine resolution temperature value */
    int32_t t_fine;
};

/*!
 * @brief Sensor configuration structure
 */
struct bmp2_config
{
    /*! Over-sampling of temperature data */
    uint8_t os_temp;

    /*! Over-sampling of pressure data */
    uint8_t os_pres;

    /*! Odr set by setting standby duration */
    uint8_t odr;

    /*! Over-sampling mode */
    uint8_t os_mode;

    /*! Time constant of IIR filter */
    uint8_t filter;

    /*! Enable 3-wire SPI interface */
    uint8_t spi3w_en;
};

/*!
 * @brief Sensor status structure
 */
struct bmp2_status
{
    /*! Data registers' status */
    uint8_t measuring;

    /*! Image register status */
    uint8_t im_update;
};

/*!
 * @brief Uncompensated data structure
 */
struct bmp2_uncomp_data
{
    /*! Uncompensated temperature data */
    int32_t temperature;

    /*! Uncompensated pressure data */
    uint32_t pressure;
};

/*!
 * @brief bmp2 sensor structure which comprises of temperature and pressure
 */
#ifdef BMP2_DOUBLE_COMPENSATION
struct bmp2_data
{
    /*! Compensated pressure */
    double pressure;

    /*! Compensated temperature */
    double temperature;
};
#else
struct bmp2_data
{
    /*! Compensated pressure */
    uint32_t pressure;

    /*! Compensated temperature */
    int32_t temperature;
};
#endif

/*!
 * @brief API device structure
 */
struct bmp2_dev
{
    /*! Chip Id */
    uint8_t chip_id;

    /*! SPI/I2C Interface selection */
    enum bmp2_intf intf;

    /*!
     * The interface pointer is used to enable the user
     * to link their interface descriptors for reference during the
     * implementation of the read and write interfaces to the
     * hardware.
     */
    void *intf_ptr;

    /*! Variable that holds result of read/write function */
    BMP2_INTF_RET_TYPE intf_rslt;

    /*! Bus read function pointer */
    bmp2_read_fptr_t read;

    /*! Bus write function pointer */
    bmp2_write_fptr_t write;

    /*! Delay(in us) function pointer */
    bmp2_delay_us_fptr_t delay_us;

    /*! Powermode of the sensor */
    uint8_t power_mode;

    /*! Structure of calibration parameters' */
    struct bmp2_calib_param calib_param;
};

#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif /* _BMP2_DEFS_H */
