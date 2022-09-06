//**************************************************************************************************
// @Module        TASK_READ_SENSORS
// @Filename      task_read_sensors.c
//--------------------------------------------------------------------------------------------------
// @Platform      STM32L4xx
//--------------------------------------------------------------------------------------------------
// @Compatible    STM32L476, STM32L452
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the TASK_READ_SENSORS functionality.
//
//
//                Abbreviations:
//                  None.
//
//
//                Global (public) functions:
//
//
//                Local (private) functions:
//
//
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Native header
#include "task_read_sensors.h"

// Get record manager interface
#include "record_manager.h"

// Get LL HAL
#include "stm32l4xx_ll_i2c.h"

// drivers
#include "ds18b20.h"
#include "am2305_drv.h"
#include "bmp2.h"

#include "stdlib.h"
#include "printf.h"
#include "ftoa.h"
#include "Init.h"
#include "string.h"

#include "time_drv.h"


//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// FreeRtos handler for TASK_READ_SEN
TaskHandle_t    TASK_READ_SEN_hHandlerTask;



//**************************************************************************************************
// Declarations of local (private) data types
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

#define DS18B20_ONE_WIRE_CH             (0U)
#define TASK_SENS_RD_SIZE_BUFF_PRINT    (128U)

// BME280 I2C
#define BME280_I2C_CH                   (I2C1)

// Mutex Acquisition Delay
#define TASK_READ_SENS_MUTEX_DELAY      (1000U)

// ADC max value
#define TASK_READ_SEN_ADC_MAX           (1023U)

// V power
#define TASK_READ_SEN_V_POWER           (float)(3.3)
#define TASK_READ_SEN_ADC_E_M_R                         (float)(TASK_READ_SEN_V_POWER / TASK_READ_SEN_ADC_MAX)

// Battery voltage coefficient
#define TASK_READ_SEN_BAT_RES_COEFFICIENT               (float)(4.5833333)
#define TASK_READ_SEN_BAT_CORRECTION_FACTOR             (float)(1.0)

// Anemometer voltage coefficient
#define TASK_READ_SEN_ANEMOMETER_RES_COEFFICIENT        (float)(4.5833333)
#define TASK_READ_SEN_ANEMOMETER_WIND_COEFFICIENT       (float)(6.0)
#define TASK_READ_SEN_ANEMOMETER_CORRECTION_FACTOR      (float)(1.0)



//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// printf buffer.
static char bufferPrintf[TASK_SENS_RD_SIZE_BUFF_PRINT];

// bme280 structure
//static struct bmp2_dev bmp280;

// addr bme i2c
//static uint8_t bmp280_addr = BMP2_I2C_ADDR_PRIM;

//static TF02_PRO_MEASURED_DATA TF02_PRO_Lidar;

static RECORD_MAN_TYPE_RECORD TASK_READ_SENS_stMeasData;

// BMP280 config structure
static struct bmp2_config bmp280Config;

// BMP280 data structure
static struct bmp2_data bmp280Data;

// BMP280 status structure
static struct bmp2_status bmp280Status;

// BMP280 device structure
static struct bmp2_dev bmp280;

static uint8_t TASK_READ_SEN_BMP280_DEV_ADR = TASK_READ_SEN_BMP280_ADR;

// Temperature of DS18B20 sensor
static float TASK_READ_SEN_fDS18B20_temp = 0.0f;

// Id of DS18B20
static uint64_t TASK_READ_SEN_nDS18B20_ID = 0;//0xd501211280621728U;

// AM2305 humidity
static float TASK_READ_SEN_AM2305_fHumidity = 0.0f;

// AM2305 temperature
static float TASK_READ_SEN_AM2305_fTemperature = 0.0f;

// Battery voltage
static float TASK_READ_SEN_fBatVoltage = 0.0f;

// Battery voltage
static float TASK_READ_SEN_fAnemometer = 0.0f;



//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// I2C read
static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

// I2C write
static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);

