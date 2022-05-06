#include "stm32l1xx.h"
//#include "FreeRTOS.h"
//#include "task.h"
#include "OneWire.h"
#include "usart_drv.h"
#include "Init.h"
#include "ds18b20.h"
#include "ftoa.h"
#include "printf.h"
#define PRINTF_DISABLE_SUPPORT_FLOAT
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL
//extern void vTask(void *pvParameters);
#define DS18B20_ONE_WIRE_CH             (0U)
#define TLM_CHANNEL                     (0)

char buffer[128];


void main(void)
{
    Init();
    // Init OneWire
    ONE_WIRE_init();
    USART_init();
    float t = 0.0f;


   uint8_t presence=0;
   //uint64_t ID=0x28176280122101d5U;
   uint64_t ID=0;//0xd501211280621728U;
    STD_RESULT result = RESULT_NOT_OK;

    result = DS18B20_GetID(DS18B20_ONE_WIRE_CH,&ID);

    while(1)
   {
       //presence = ONE_WIRE_reset();
       //ID = DS18B20_GetID();
       result = DS18B20_GetTemperature(DS18B20_ONE_WIRE_CH,&ID,&t);
       //printf("ID = %llx\r\n",ID);

       if (result == RESULT_NOT_OK)
       {
           printf("RESULT_NOT_OK\r\n");
       }
       else
       {
           ftoa(t, buffer, 10);
           printf("t = %s\r\n",buffer);
       }

       INIT_Delay(1000000);
   }

/*
    xTaskCreate(vTask,"Task1",50,NULL,1,NULL);

    vTaskStartScheduler();
*/
    while(1)
    {
        for (int i=0;i<0xfffff;i++);

    }
}



void _putchar(char character)
{
    USART_PutChar(TLM_CHANNEL, character);
}
