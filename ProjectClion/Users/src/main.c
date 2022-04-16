#include "stm32l1xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OneWire.h"

extern void Init(void);
extern void vTask(void *pvParameters);

void SWO_PrintChar  (char c);
void SWO_PrintString(const char *s);

/*********************************************************************
* Определения для блока отладки (debug unit) Cortex-M
*/
#define ITM_STIM_U32 (*(volatile unsigned int*)0xE0000000) // Stimulus Port Register, 32-битный доступ
#define ITM_STIM_U8 (*(volatile char*)0xE0000000) // Stimulus Port Register, 8-битный доступ
#define ITM_ENA (*(volatile unsigned int*)0xE0000E00) // Trace Enable Ports Register
#define ITM_TCR (*(volatile unsigned int*)0xE0000E80) // Trace control register

/*********************************************************************
* SWO_PrintChar()
*
* Описание функции: проверяет, настроен ли SWO. Если нет, то производит возврат,
* чтобы избежать зависания приложения, когда отладчик не подключен. Если SWO
* настроен, то печатает символ в регистр ITM_STIM, чтобы передать его данные
* через SWO.
* Параметры
* c: печатаемый символ.
* Примечания: могут быть добавлены дополнительные проверки регистров, специфических
* для некоторых моделей микроконтроллеров.
*/
void SWO_PrintChar(char c)
{
    // Проверка: ITM_TCR.ITMENA установлен?
    if ((ITM_TCR & 1) == 0)
    {
        return;
    }
    // Проверка: stimulus port разрешен?
    if ((ITM_ENA & 1) == 0)
    {
        return;
    }
    // Ожидание, пока STIMx не освободится,
    // затем передача данных.
    while ((ITM_STIM_U8 & 1) == 0);
    ITM_STIM_U8 = c;
}

/*********************************************************************
* SWO_PrintString()
*
* Описание функции: вывод строки через SWO.
*/
void SWO_PrintString(const char *s)
{
    // Печать символов друг за другом.
    while (*s)
    {
        SWO_PrintChar(*s++);
    }
}

void main(void)
{
   Init();
   // Init OneWire
   ONE_WIRE_init();

   uint8_t presence=0;
   while(1)
   {
       //presence = ONE_WIRE_reset();
       //ONE_WIRE_readBit();
       //ONE_WIRE_writeBit(1);
       //ONE_WIRE_readByte();
       ONE_WIRE_writeByte(0xaa);
       INIT_Delay(10000);
   }

    /* Enable trace in core debug */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    DBGMCU->CR = 0x27;
    /* Enable PC and IRQ sampling output */
   // DWT->CTRL = 0x400113FF;
    /* Set TPIU prescaler to 16. */
    TPI->ACPR = 0x3f;
    /* Set protocol to NRZ */
    TPI->SPPR = 2;


    ITM->LAR = 0xC5ACCE55;
    ITM->TCR = 0x1000D;
    ITM->TER |= 0x01;
/*
    xTaskCreate(vTask,"Task1",50,NULL,1,NULL);

    vTaskStartScheduler();
*/
    while(1)
    {
       // SPI_I2S_SendData(SPI1, 0xaa);
        SWO_PrintString("Hello world\r\n");
        for (int i=0;i<0xfffff;i++);

    }
}