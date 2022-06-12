//**************************************************************************************************
// @Module        TASK_SENSOR_READ
// @Filename      task_sensors_read.c
//--------------------------------------------------------------------------------------------------
// @Platform      STM32
//--------------------------------------------------------------------------------------------------
// @Compatible    STM32L151
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the TASK_SENSOR_READ functionality.
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
#include "tasks_sensors_read.h"
// drivers
#include "ds18b20.h"
#include "am2305_drv.h"
#include "bme280.h"
#include "printf.h"
#include "ftoa.h"
// FreeRtos
#include "FreeRTOS.h"
#include "task.h"


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

#define DS18B20_ONE_WIRE_CH             (0U)
#define TASK_SENS_RD_SIZE_BUFF_PRINT    (128U)

// BME280 I2C
#define BME280_I2C_CH                   (I2C1)

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// printf buffer.
static char bufferPrintf[TASK_SENS_RD_SIZE_BUFF_PRINT];

// bme280 structure
static struct bme280_dev bme280;

// addr bme i2c
static uint8_t dev_addr = BME280_I2C_ADDR_PRIM;




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
// @Function      vTaskSensorsRead()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void vTaskSensorsRead(void *pvParameters)
{
    STD_RESULT result = RESULT_NOT_OK;
    float tDS = 0.0f;
    float tAM = 0.0f;
    float humidity = 0.0f;
    float k = 3.0f / 1024.0f;
    float k2 = 6.0f * k;
    float wind = 0.0f;
    uint64_t ID=0;//0xd501211280621728U;
    uint8_t presence=0;

    bme280.intf_ptr = &dev_addr;
    bme280.intf = BME280_I2C_INTF;
    bme280.read = user_i2c_read;
    bme280.write = user_i2c_write;
    bme280.delay_ms = user_delay_ms;

    void I2C_SendData(I2C_TypeDef* I2Cx, uint8_t Data);
    uint8_t I2C_ReceiveData(I2C_TypeDef* I2Cx);



    if (BME280_OK== bme280_init(&bme280))
    {
        printf("BME280 was initialize\r\n");
    }
    else
    {
        printf("BME280 wasn't initialize\r\n");
    }


    result = DS18B20_GetID(DS18B20_ONE_WIRE_CH,&ID);
/*
    for (int i=0;i<TAS K_SENS_RD_SIZE_BUFF_PRINT;i++)
    {
        bufferPrintf[i] = 0;
    }
*/
    ADC_SoftwareStartConv(ADC1);
uint8_t  cnt=0;
    while(1)
    {
        uint16_t get_val;
        get_val = ADC_GetConversionValue(ADC1);
        wind = (float)get_val * k2;
        ftoa(wind, bufferPrintf, 2);
        printf("ADC_anemometer value is %s\r\n", bufferPrintf);
        printf("cnt %d\r\n", cnt);
        cnt++;




//        result = AM2305_GetHumidityTemperature(&humidity,&tAM);
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
            printf("I am ready!!!\r\n");


        vTaskDelay(1000/portTICK_RATE_MS);
    }
}// end of vTaskSensorsRead



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      user_i2c_read()
//--------------------------------------------------------------------------------------------------
// @Description  Read data I2C
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

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
    ErrorStatus I2C_CheckEvent(BME280_I2C_CH, uint32_t I2C_EVENT)

    uint8_t I2C_ReceiveData(I2C_TypeDef* I2Cx)

    return rslt;
}// end of user_i2c_read()

//****************************************** end of file *******************************************