// Delay function for BME280 driver
static void user_delay_us(uint32_t period, void *intf_ptr);



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      vTaskReadSensors()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void vTaskReadSensors(void *pvParameters)
{
    uint32_t nQtyRecords = 0U;

    STD_RESULT result = RESULT_NOT_OK;


//    float k = 3.0f / 1024.0f;
//    float k2 = 6.0f * k;
//    float wind = 0.0f;

    bmp280.intf_ptr = &TASK_READ_SEN_BMP280_DEV_ADR;
    bmp280.intf = BMP2_I2C_INTF;
    bmp280.read = user_i2c_read;
    bmp280.write = user_i2c_write;
    bmp280.delay_us = user_delay_us;

    taskENTER_CRITICAL();

    if (BMP2_OK == bmp2_init(&bmp280))
    {
        printf("BMP280 was initialize\r\n");
    }
    else
    {
        printf("BMP280 wasn't initialize\r\n");
    }

    // Get config
    if (BMP2_OK == bmp2_get_config(&bmp280Config, &bmp280))
    {
        printf("BMP280 read config OK \r\n");
    }
    else
    {
        printf("BMP280 read config FAIL \r\n");
    }

    bmp280Config.filter = BMP2_FILTER_OFF;
    bmp280Config.os_mode = BMP2_OS_MODE_ULTRA_LOW_POWER;
    bmp280Config.spi3w_en = BMP2_SPI3_WIRE_DISABLE;
    bmp280Config.odr = BMP2_ODR_125_MS;
    // Set config
    if (BMP2_OK == bmp2_set_config(&bmp280Config, &bmp280))
    {
        printf("BMP280 configured\r\n");
    }
    else
    {
        printf("BMP280 wasn't configure\r\n");
    }
    taskEXIT_CRITICAL();


    taskENTER_CRITICAL();
    if (RESULT_OK == DS18B20_GetID(DS18B20_ONE_WIRE_CH,&TASK_READ_SEN_nDS18B20_ID))
    {
        printf("DS18B20 read ID OK\r\n");
    }
    else
    {
        printf("DS18B20 read ID FAIL\r\n");
    }
    taskEXIT_CRITICAL();

    HAL_ADCEx_Calibration_Start(&ADC_Handle,ADC_SINGLE_ENDED);

    for(;;)
    {
        // Pressure measure
        taskENTER_CRITICAL();
        // Start one measure in force mode
        bmp2_set_power_mode(BMP2_POWERMODE_FORCED, &bmp280Config, &bmp280);

        // Delay 100 ms
        user_delay_us(100000,0);

        // Get measure data BMP280
        bmp2_get_sensor_data(&bmp280Data, &bmp280);
        taskEXIT_CRITICAL();

        ftoa((float)bmp280Data.pressure, bufferPrintf, 1);
        printf("Pressure = %s\r\n",bufferPrintf);

        // Temperature measure
        taskENTER_CRITICAL();
        result = DS18B20_GetTemperature(DS18B20_ONE_WIRE_CH,&TASK_READ_SEN_nDS18B20_ID,&TASK_READ_SEN_fDS18B20_temp);

        if (result == RESULT_NOT_OK)
        {
            printf("DS18B20 isn't OK\r\n");
        }
        else
        {
            ftoa(TASK_READ_SEN_fDS18B20_temp, bufferPrintf, 4);
            printf("tDS = %s\r\n",bufferPrintf);
        }
        taskEXIT_CRITICAL();

        // Humidity measure
        taskENTER_CRITICAL();
        result = AM2305_GetHumidityTemperature(&TASK_READ_SEN_AM2305_fHumidity,
                                               &TASK_READ_SEN_AM2305_fTemperature);
        taskEXIT_CRITICAL();
        if (result == RESULT_NOT_OK)
        {
            printf("AM2305 isn't OK\r\n");
        }
        else
        {
            ftoa(TASK_READ_SEN_AM2305_fTemperature, bufferPrintf, 4);
            printf("tAM = %s\r\n",bufferPrintf);
            ftoa(TASK_READ_SEN_AM2305_fHumidity, bufferPrintf, 3);
            printf("humidity = %s\r\n",bufferPrintf);
        }

        // Battery voltage measure
        HAL_ADCEx_InjectedStart(&ADC_Handle);
        user_delay_us(1000000,0);
        if (HAL_OK == HAL_ADCEx_InjectedPollForConversion(&ADC_Handle, 1000))
        {
            uint32_t temp = 0;
            temp = HAL_ADCEx_InjectedGetValue(&ADC_Handle, INIT_BAT_RANK);
            TASK_READ_SEN_fBatVoltage = (((float)temp * TASK_READ_SEN_ADC_E_M_R) * TASK_READ_SEN_BAT_CORRECTION_FACTOR) * \
                                            TASK_READ_SEN_BAT_RES_COEFFICIENT;
            ftoa(TASK_READ_SEN_fBatVoltage, bufferPrintf, 2);
            printf("Battery voltage is %s\r\n", bufferPrintf);
//            printf("Battery CODE voltage is %d\r\n", temp);

            temp = HAL_ADCEx_InjectedGetValue(&ADC_Handle, INIT_ANEMOMETER_RANK);
            TASK_READ_SEN_fAnemometer = ((((float)temp * TASK_READ_SEN_ADC_E_M_R) * TASK_READ_SEN_ANEMOMETER_CORRECTION_FACTOR) * \
                                            TASK_READ_SEN_ANEMOMETER_RES_COEFFICIENT) * TASK_READ_SEN_ANEMOMETER_WIND_COEFFICIENT;
            ftoa(TASK_READ_SEN_fAnemometer, bufferPrintf, 2);
//            printf("Anemometer voltage is %s\r\n", bufferPrintf);
            printf("Wind speed m/s %s\r\n", bufferPrintf);
//            printf("Anemometer CODE voltage is %d\r\n", temp);
        }
        else
        {
            printf("Battery voltage FAIL measurement\r\n");
        }


        // Save data
        TASK_READ_SENS_stMeasData.fTemperature = TASK_READ_SEN_fDS18B20_temp;
        TASK_READ_SENS_stMeasData.fHumidity = TASK_READ_SEN_AM2305_fHumidity;
        TASK_READ_SENS_stMeasData.fPressure = (float)bmp280Data.pressure;
        TASK_READ_SENS_stMeasData.fWindSpeed = TASK_READ_SEN_fAnemometer;
        TASK_READ_SENS_stMeasData.fBatteryVoltage = TASK_READ_SEN_fBatVoltage;


        TASK_READ_SENS_stMeasData.nUnixTime = TIME_GetUnixTimestamp();

        // Attempt get mutex
        if (pdTRUE == xSemaphoreTake(RECORD_MAN_xMutex, TASK_READ_SENS_MUTEX_DELAY))
        {
            // Store record
            if (RESULT_OK == RECORD_MAN_Store((uint8_t*)&TASK_READ_SENS_stMeasData,
                                              RECORD_MAN_SIZE_OF_RECORD_BYTES,
                                              &nQtyRecords))
            {
                printf("Record store OK; Quantity records %d\r\n",nQtyRecords);
            }
            else
            {
                printf("Record store error\r\n");
            }

            // Return mutex
            xSemaphoreGive(RECORD_MAN_xMutex);
        }
        else
        {
            printf("TASK_READ_SENS: Mutex of record manager is busy\r\n");
        }

//        vTaskDelay(1000/portTICK_RATE_MS);

        // Blocking vTaskReadSensors
        vTaskSuspend(TASK_READ_SEN_hHandlerTask);
    }

}// end of vTaskReadSensors



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      TASK_READ_SEN_MeasurePressure()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static STD_RESULT TASK_READ_SEN_MeasurePressure(void)
{
    STD_RESULT enResult = RESULT_NOT_OK;



    return enResult;
} // end of TASK_READ_SEN_MeasurePressure()


