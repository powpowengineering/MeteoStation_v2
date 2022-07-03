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
* @file       bmp2.c
* @date       2021-05-21
* @version    v1.0.1
*
*/

/*! @file bmp2.c
 * @brief Sensor driver for BMP2 sensor
 */

#include "bmp2.h"

/********************** Static function declarations ************************/

/*!
 * @brief This internal API is used to check for null-pointers in the device
 * structure.
 *
 * @param[in] dev : Structure instance of bmp2_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static int8_t null_ptr_check(const struct bmp2_dev *dev);

/*!
 * @brief This internal API interleaves the register addresses and respective
 * register data for a burst write
 *
 * @param[in] reg_addr   : Register address array
 * @param[out] temp_buff : Interleaved register address and data array
 * @param[in] reg_data   : Register data array
 * @param[in] len        : Length of the reg_addr and reg_data arrays
 *
 */
static void interleave_data(const uint8_t *reg_addr, uint8_t *temp_buff, const uint8_t *reg_data, uint32_t len);

/*!
 * @brief This API is used to read the calibration parameters used
 * for calculating the compensated data.
 *
 * @param[in] dev : Structure instance of bmp2_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static int8_t get_calib_param(struct bmp2_dev *dev);

/*!
 * @brief This internal API to reset the sensor, restore/set conf, restore/set mode
 *
 * @param[in] mode : Desired mode
 * @param[in] conf : Desired configuration to the bmp2
 *
 * conf->os_mode = BMP2_OS_MODE_ULTRA_LOW_POWER, BMP2_OS_MODE_LOW_POWER, BMP2_OS_MODE_STANDARD_RESOLUTION,
 *     BMP2_OS_MODE_HIGH_RESOLUTION, BMP2_OS_MODE_ULTRA_HIGH_RESOLUTION
 *
 * conf->mode = BMP2_POWERMODE_SLEEP, BMP2_POWERMODE_FORCED, BMP2_POWERMODE_NORMAL
 *
 * conf->odr = BMP2_ODR_0_5_MS, BMP2_ODR_62_5_MS, BMP2_ODR_125_MS,
 *     BMP2_ODR_250_MS, BMP2_ODR_500_MS, BMP2_ODR_1000_MS,
 *     BMP2_ODR_2000_MS, BMP2_ODR_4000_MS
 *
 * conf->filter = BMP2_FILTER_OFF, BMP2_FILTER_COEFF_2,
 *     BMP2_FILTER_COEFF_4, BMP2_FILTER_COEFF_8, BMP2_FILTER_COEFF_16
 *
 * @param[in] dev  : Structure instance of bmp2_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static int8_t conf_sensor(uint8_t mode, const struct bmp2_config *conf, struct bmp2_dev *dev);

/*!
 *  @brief This API is used to set the over-sampling rate of temperature and pressure
 *  based on the over-sampling mode.
 *
 *  @param[in] reg_data  : Contains register data
 *  @param[out] conf     : Desired configuration to the bmp2
 *
 */
static void set_os_mode(uint8_t *reg_data, const struct bmp2_config *conf);

/*!
 *  @brief This API is used to parse the pressure and temperature
 *  data and store it in the bmp2_uncomp_data structure instance.
 *
 *  @param[in] reg_data     : Contains register data which needs to be parsed
 *  @param[out] uncomp_data : Contains the uncompensated pressure, temperature
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static int8_t parse_sensor_data(const uint8_t *reg_data, struct bmp2_uncomp_data *uncomp_data);

#ifdef BMP2_DOUBLE_COMPENSATION

/*!
 * @brief This internal API is used to get the compensated temperature from
 * uncompensated temperature. This API uses double floating precision.
 *
 * @param[out] comp_temperature : Compensated temperature data in double.
 * @param[in] uncomp_data       : Contains the uncompensated temperature data.
 * @param[in] dev               : Structure instance of bmp2_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t compensate_temperature(double *comp_temperature,
                                     const struct bmp2_uncomp_data *uncomp_data,
                                     struct bmp2_dev *dev);

/*!
 * @brief This internal API is used to get the compensated pressure from
 * uncompensated pressure. This API uses double floating precision.
 *
 * @param[out] comp_pressure : Compensated pressure data in double.
 * @param[in] uncomp_data : Contains the uncompensated pressure data.
 * @param[in] dev         : Structure instance of bmp2_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t compensate_pressure(double *comp_pressure,
                                  const struct bmp2_uncomp_data *uncomp_data,
                                  const struct bmp2_dev *dev);

#else

/*!
 * @brief This internal API is used to get the compensated temperature from
 * uncompensated temperature. This API uses 32bit integer data type.
 *
 * @param[out] comp_temperature : Compensated temperature data in integer.
 * @param[in] uncomp_data       : Contains the uncompensated temperature data.
 * @param[in] dev               : Structure instance of bmp2_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t compensate_temperature(int32_t *comp_temperature,
                                     const struct bmp2_uncomp_data *uncomp_data,
                                     struct bmp2_dev *dev);

/*!
 * @brief This internal API is used to get the compensated pressure from
 * uncompensated pressure. This API uses 64bit integer data type.
 *
 * @param[out] comp_pressure : Compensated pressure data in integer.
 * @param[in] uncomp_data    : Contains the uncompensated pressure data.
 * @param[in] dev            : Structure instance of bmp2_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval >0 -> Warning
 * @retval <0 -> Fail
 */
