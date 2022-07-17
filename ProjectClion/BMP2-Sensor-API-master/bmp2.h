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
* @file       bmp2.h
* @date       2021-05-21
* @version    v1.0.1
*
*/

/*! @file bmp2.h
 * @brief Sensor driver for BMP2 sensor
 */

/*!
 * @defgroup bmp2 BMP2
 */

#ifndef _BMP2_H
#define _BMP2_H

/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/

/*!             Header files
 ****************************************************************************/
#include "bmp2_defs.h"

/***************************************************************************/

/*!     BMP2 User Interface function prototypes
 ****************************************************************************/

/**
 * \ingroup bmp2
 * \defgroup bmp2ApiInit Initialization
 * @brief Perform initialization of sensor
 */

/*!
 * \ingroup bmp2ApiInit
 * \page bmp2_api_bmp2_init bmp2_init
 * \code
 * int8_t bmp2_init(struct bmp2_dev *dev);
 * \endcode
 * @details This API is the entry point.
 *  It reads the chip-id and calibration data from the sensor.
 *
 *  @param[in,out] dev : Structure instance of bmp2_dev
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 */
int8_t bmp2_init(struct bmp2_dev *dev);

/**
 * \ingroup bmp2
 * \defgroup bmp2ApiRegs Registers
 * @brief Initialize the sensor and device structure
 */

/*!
 * \ingroup bmp2ApiRegs
 * \page bmp2_api_bmp2_get_regs bmp2_get_regs
 * \code
 * int8_t bmp2_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmp2_dev *dev);
 * \endcode
 * @details This API reads the data from the given register address of the
 * sensor.
 *
 * @param[in] reg_addr  : Register address from where the data to be read
 * @param[out] reg_data : Pointer to data buffer to store the read data.
 * @param[in] len       : No of bytes of data to be read.
 * @param[in] dev       : Structure instance of bmp2_dev.
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 */
int8_t bmp2_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmp2_dev *dev);

/*!
 * \ingroup bmp2ApiRegs
 * \page bmp2_api_bmp2_set_regs bmp2_set_regs
 * \code
 * int8_t bmp2_set_regs(uint8_t *reg_addr, const uint8_t *reg_data, uint32_t len, struct bmp2_dev *dev);
 * \endcode
 * @details This API writes the given data to the register addresses
 * of the sensor.
 *
 * @param[in] reg_addr : Register address from where the data to be written.
 * @param[in] reg_data : Pointer to data buffer which is to be written
 * in the sensor.
 * @param[in] len      : No of bytes of data to write..
 * @param[in] dev      : Structure instance of bmp2_dev.
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 */
int8_t bmp2_set_regs(uint8_t *reg_addr, const uint8_t *reg_data, uint32_t len, struct bmp2_dev *dev);

/**
 * \ingroup bmp2
 * \defgroup bmp2ApiSoftreset Soft reset
 * @brief Perform self test
 */

/*!
 * \ingroup bmp2ApiSoftreset
 * \page bmp2_api_bmp2_soft_reset bmp2_soft_reset
 * \code
 * int8_t bmp2_soft_reset(struct bmp2_dev *dev);
 * \endcode
 * @details This API triggers the soft reset of the sensor.
 *
 * @param[in] dev : Structure instance of bmp2_dev.
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 */
int8_t bmp2_soft_reset(struct bmp2_dev *dev);

/**
 * \ingroup bmp2
 * \defgroup bmp2ApiPowermode Power mode
 * @brief Set / Get power mode of the sensor
 */

/*!
 * \ingroup bmp2ApiPowermode
 * \page bmp2_api_bmp2_get_power_mode bmp2_get_power_mode
 * \code
 * int8_t bmp2_get_power_mode(uint8_t *mode, struct bmp2_dev *dev);
 * \endcode
 * @details This API reads the power mode.
 *
 * @param[out] mode : BMP2_POWERMODE_SLEEP, BMP2_POWERMODE_NORMAL,
 *     BMP2_POWERMODE_FORCED
 * @param[in] dev   : Structure instance of bmp2_dev
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 */
int8_t bmp2_get_power_mode(uint8_t *mode, struct bmp2_dev *dev);

