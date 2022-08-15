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
//#include "ds18b20.h"
//#include "am2305_drv.h"
#include "bmp2.h"
//#include "tf02Pro_drv.h"
#include "stdlib.h"
#include "printf.h"
#include "ftoa.h"
#include "Init.h"
#include "string.h"


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

//    STD_RESULT result = RESULT_NOT_OK;
//    float tDS = 0.0f;
//    float tAM = 0.0f;
//    float humidity = 0.0f;
//    float k = 3.0f / 1024.0f;
//    float k2 = 6.0f * k;
//    float wind = 0.0f;
//    struct bmp2_data bmp280Data;
//    struct bmp2_status bmp280Status;
//    struct bmp2_config bmp280Config;
//    uint64_t ID=0;//0xd501211280621728U;
//    uint8_t presence=0;
//    uint32_t meas_time;
//
//    bmp280.intf_ptr = &bmp280_addr;
//    bmp280.intf = BMP2_I2C_INTF;
//    bmp280.read = user_i2c_read;
//    bmp280.write = user_i2c_write;
//    bmp280.delay_us = user_delay_us;
//
//    if (BMP2_OK== bmp2_init(&bmp280))
//    {
//        printf("BME280 was initialize\r\n");
//    }
//    else
//    {
//        printf("BME280 wasn't initialize\r\n");
//    }
//
//    // Get config
//    bmp2_get_config(&bmp280Config, &bmp280);
//
//    bmp280Config.filter = BMP2_FILTER_OFF;
//    bmp280Config.os_mode = BMP2_OS_MODE_LOW_POWER;//;
//    bmp280Config.spi3w_en = BMP2_SPI3_WIRE_DISABLE;
//    bmp280Config.odr = BMP2_ODR_125_MS;
//
//
//    // Set config
//    bmp2_set_config(&bmp280Config, &bmp280);
//    taskENTER_CRITICAL();
//    // set Power mode NORMAL BMP280
//    bmp2_set_power_mode(BMP2_POWERMODE_FORCED, &bmp280Config, &bmp280);
//    taskEXIT_CRITICAL();
//    // Calculate measurement time in microseconds
//    bmp2_compute_meas_time(&meas_time, &bmp280Config, &bmp280);
//    printf("Measurement time %d\r\n", meas_time);
//
//
//       result = DS18B20_GetID(DS18B20_ONE_WIRE_CH,&ID);
/////*
////    for (int i=0;i<TAS K_SENS_RD_SIZE_BUFF_PRINT;i++)
////    {
////        bufferPrintf[i] = 0;
////    }
////*/
//    ADC_SoftwareStartConv(ADC1);
//    uint8_t  cnt=0;
//    while(1)
//    {
////        uint16_t get_val;
////        get_val = ADC_GetConversionValue(ADC1);
////        wind = (float)get_val * k2;
////        ftoa(wind, bufferPrintf, 2);
////        printf("ADC_anemometer value is %s\r\n", bufferPrintf);
////        printf("cnt %d\r\n", cnt);
////        cnt++;
//
//
//
//        taskENTER_CRITICAL();
//        result = AM2305_GetHumidityTemperature(&humidity,&tAM);
//        taskEXIT_CRITICAL();
//        if (result == RESULT_NOT_OK)
//        {
//            printf("AM2305 isn't OK\r\n");
//        }
//        else
//        {
//            ftoa(tAM, bufferPrintf, 4);
//            printf("tAM = %s\r\n",bufferPrintf);
//            ftoa(humidity, bufferPrintf, 3);
//            printf("humidity = %s\r\n",bufferPrintf);
//        }
//
//        result = DS18B20_GetTemperature(DS18B20_ONE_WIRE_CH,&ID,&tDS);
//        //printf("ID = %llx\r\n",ID);
//
//        if (result == RESULT_NOT_OK)
//        {
//            printf("DS18B20 isn't OK\r\n");
//        }
//        else
//        {
//            ftoa(tDS, bufferPrintf, 4);
//            printf("tDS = %s\r\n",bufferPrintf);
//        }
//        printf("I am ready!!!\r\n");
//
//
//        taskENTER_CRITICAL();
//        bmp2_get_sensor_data(&bmp280Data, &bmp280);
//        taskEXIT_CRITICAL();
//
//        ftoa((float)bmp280Data.temperature, bufferPrintf, 1);
//        printf("Temperature = %s\r\n",bufferPrintf);
//
//        ftoa((float)bmp280Data.pressure, bufferPrintf, 1);
//        printf("Pressure = %s\r\n",bufferPrintf);
//        taskENTER_CRITICAL();
//        bmp2_set_power_mode(BMP2_POWERMODE_FORCED, &bmp280Config, &bmp280);
//        taskEXIT_CRITICAL();
////        taskENTER_CRITICAL();
////        TF02_PRO_GetMeasuredData(&TF02_PRO_Lidar);
////        taskEXIT_CRITICAL();
////
////        printf("Strength = %d\r\n",TF02_PRO_Lidar.nStrength);
////        printf("Distance = %d\r\n",TF02_PRO_Lidar.nDistance);
////        printf("\r\n");
//
//        TASK_SENSOR_READ_data.temperature = tDS;
//        TASK_SENSOR_READ_data.humidity = humidity;
//        TASK_SENSOR_READ_data.pressure = (float)bmp280Data.pressure;
//
//        // Put data in queue
//        xQueueSendToBack( xQueueMeasureData, &TASK_SENSOR_READ_data, 0 );
//
//        // Resume MQTT task
//        vTaskResume( HandleTask_MQTT );
//
//        vTaskDelay((60000 * 5)/portTICK_RATE_MS);
//    }

    for(;;)
    {
        // Prepare test data
        TASK_READ_SENS_stMeasData.fTemperature = ((float)rand()/(float)(RAND_MAX)) * 5;
        TASK_READ_SENS_stMeasData.fHumidity = ((float)rand()/(float)(RAND_MAX)) * 5;
        TASK_READ_SENS_stMeasData.fPressure = ((float)rand()/(float)(RAND_MAX)) * 5;
        TASK_READ_SENS_stMeasData.fWindSpeed = ((float)rand()/(float)(RAND_MAX)) * 5;
        TASK_READ_SENS_stMeasData.fBatteryVoltage = ((float)rand()/(float)(RAND_MAX)) * 5;
        TASK_READ_SENS_stMeasData.nUnixTime = rand();

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

        vTaskDelay(1000/portTICK_RATE_MS);
    }
}// end of vTaskReadSensors



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

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

    // Transfer request
    LL_I2C_SetTransferRequest(I2CBMP280Handler.Instance,LL_I2C_REQUEST_WRITE);

    // Set transfer size
    LL_I2C_SetTransferSize(I2CBMP280Handler.Instance,1U);

    // Set slave address
    LL_I2C_SetSlaveAddr(I2CBMP280Handler.Instance, (*(uint8_t*)intf_ptr));

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

    // Transfer request
    LL_I2C_SetTransferRequest(I2CBMP280Handler.Instance,LL_I2C_REQUEST_READ);

    // Set transfer size
    LL_I2C_SetTransferSize(I2CBMP280Handler.Instance,len);

    // Set slave address
    LL_I2C_SetSlaveAddr(I2CBMP280Handler.Instance, (*(uint8_t*)intf_ptr));

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

    // Transfer request
    LL_I2C_SetTransferRequest(I2CBMP280Handler.Instance,LL_I2C_REQUEST_WRITE);

    // Set transfer size
    LL_I2C_SetTransferSize(I2CBMP280Handler.Instance,len);

    // Set slave address
    LL_I2C_SetSlaveAddr(I2CBMP280Handler.Instance, (*(uint8_t*)intf_ptr));

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