static int8_t compensate_pressure(uint32_t *comp_pressure,
                                  const struct bmp2_uncomp_data *uncomp_data,
                                  const struct bmp2_dev *dev);

#endif

/*!
 * @This internal API checks whether the uncompensated temperature and pressure are within the range
 *
 * @param[in] utemperature : Uncompensated temperature
 * @param[in] upressure    : Uncompensated pressure
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval <0 -> Fail
 */
static int8_t st_check_boundaries(int32_t utemperature, int32_t upressure);

/****************** User Function Definitions *******************************/

/*!
 * @brief This API is the entry point.
 * It reads the chip-id and calibration data from the sensor.
 */
int8_t bmp2_init(struct bmp2_dev *dev)
{
    int8_t rslt;

    rslt = null_ptr_check(dev);

    if (rslt == BMP2_OK)
    {
        rslt = bmp2_get_regs(BMP2_REG_CHIP_ID, &dev->chip_id, 1, dev);

        /* Check for chip id validity */
        if (rslt == BMP2_OK)
        {
            if (dev->chip_id == BMP2_CHIP_ID)
            {
                rslt = get_calib_param(dev);
            }
            else
            {
                rslt = BMP2_E_DEV_NOT_FOUND;
            }
        }
    }

    return rslt;
}

/*!
 * @brief This API reads the data from the given register address of the
 * sensor.
 */