/*!
 * \ingroup bmp2ApiPowermode
 * \page bmp2_api_bmp2_set_power_mode bmp2_set_power_mode
 * \code
 * int8_t bmp2_set_power_mode(uint8_t mode, const struct bmp2_config *conf, struct bmp2_dev *dev);
 * \endcode
 * @details This API writes the power mode.
 *
 * @param[in] mode : BMP2_POWERMODE_SLEEP, BMP2_POWERMODE_NORMAL,
 *     BMP2_POWERMODE_FORCED
 * @param[in] conf   : Structure instance of bmp2_config
 * @param[in] dev    : Structure instance of bmp2_dev
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 */
int8_t bmp2_set_power_mode(uint8_t mode, const struct bmp2_config *conf, struct bmp2_dev *dev);

/**
 * \ingroup bmp2
 * \defgroup bmp2ApiConfig Configuration
 * @brief Set / Get configurations of the sensor
 */

/*!
 * \ingroup bmp2ApiConfig
 * \page bmp2_api_bmp2_get_config bmp2_get_config
 * \code
 * int8_t bmp2_get_config(struct bmp2_config *conf, struct bmp2_dev *dev);
 * \endcode
 * @details This API reads the data from the ctrl_meas register and config
 * register. It gives the currently set temperature and pressure over-sampling
 * configuration, power mode configuration, sleep duration and
 * IIR filter coefficient.
 *
 * @param[out] conf : Current configuration of the bmp2
 * conf->osrs_t, conf->osrs_p = BMP2_OS_NONE, BMP2_OS_1X,
 *     BMP2_OS_2X, BMP2_OS_4X, BMP2_OS_8X, BMP2_OS_16X
 *
 * conf->odr = BMP2_ODR_0_5_MS, BMP2_ODR_62_5_MS, BMP2_ODR_125_MS,
 *     BMP2_ODR_250_MS, BMP2_ODR_500_MS, BMP2_ODR_1000_MS,
 *     BMP2_ODR_2000_MS, BMP2_ODR_4000_MS
 *
 * conf->filter = BMP2_FILTER_OFF, BMP2_FILTER_COEFF_2,
 *     BMP2_FILTER_COEFF_4, BMP2_FILTER_COEFF_8, BMP2_FILTER_COEFF_16
 *
 * conf->spi3w_en = BMP2_SPI3_WIRE_ENABLE, BMP2_SPI3_WIRE_DISABLE
 *
 * @param[in] dev   : Structure instance of bmp2_dev
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 */
int8_t bmp2_get_config(struct bmp2_config *conf, struct bmp2_dev *dev);

/*!
 * \ingroup bmp2ApiConfig
 * \page bmp2_api_bmp2_set_config bmp2_set_config
 * \code
 * int8_t bmp2_set_config(const struct bmp2_config *conf, struct bmp2_dev *dev);
 * \endcode
 * @details This API writes the data to the ctrl_meas register and config register.
 * It sets the over-sampling mode, power mode configuration,
 * sleep duration and IIR filter coefficient.
 *
 * @param[in] conf : Desired configuration to the bmp2
 * conf->osrs_t, conf->osrs_p = BMP2_OS_NONE, BMP2_OS_1X,
 *     BMP2_OS_2X, BMP2_OS_4X, BMP2_OS_8X, BMP2_OS_16X
 *
 * conf->odr = BMP2_ODR_0_5_MS, BMP2_ODR_62_5_MS, BMP2_ODR_125_MS,
 *     BMP2_ODR_250_MS, BMP2_ODR_500_MS, BMP2_ODR_1000_MS,
 *     BMP2_ODR_2000_MS, BMP2_ODR_4000_MS
 *
 * conf->filter = BMP2_FILTER_OFF, BMP2_FILTER_COEFF_2,
 *     BMP2_FILTER_COEFF_4, BMP2_FILTER_COEFF_8, BMP2_FILTER_COEFF_16
 *
 * conf->spi3w_en = BMP2_SPI3_WIRE_ENABLE, BMP2_SPI3_WIRE_DISABLE
 *
 *  Over-sampling settings   |   conf->os_pres   |   conf->os_temp
 *--------------------------|-------------------|---------------------------
 *  Ultra low power         |       1           |       1
 *  Low power               |       2           |       1
 *  Standard resolution     |       4           |       1
 *  High resolution         |       8           |       1
 *  Ultra high resolution   |       16          |       2
 *
 * @param[in] dev  : Structure instance of bmp2_dev
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 */
int8_t bmp2_set_config(const struct bmp2_config *conf, struct bmp2_dev *dev);

