#include "stm32l1xx.h"
#include "FreeRTOS.h"
#include "task.h"


 void vTask(void *pvParameters)
{
    while(1)
    {
        SPI_I2S_SendData(SPI1, 0xaa);
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}