int8_t bmp2_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmp2_dev *dev)
{
    int8_t rslt;

    rslt = null_ptr_check(dev);

    if ((rslt == BMP2_OK) && (reg_data != NULL))
    {
        /* Mask the register address' MSB if interface selected is SPI */
        if (dev->intf == BMP2_SPI_INTF)
        {
            reg_addr = reg_addr | BMP2_SPI_RD_MASK;
        }

        dev->intf_rslt = dev->read(reg_addr, reg_data, len, dev->intf_ptr);

        /* Check for communication error and mask with an internal error code */
        if (dev->intf_rslt != BMP2_INTF_RET_SUCCESS)
        {
            rslt = BMP2_E_COM_FAIL;
        }
    }
    else
    {
        rslt = BMP2_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API writes the given data to the register addresses
 * of the sensor.
 */
int8_t bmp2_set_regs(uint8_t *reg_addr, const uint8_t *reg_data, uint32_t len, struct bmp2_dev *dev)
{
    int8_t rslt;
    uint8_t temp_buff[8]; /* Typically not to write more than 4 registers */
    uint32_t temp_len;
    uint8_t reg_addr_cnt;

    if (len > BMP2_MAX_LEN)
    {
        len = BMP2_MAX_LEN;
    }

    rslt = null_ptr_check(dev);

    if ((rslt == BMP2_OK) && (reg_addr != NULL) && (reg_data != NULL))
    {
        if (len > 0)
        {
            temp_buff[0] = reg_data[0];

            /* Mask the register address' MSB if interface selected is SPI */
            if (dev->intf == BMP2_SPI_INTF)
            {
                /* Converting all the reg address into proper SPI write address
                 * i.e making MSB(R/`W) bit 0
                 */
                for (reg_addr_cnt = 0; reg_addr_cnt < len; reg_addr_cnt++)
                {
                    reg_addr[reg_addr_cnt] = reg_addr[reg_addr_cnt] & BMP2_SPI_WR_MASK;
                }
            }

            /* Burst write mode */
            if (len > 1)
            {
                /* Interleave register address w.r.t data for burst write */
                interleave_data(reg_addr, temp_buff, reg_data, len);
                temp_len = ((len * 2) - 1);
            }
            else
            {
                temp_len = len;
            }

            dev->intf_rslt = dev->write(reg_addr[0], temp_buff, temp_len, dev->intf_ptr);

            /* Check for communication error and mask with an internal error code */
            if (dev->intf_rslt != BMP2_INTF_RET_SUCCESS)
            {
                rslt = BMP2_E_COM_FAIL;
            }
        }
        else
        {
            rslt = BMP2_E_INVALID_LEN;
        }
    }
    else
    {
        rslt = BMP2_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API triggers the soft-reset of the sensor.
 */
int8_t bmp2_soft_reset(struct bmp2_dev *dev)
{
    int8_t rslt;
    uint8_t reg_addr = BMP2_REG_SOFT_RESET;
    uint8_t soft_rst_cmd = BMP2_SOFT_RESET_CMD;

    rslt = bmp2_set_regs(&reg_addr, &soft_rst_cmd, 1, dev);

    return rslt;
}

/*!
 * @brief This API reads the data from the ctrl_meas register and config
 * register. It gives the currently set temperature and pressure over-sampling
 * configuration, power mode configuration, sleep duration and
 * IIR filter coefficient.
 */
int8_t bmp2_get_config(struct bmp2_config *conf, struct bmp2_dev *dev)
{
    int8_t rslt;
    uint8_t temp[2] = { 0, 0 };

    if (conf != NULL)
    {
        rslt = bmp2_get_regs(BMP2_REG_CTRL_MEAS, temp, 2, dev);

        if (rslt == BMP2_OK)
        {
            conf->os_temp = BMP2_GET_BITS(temp[0], BMP2_OS_TEMP);
            conf->os_pres = BMP2_GET_BITS(temp[0], BMP2_OS_PRES);
            conf->odr = BMP2_GET_BITS(temp[1], BMP2_STANDBY_DURN);
            conf->filter = BMP2_GET_BITS(temp[1], BMP2_FILTER);
            conf->spi3w_en = BMP2_GET_BITS_POS_0(temp[1], BMP2_SPI3_ENABLE);
        }
    }
    else
    {
        rslt = BMP2_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API writes the data to the ctrl_meas register and config register.
 * It sets the over-sampling mode, power mode configuration,
 * sleep duration and IIR filter coefficient.
 */
int8_t bmp2_set_config(const struct bmp2_config *conf, struct bmp2_dev *dev)
{
    return conf_sensor(BMP2_POWERMODE_SLEEP, conf, dev);
}

/*!
 * @brief This API reads the status register
 */
int8_t bmp2_get_status(struct bmp2_status *status, struct bmp2_dev *dev)
{
    int8_t rslt;
    uint8_t temp;

    if (status != NULL)
    {
        rslt = bmp2_get_regs(BMP2_REG_STATUS, &temp, 1, dev);

        if (rslt == BMP2_OK)
        {
            status->measuring = BMP2_GET_BITS(temp, BMP2_STATUS_MEAS);
            status->im_update = BMP2_GET_BITS_POS_0(temp, BMP2_STATUS_IM_UPDATE);
        }
    }
    else
    {
        rslt = BMP2_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API reads the power mode.
 */
int8_t bmp2_get_power_mode(uint8_t *mode, struct bmp2_dev *dev)
{
    int8_t rslt;
    uint8_t temp;

    if (mode != NULL)
    {
        rslt = bmp2_get_regs(BMP2_REG_CTRL_MEAS, &temp, 1, dev);

        *mode = BMP2_GET_BITS_POS_0(temp, BMP2_POWERMODE);
        dev->power_mode = *mode;
    }
    else
    {
        rslt = BMP2_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API writes the power mode.
 */
int8_t bmp2_set_power_mode(uint8_t mode, const struct bmp2_config *conf, struct bmp2_dev *dev)
{
    int8_t rslt;

    rslt = conf_sensor(mode, conf, dev);

    return rslt;
}

/*!
 * @brief This API reads the pressure and temperature data from the
 * sensor, compensates the data and store it in the bmp2_data structure
 * instance passed by the user.
 */
int8_t bmp2_get_sensor_data(struct bmp2_data *comp_data, struct bmp2_dev *dev)
{
    int8_t rslt;
    uint8_t temp[BMP2_P_T_LEN] = { 0 };
    struct bmp2_uncomp_data uncomp_data = { 0 };

    if (comp_data != NULL)
    {
        rslt = bmp2_get_regs(BMP2_REG_PRES_MSB, temp, BMP2_P_T_LEN, dev);

        if (rslt == BMP2_OK)
        {
            /* Parse the read data from the sensor */
            rslt = parse_sensor_data(temp, &uncomp_data);

            if (rslt == BMP2_OK)
            {
                /* Compensate the pressure and/or temperature
                 * data from the sensor
                 */
                rslt = bmp2_compensate_data(&uncomp_data, comp_data, dev);
            }
        }
    }
    else
    {
        rslt = BMP2_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API is used to compensate the pressure and
 * temperature data.
 */
int8_t bmp2_compensate_data(const struct bmp2_uncomp_data *uncomp_data,
                            struct bmp2_data *comp_data,
                            struct bmp2_dev *dev)
{
    int8_t rslt;

    rslt = null_ptr_check(dev);

    if ((rslt == BMP2_OK) && (uncomp_data != NULL) && (comp_data != NULL))
    {
        /* Initialize to zero */
        comp_data->temperature = 0;
        comp_data->pressure = 0;

        rslt = compensate_temperature(&comp_data->temperature, uncomp_data, dev);

        if (rslt == BMP2_OK)
        {
            rslt = compensate_pressure(&comp_data->pressure, uncomp_data, dev);
        }
    }
    else
    {
        rslt = BMP2_E_NULL_PTR;
    }

    return rslt;
}

/*!
 * @brief This API computes the measurement time in microseconds for the
 * active configuration based on standbytime(conf->odr) and over-sampling mode(conf->os_mode)
 */
int8_t bmp2_compute_meas_time(uint32_t *sampling_time, const struct bmp2_config *conf, const struct bmp2_dev *dev)
{
    int8_t rslt;

    /* Array contains measurement time in microseconds */
    uint32_t measurement_time[] = { 5500, 7500, 11500, 19500, 37500 };
    uint32_t standby_time[] = { 500, 62500, 125000, 250000, 500000, 1000000, 2000000, 4000000 };

    rslt = null_ptr_check(dev);

    if ((rslt == BMP2_OK) && (conf != NULL))
    {
        if (dev->power_mode == BMP2_POWERMODE_NORMAL)
        {
            /* Time in microseconds */
            (*sampling_time) = measurement_time[conf->os_mode] + standby_time[conf->odr];
        }
        else
        {
            /* Time in microseconds */
            (*sampling_time) = measurement_time[conf->os_mode];
        }
    }
    else
    {
        rslt = BMP2_E_NULL_PTR;
    }

    return rslt;
}

/****************** Static Function Definitions *******************************/

/*!
 * @brief This internal API is used to check for null-pointers in the device
 * structure.
 */
static int8_t null_ptr_check(const struct bmp2_dev *dev)
{
    int8_t rslt;

    if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_us == NULL))
    {
        /* Null-pointer found */
        rslt = BMP2_E_NULL_PTR;
    }
    else
    {
        rslt = BMP2_OK;
    }

    return rslt;
}

/*!
 * @brief This internal API interleaves the register addresses and respective
 * register data for a burst write
 */
static void interleave_data(const uint8_t *reg_addr, uint8_t *temp_buff, const uint8_t *reg_data, uint32_t len)
{
    uint32_t index;

    for (index = 1; index < len; index++)
    {
        temp_buff[(index * 2) - 1] = reg_addr[index];
        temp_buff[index * 2] = reg_data[index];
    }
}

/*!
 * @brief This API is used to read the calibration parameters used
 * for calculating the compensated data.
 */
static int8_t get_calib_param(struct bmp2_dev *dev)
{
    int8_t rslt;
    uint8_t temp[BMP2_CALIB_DATA_SIZE] = { 0 };

    rslt = bmp2_get_regs(BMP2_REG_DIG_T1_LSB, temp, BMP2_CALIB_DATA_SIZE, dev);

    if (rslt == BMP2_OK)
    {
        dev->calib_param.dig_t1 = (uint16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_T1_MSB_POS], temp[BMP2_DIG_T1_LSB_POS]));
        dev->calib_param.dig_t2 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_T2_MSB_POS], temp[BMP2_DIG_T2_LSB_POS]));
        dev->calib_param.dig_t3 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_T3_MSB_POS], temp[BMP2_DIG_T3_LSB_POS]));
        dev->calib_param.dig_p1 = (uint16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P1_MSB_POS], temp[BMP2_DIG_P1_LSB_POS]));
        dev->calib_param.dig_p2 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P2_MSB_POS], temp[BMP2_DIG_P2_LSB_POS]));
        dev->calib_param.dig_p3 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P3_MSB_POS], temp[BMP2_DIG_P3_LSB_POS]));
        dev->calib_param.dig_p4 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P4_MSB_POS], temp[BMP2_DIG_P4_LSB_POS]));
        dev->calib_param.dig_p5 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P5_MSB_POS], temp[BMP2_DIG_P5_LSB_POS]));
        dev->calib_param.dig_p6 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P6_MSB_POS], temp[BMP2_DIG_P6_LSB_POS]));
        dev->calib_param.dig_p7 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P7_MSB_POS], temp[BMP2_DIG_P7_LSB_POS]));
        dev->calib_param.dig_p8 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P8_MSB_POS], temp[BMP2_DIG_P8_LSB_POS]));
        dev->calib_param.dig_p9 = (int16_t) (BMP2_MSBLSB_TO_U16(temp[BMP2_DIG_P9_MSB_POS], temp[BMP2_DIG_P9_LSB_POS]));
        dev->calib_param.dig_p10 = (int8_t) ((uint8_t)(temp[BMP2_DIG_P10_POS]));
    }

    return rslt;
}