/**
 * \ingroup bmp2
 * \defgroup bmp2ApiStatus Status register
 * @brief Read status register
 */

/*!
 * \ingroup bmp2ApiStatus
 * \page bmp2_api_bmp2_get_status bmp2_get_status
 * \code
 * int8_t bmp2_get_status(struct bmp2_status *status, struct bmp2_dev *dev);
 * \endcode
 * @details This API reads the status register
 *
 * @param[in,out] status : Structure instance of bmp2_status.
 * @param[in] dev        : Structure instance of bmp2_dev
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 */
int8_t bmp2_get_status(struct bmp2_status *status, struct bmp2_dev *dev);

/**
 * \ingroup bmp2
 * \defgroup bmp2ApiSensorData Sensor Data
 * @brief Data processing of sensor
 */

/*!
 * \ingroup bmp2ApiSensorData
 * \page bmp2_api_bmp2_get_sensor_data bmp2_get_sensor_data
 * \code
 * int8_t bmp2_get_sensor_data(struct bmp2_data *comp_data, struct bmp2_dev *dev);
 * \endcode
 * @details This API reads the pressure and temperature data from the
 * sensor, compensates the data and store it in the bmp2_data structure
 * instance passed by the user.
 *
 * @param[in] comp_data : Structure instance of bmp2_data
 * @param[in] dev       : Structure instance of bmp2_dev
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 */
int8_t bmp2_get_sensor_data(struct bmp2_data *comp_data, struct bmp2_dev *dev);

/*!
 * \ingroup bmp2ApiSensorData
 * \page bmp2_api_bmp2_compensate_data bmp2_compensate_data
 * \code
 * int8_t bmp2_compensate_data(const struct bmp2_uncomp_data *uncomp_data,
 *                               struct bmp2_data *comp_data,
 *                               struct bmp2_dev *dev);
 * \endcode
 * @details This API is used to compensate the pressure and
 * temperature data.
 *
 * @param[in] uncomp_data : Contains the uncompensated pressure, temperature data.
 * @param[out] comp_data  : Contains the compensated pressure and/or temperature data.
 * @param[in] dev         : Structure instance of bmp2_dev.
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval > 0 -> Warning.
 * @retval < 0 -> Fail.
 *
 */
int8_t bmp2_compensate_data(const struct bmp2_uncomp_data *uncomp_data,
                            struct bmp2_data *comp_data,
                            struct bmp2_dev *dev);

/**
 * \ingroup bmp2
 * \defgroup bmp2ApiMeasTime Compute measurement time
 * @brief Computes measurement time (in microseconds)
 */

/*!
 * \ingroup bmp2ApiMeasTime
 * \page bmp2_api_bmp2_compute_meas_time bmp2_compute_meas_time
 * \code
 * int8_t bmp2_compute_meas_time(uint32_t *sampling_time, const struct bmp2_config *conf, const struct bmp2_dev *dev);
 * \endcode
 * @details This API computes the measurement time in microseconds for the
 * active configuration based on standbytime(conf->odr) and over-sampling mode(conf->os_mode)
 *
 * @param[out] sampling_time : Measurement time for the active configuration in microseconds
 * @param[in] conf           : Structure instance of bmp2_config
 * @param[in] dev            : Structure instance of bmp2_dev
 *
 * @return Result of API execution status.
 *
 * @retval   0 -> Success.
 * @retval < 0 -> Fail.
 *
 */
int8_t bmp2_compute_meas_time(uint32_t *sampling_time, const struct bmp2_config *conf, const struct bmp2_dev *dev);

#ifdef __cplusplus
}
#endif /* End of CPP guard */

#endif /* _BMP2_H */