//**************************************************************************************************
// @Function      user_i2c_read()
//--------------------------------------------------------------------------------------------------
// @Description   Read data from I2C
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   0 - success,
//                non-zero - fail
//--------------------------------------------------------------------------------------------------
// @Parameters    reg_addr - address register for read
//                reg_data - pointer data to write
//                len - length data to read
//                intf_ptr can be used as a variable to store the I2C address of the device
//**************************************************************************************************
static int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
    uint32_t nCntBytes = 0U;

    // Wait BUSY flag
    while (TRUE == LL_I2C_IsActiveFlag_BUSY(I2CBMP280Handler.Instance))
    {
        DoNothing();
    }

    /*
     * The parameter intf_ptr can be used as a variable to store the I2C address of the device
     */

    /*
     * Data on the bus should be like
     * |------------+---------------------|
     * | I2C action | Data                |
     * |------------+---------------------|
     * | Start      | -                   |
     * | Write      | (reg_addr)          |
     * | Stop       | -                   |
     * | Start      | -                   |
     * | Read       | (reg_data[0])       |
     * | Read       | (....)              |
     * | Read       | (reg_data[len - 1]) |
     * | Stop       | -                   |
     * |------------+---------------------|
     */

    // Disable auto end mode
    LL_I2C_DisableAutoEndMode(I2CBMP280Handler.Instance);

    // Disable reload mode
    LL_I2C_DisableReloadMode(I2CBMP280Handler.Instance);

    // Set slave address
    LL_I2C_SetSlaveAddr(I2CBMP280Handler.Instance, (*(uint8_t*)intf_ptr) << 1U);

    // Transfer request
    LL_I2C_SetTransferRequest(I2CBMP280Handler.Instance,LL_I2C_REQUEST_WRITE);

    // Set transfer size
    LL_I2C_SetTransferSize(I2CBMP280Handler.Instance,1U);

    // Generate start condition
    LL_I2C_GenerateStartCondition(I2CBMP280Handler.Instance);

    // Wait transmit start bit
    while (FALSE == LL_I2C_IsActiveFlag_BUSY(I2CBMP280Handler.Instance))
    {
        DoNothing();
    }

    while ((FALSE == LL_I2C_IsActiveFlag_TC(I2CBMP280Handler.Instance)) && \
           (FALSE == LL_I2C_IsActiveFlag_NACK(I2CBMP280Handler.Instance)) && \
           (TRUE == LL_I2C_IsActiveFlag_BUSY(I2CBMP280Handler.Instance)))
    {
        // Write register address
        if (TRUE == LL_I2C_IsActiveFlag_TXIS(I2CBMP280Handler.Instance))
        {
            LL_I2C_TransmitData8(I2CBMP280Handler.Instance, reg_addr);
        }
        else
        {
            DoNothing();
        }
    }

    // Disable auto end mode
    LL_I2C_DisableAutoEndMode(I2CBMP280Handler.Instance);

    // Disable reload mode
    LL_I2C_DisableReloadMode(I2CBMP280Handler.Instance);

    // Set slave address
    LL_I2C_SetSlaveAddr(I2CBMP280Handler.Instance, (*(uint8_t*)intf_ptr) << 1U);

    // Transfer request
    LL_I2C_SetTransferRequest(I2CBMP280Handler.Instance,LL_I2C_REQUEST_READ);

    // Set transfer size
    LL_I2C_SetTransferSize(I2CBMP280Handler.Instance,len);

    // Generate start condition
    LL_I2C_GenerateStartCondition(I2CBMP280Handler.Instance);

    while ((FALSE == LL_I2C_IsActiveFlag_TC(I2CBMP280Handler.Instance)) && \
           (FALSE == LL_I2C_IsActiveFlag_NACK(I2CBMP280Handler.Instance)) && \
           (TRUE == LL_I2C_IsActiveFlag_BUSY(I2CBMP280Handler.Instance)))
    {
        if (TRUE == LL_I2C_IsActiveFlag_RXNE(I2CBMP280Handler.Instance))
        {
            reg_data[nCntBytes] = LL_I2C_ReceiveData8(I2CBMP280Handler.Instance);
            nCntBytes++;
        }
        else
        {
            DoNothing();
        }
    }

    // Generate stop
    LL_I2C_GenerateStopCondition(I2CBMP280Handler.Instance);

    if (len == nCntBytes)
    {
        rslt = 0U;
    }
    else
    {
        rslt = 1U;
    }

    return rslt;
}// end of user_i2c_read()