/*!
 * @brief This internal API to reset the sensor, restore/set conf, restore/set mode
 */
static int8_t conf_sensor(uint8_t mode, const struct bmp2_config *conf, struct bmp2_dev *dev)
{
    int8_t rslt;
    uint8_t temp[2] = { 0, 0 };
    uint8_t reg_addr[2] = { BMP2_REG_CTRL_MEAS, BMP2_REG_CONFIG };

    if (conf != NULL)
    {
        rslt = bmp2_get_regs(BMP2_REG_CTRL_MEAS, temp, 2, dev);

        if (rslt == BMP2_OK)
        {
            /* Here the intention is to put the device to sleep
             * within the shortest period of time
             */
            rslt = bmp2_soft_reset(dev);

            if (rslt == BMP2_OK)
            {
                set_os_mode(temp, conf);
                temp[1] = BMP2_SET_BITS(temp[1], BMP2_STANDBY_DURN, conf->odr);
                temp[1] = BMP2_SET_BITS(temp[1], BMP2_FILTER, conf->filter);
                temp[1] = BMP2_SET_BITS_POS_0(temp[1], BMP2_SPI3_ENABLE, conf->spi3w_en);

                rslt = bmp2_set_regs(reg_addr, temp, 2, dev);

                if ((rslt == BMP2_OK) && (mode != BMP2_POWERMODE_SLEEP))
                {
                    dev->power_mode = mode;

                    /* Write only the power mode register in a separate write */
                    temp[0] = BMP2_SET_BITS_POS_0(temp[0], BMP2_POWERMODE, mode);
                    rslt = bmp2_set_regs(reg_addr, temp, 1, dev);
                }
            }
        }
    }
    else
    {
        rslt = BMP2_E_NULL_PTR;
    }

    return rslt;
}

/*!
 *  @brief This internal API is used to set the over-sampling rate of temperature and pressure
 *  based on the over-sampling mode.
 */
static void set_os_mode(uint8_t *reg_data, const struct bmp2_config *conf)
{
    switch (conf->os_mode)
    {
        case BMP2_OS_MODE_ULTRA_LOW_POWER:
            reg_data[0] = BMP2_SET_BITS(reg_data[0], BMP2_OS_TEMP, BMP2_OS_1X);
            reg_data[0] = BMP2_SET_BITS(reg_data[0], BMP2_OS_PRES, BMP2_OS_1X);
            break;
        case BMP2_OS_MODE_LOW_POWER:
            reg_data[0] = BMP2_SET_BITS(reg_data[0], BMP2_OS_TEMP, BMP2_OS_1X);
            reg_data[0] = BMP2_SET_BITS(reg_data[0], BMP2_OS_PRES, BMP2_OS_2X);
            break;
        case BMP2_OS_MODE_STANDARD_RESOLUTION:
            reg_data[0] = BMP2_SET_BITS(reg_data[0], BMP2_OS_TEMP, BMP2_OS_1X);
            reg_data[0] = BMP2_SET_BITS(reg_data[0], BMP2_OS_PRES, BMP2_OS_4X);
            break;
        case BMP2_OS_MODE_HIGH_RESOLUTION:
            reg_data[0] = BMP2_SET_BITS(reg_data[0], BMP2_OS_TEMP, BMP2_OS_1X);
            reg_data[0] = BMP2_SET_BITS(reg_data[0], BMP2_OS_PRES, BMP2_OS_8X);
            break;
        case BMP2_OS_MODE_ULTRA_HIGH_RESOLUTION:
            reg_data[0] = BMP2_SET_BITS(reg_data[0], BMP2_OS_TEMP, BMP2_OS_2X);
            reg_data[0] = BMP2_SET_BITS(reg_data[0], BMP2_OS_PRES, BMP2_OS_16X);
            break;
        default:
            break;
    }
}