//**************************************************************************************************
// @Function      user_i2c_write()
//--------------------------------------------------------------------------------------------------
// @Description   Write data to I2C
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   0 - success,
//                non-zero - fail
//--------------------------------------------------------------------------------------------------
// @Parameters    reg_addr - address register to write
//                reg_data - pointer data to write
//                len - length data to write
//                intf_ptr can be used as a variable to store the I2C address of the device
//**************************************************************************************************
static int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    uint32_t nCntBytes = 0U;

    // Wait BUSY flag
    while (TRUE == LL_I2C_IsActiveFlag_BUSY(I2CBMP280Handler.Instance))
    {
        DoNothing();
    }

    /*
     * The parameter intf_ptr can be used as a variable to store the I2C address of the device
     */

    /*
     * Data on the bus should be like
     * |------------+---------------------|
     * | I2C action | Data                |
     * |------------+---------------------|
     * | Start      | -                   |
     * | Write      | (reg_addr)          |
     * | Write      | (reg_data[0])       |
     * | Write      | (....)              |
     * | Write      | (reg_data[len - 1]) |
     * | Stop       | -                   |
     * |------------+---------------------|
     */

// Disable auto end mode
    LL_I2C_DisableAutoEndMode(I2CBMP280Handler.Instance);

    // Disable reload mode
    LL_I2C_DisableReloadMode(I2CBMP280Handler.Instance);

    // Set slave address
    LL_I2C_SetSlaveAddr(I2CBMP280Handler.Instance, (*(uint8_t*)intf_ptr) << 1U);

    // Transfer request
    LL_I2C_SetTransferRequest(I2CBMP280Handler.Instance,LL_I2C_REQUEST_WRITE);

    // Set transfer size
    LL_I2C_SetTransferSize(I2CBMP280Handler.Instance,len + 1U);

    // Generate start condition
    LL_I2C_GenerateStartCondition(I2CBMP280Handler.Instance);

    // Wait transmit start bit
    while (FALSE == LL_I2C_IsActiveFlag_BUSY(I2CBMP280Handler.Instance))
    {
        DoNothing();
    }

    while ((FALSE == LL_I2C_IsActiveFlag_TXIS(I2CBMP280Handler.Instance)) && \
           (FALSE == LL_I2C_IsActiveFlag_NACK(I2CBMP280Handler.Instance)) && \
           (TRUE == LL_I2C_IsActiveFlag_BUSY(I2CBMP280Handler.Instance)))
    {
        DoNothing();
    }

    // Write register address
    if (TRUE == LL_I2C_IsActiveFlag_TXIS(I2CBMP280Handler.Instance))
    {
        LL_I2C_TransmitData8(I2CBMP280Handler.Instance, reg_addr);
    }
    else
    {

        DoNothing();
    }

    while ((FALSE == LL_I2C_IsActiveFlag_TC(I2CBMP280Handler.Instance)) && \
           (FALSE == LL_I2C_IsActiveFlag_NACK(I2CBMP280Handler.Instance)) && \
           (TRUE == LL_I2C_IsActiveFlag_BUSY(I2CBMP280Handler.Instance)))
    {
        if (TRUE == LL_I2C_IsActiveFlag_TXIS(I2CBMP280Handler.Instance))
        {
            LL_I2C_TransmitData8(I2CBMP280Handler.Instance,reg_data[nCntBytes]);
            nCntBytes++;
        }
        else
        {
            DoNothing();
        }
    }

    // Generate stop
    LL_I2C_GenerateStopCondition(I2CBMP280Handler.Instance);

    if (len == nCntBytes)
    {
        rslt = 0U;
    }
    else
    {
        rslt = 1U;
    }

    return rslt;
}// end of user_i2c_write()



//**************************************************************************************************
// @Function      user_delay_ms()
//--------------------------------------------------------------------------------------------------
// @Description   Delay in microseconds
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    period - Delay in microseconds.
//               intf_ptr - oid pointer that can enable the linking of descriptors for interface
//               related call backs
//**************************************************************************************************
static void user_delay_us(uint32_t period, void *intf_ptr)
{
    INIT_Delay(period);
}// end of user_delay_ms
//****************************************** end of file *******************************************