/*!
 *  @brief This internal API is used to parse the pressure and temperature
 *  data and store it in the bmp2_uncomp_data structure instance.
 */
static int8_t parse_sensor_data(const uint8_t *reg_data, struct bmp2_uncomp_data *uncomp_data)
{
    int8_t rslt;

    /* Variables to store the sensor data */
    uint32_t data_xlsb;
    uint32_t data_lsb;
    uint32_t data_msb;

    /* Store the parsed register values for pressure data */
    data_msb = (uint32_t)reg_data[0] << 12;
    data_lsb = (uint32_t)reg_data[1] << 4;
    data_xlsb = (uint32_t)reg_data[2] >> 4;
    uncomp_data->pressure = data_msb | data_lsb | data_xlsb;

    /* Store the parsed register values for temperature data */
    data_msb = (int32_t)reg_data[3] << 12;
    data_lsb = (int32_t)reg_data[4] << 4;
    data_xlsb = (int32_t)reg_data[5] >> 4;
    uncomp_data->temperature = (int32_t)(data_msb | data_lsb | data_xlsb);

    rslt = st_check_boundaries((int32_t)uncomp_data->temperature, (int32_t)uncomp_data->pressure);

    return rslt;
}

#ifdef BMP2_DOUBLE_COMPENSATION

/*!
 * @brief This internal API is used to get the compensated temperature from
 * uncompensated temperature. This API uses double floating precision.
 */
static int8_t compensate_temperature(double *comp_temperature,
                                     const struct bmp2_uncomp_data *uncomp_data,
                                     struct bmp2_dev *dev)
{
    int8_t rslt = BMP2_OK;
    double var1, var2;
    double temperature;

    var1 = (((double) uncomp_data->temperature) / 16384.0 - ((double) dev->calib_param.dig_t1) / 1024.0) *
           ((double) dev->calib_param.dig_t2);
    var2 =
        ((((double) uncomp_data->temperature) / 131072.0 - ((double) dev->calib_param.dig_t1) / 8192.0) *
         (((double) uncomp_data->temperature) / 131072.0 - ((double) dev->calib_param.dig_t1) / 8192.0)) *
        ((double) dev->calib_param.dig_t3);

    dev->calib_param.t_fine = (int32_t) (var1 + var2);
    temperature = (var1 + var2) / 5120.0;

    if (temperature < BMP2_MIN_TEMP_DOUBLE)
    {
        temperature = BMP2_MIN_TEMP_DOUBLE;
        rslt = BMP2_W_MIN_TEMP;
    }

    if (temperature > BMP2_MAX_TEMP_DOUBLE)
    {
        temperature = BMP2_MAX_TEMP_DOUBLE;
        rslt = BMP2_W_MAX_TEMP;
    }

    (*comp_temperature) = temperature;

    return rslt;
}

/*!
 * @brief This internal API is used to get the compensated pressure from
 * uncompensated pressure. This API uses double floating precision.
 */
static int8_t compensate_pressure(double *comp_pressure,
                                  const struct bmp2_uncomp_data *uncomp_data,
                                  const struct bmp2_dev *dev)
{
    int8_t rslt = BMP2_OK;
    double var1, var2;
    double pressure = 0.0;

    var1 = ((double) dev->calib_param.t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double) dev->calib_param.dig_p6) / 32768.0;
    var2 = var2 + var1 * ((double) dev->calib_param.dig_p5) * 2.0;
    var2 = (var2 / 4.0) + (((double) dev->calib_param.dig_p4) * 65536.0);
    var1 = (((double)dev->calib_param.dig_p3) * var1 * var1 / 524288.0 + ((double)dev->calib_param.dig_p2) * var1) /
           524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double) dev->calib_param.dig_p1);

    if (var1 < 0 || var1 > 0)
    {
        pressure = 1048576.0 - (double)uncomp_data->pressure;
        pressure = (pressure - (var2 / 4096.0)) * 6250.0 / var1;
        var1 = ((double)dev->calib_param.dig_p9) * pressure * pressure / 2147483648.0;
        var2 = pressure * ((double)dev->calib_param.dig_p8) / 32768.0;

        pressure = pressure + (var1 + var2 + ((double)dev->calib_param.dig_p7)) / 16.0;

        if (pressure < BMP2_MIN_PRES_DOUBLE)
        {
            pressure = BMP2_MIN_PRES_DOUBLE;
            rslt = BMP2_W_MIN_PRES;
        }

        if (pressure > BMP2_MAX_PRES_DOUBLE)
        {
            pressure = BMP2_MAX_PRES_DOUBLE;
            rslt = BMP2_W_MAX_PRES;
        }

        (*comp_pressure) = pressure;
    }

    return rslt;
}

#else

/*!
 * @brief This internal API is used to get the compensated temperature from
 * uncompensated temperature. This API uses 32bit integer data type.
 */
static int8_t compensate_temperature(int32_t *comp_temperature,
                                     const struct bmp2_uncomp_data *uncomp_data,
                                     struct bmp2_dev *dev)
{
    int8_t rslt = BMP2_OK;
    int32_t var1, var2;
    int32_t temperature;

    var1 =
        ((((uncomp_data->temperature / 8) - ((int32_t) dev->calib_param.dig_t1 * 2))) *
         ((int32_t) dev->calib_param.dig_t2)) / 2048;
    var2 =
        (((((uncomp_data->temperature / 16) - ((int32_t) dev->calib_param.dig_t1)) *
           ((uncomp_data->temperature / 16) - ((int32_t) dev->calib_param.dig_t1))) / 4096) *
         ((int32_t) dev->calib_param.dig_t3)) / 16384;

    dev->calib_param.t_fine = var1 + var2;

    temperature = (dev->calib_param.t_fine * 5 + 128) / 256;

    if (temperature < BMP2_MIN_TEMP_INT)
    {
        temperature = BMP2_MIN_TEMP_INT;
        rslt = BMP2_W_MIN_TEMP;
    }

    if (temperature > BMP2_MAX_TEMP_INT)
    {
        temperature = BMP2_MAX_TEMP_INT;
        rslt = BMP2_W_MAX_TEMP;
    }

    (*comp_temperature) = temperature;

    return rslt;
}

#ifndef BMP2_32BIT_COMPENSATION

/*!
 * @brief This internal API is used to get the compensated pressure from
 * uncompensated pressure. This API uses 64bit integer data type.
 */
static int8_t compensate_pressure(uint32_t *comp_pressure,
                                  const struct bmp2_uncomp_data *uncomp_data,
                                  const struct bmp2_dev *dev)
{
    int8_t rslt = BMP2_OK;
    int64_t var1, var2, p;
    uint32_t pressure = 0;

    var1 = ((int64_t) dev->calib_param.t_fine) - 128000;
    var2 = var1 * var1 * (int64_t) dev->calib_param.dig_p6;
    var2 = var2 + ((var1 * (int64_t) dev->calib_param.dig_p5) * 131072);
    var2 = var2 + (((int64_t) dev->calib_param.dig_p4) * 34359738368);
    var1 = ((var1 * var1 * (int64_t) dev->calib_param.dig_p3) / 256) +
           ((var1 * (int64_t) dev->calib_param.dig_p2) * 4096);
    var1 = (((((int64_t)1) * 140737488355328) + var1)) * ((int64_t)dev->calib_param.dig_p1) / 8589934592;

    if (var1 != 0)
    {
        p = 1048576 - uncomp_data->pressure;
        p = (((p * 2147483648) - var2) * 3125) / var1;
        var1 = (((int64_t) dev->calib_param.dig_p9) * (p / 8192) * (p / 8192)) / 33554432;
        var2 = (((int64_t) dev->calib_param.dig_p8) * p) / 524288;

        p = ((p + var1 + var2) / 256) + (((int64_t)dev->calib_param.dig_p7) * 16);
        pressure = (uint32_t)p;

        if (pressure < BMP2_MIN_PRES_64INT)
        {
            pressure = BMP2_MIN_PRES_64INT;
            rslt = BMP2_W_MIN_PRES;
        }

        if (pressure > BMP2_MAX_PRES_64INT)
        {
            pressure = BMP2_MAX_PRES_64INT;
            rslt = BMP2_W_MAX_PRES;
        }

        (*comp_pressure) = pressure;
    }

    return rslt;
}

#else

/*!
 * @brief This internal API is used to get the compensated pressure from
 * uncompensated pressure. This API uses 32bit integer data type.
 */
static int8_t compensate_pressure(uint32_t *comp_pressure,
                                  const struct bmp2_uncomp_data *uncomp_data,
                                  const struct bmp2_dev *dev)
{
    int8_t rslt = BMP2_OK;
    int32_t var1, var2;
    uint32_t pressure = 0;

    var1 = (((int32_t) dev->calib_param.t_fine) / 2) - (int32_t) 64000;
    var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((int32_t) dev->calib_param.dig_p6);
    var2 = var2 + ((var1 * ((int32_t) dev->calib_param.dig_p5)) * 2);
    var2 = (var2 / 4) + (((int32_t) dev->calib_param.dig_p4) * 65536);
    var1 =
        (((dev->calib_param.dig_p3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8) +
         ((((int32_t) dev->calib_param.dig_p2) * var1) / 2)) / 262144;
    var1 = ((((32768 + var1)) * ((int32_t) dev->calib_param.dig_p1)) / 32768);

    /* Avoid exception caused by division with zero */
    if (var1 != 0)
    {
        pressure = (((uint32_t)(((int32_t)1048576) - uncomp_data->pressure) - (var2 / 4096))) * 3125;

        /* Check for overflows against UINT32_MAX/2; if pressure is left-shifted by 1 */
        if (pressure < 0x80000000)
        {
            pressure = (pressure * 2) / ((uint32_t) var1);
        }
        else
        {
            pressure = (pressure / (uint32_t) var1) * 2;
        }

        var1 = (((int32_t) dev->calib_param.dig_p9) * ((int32_t) (((pressure / 8) * (pressure / 8)) / 8192))) / 4096;
        var2 = (((int32_t) (pressure / 4)) * ((int32_t) dev->calib_param.dig_p8)) / 8192;
        pressure = (uint32_t) ((int32_t) pressure + ((var1 + var2 + dev->calib_param.dig_p7) / 16));

        if (pressure < BMP2_MIN_PRES_32INT)
        {
            pressure = BMP2_MIN_PRES_32INT;
            rslt = BMP2_W_MIN_PRES;
        }

        if (pressure > BMP2_MAX_PRES_32INT)
        {
            pressure = BMP2_MAX_PRES_32INT;
            rslt = BMP2_W_MAX_PRES;
        }

        (*comp_pressure) = pressure;
    }

    return rslt;
}

#endif
#endif

/*!
 * @This internal API checks whether the uncompensated temperature and
 * uncompensated pressure are within the range
 */
static int8_t st_check_boundaries(int32_t utemperature, int32_t upressure)
{
    int8_t rslt = 0;

    /* Check Uncompensated pressure in not valid range AND uncompensated temperature in valid range */
    if ((upressure < BMP2_ST_ADC_P_MIN || upressure > BMP2_ST_ADC_P_MAX) &&
        (utemperature >= BMP2_ST_ADC_T_MIN && utemperature <= BMP2_ST_ADC_T_MAX))
    {
        rslt = BMP2_E_UNCOMP_PRESS_RANGE;
    }
    /* Check Uncompensated temperature in not valid range AND uncompensated pressure in valid range */
    else if ((utemperature < BMP2_ST_ADC_T_MIN || utemperature > BMP2_ST_ADC_T_MAX) &&
             (upressure >= BMP2_ST_ADC_P_MIN && upressure <= BMP2_ST_ADC_P_MAX))
    {
        rslt = BMP2_E_UNCOMP_TEMP_RANGE;
    }
    /* Check Uncompensated pressure in not valid range AND uncompensated temperature in not valid range */
    else if ((upressure < BMP2_ST_ADC_P_MIN || upressure > BMP2_ST_ADC_P_MAX) &&
             (utemperature < BMP2_ST_ADC_T_MIN || utemperature > BMP2_ST_ADC_T_MAX))
    {
        rslt = BMP2_E_UNCOMP_TEMP_AND_PRESS_RANGE;
    }
    else
    {
        rslt = BMP2_OK;
    }

    return rslt;
}
