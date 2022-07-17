//#include "stm32l1xx.h"
#include "stm32f4xx.h"
#include "DEV_Config.h"
#include "EPD_2in13.h"
#include "EPD_Test.h"
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "EPD_2in13.h"
//#include "FreeRTOS.h"
//#include "task.h"

extern void InitStm32F4(void);
//extern void vTask(void *pvParameters);
void ParsingDataLidar(uint8_t ch);
void SWO_PrintChar  (char c);

#define LIDAR_HEADER_BYTE           (0x59U)
#define LIDAR_FRAME_SIZE            (9U)

typedef enum LIDAR_FRAME_enum
{
    LIDAR_HEADER_LOW,
    LIDAR_HEADER_HIGH,
    LIDAR_DISTANCE_LOW,
    LIDAR_DISTANCE_HIGH,
    LIDAR_STRENGTH_LOW,
    LIDAR_STRENGTH_HIGH,
    LIDAR_TEMPERATURE_LOW,
    LIDAR_TEMPERATURE_HIGH,
    LIDAR_CRC
}LIDAR_FRAME;



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
#define SIZE_BUF_LIDAR_DATA         64U
uint8_t lidarData[SIZE_BUF_LIDAR_DATA];
uint16_t index=0;

UBYTE array[4000];
//    //Create a new image cache
UBYTE *BlackImage = array;

void main(void)
{
    //Init();

    InitStm32F4();

    DEV_Module_Init();


   // EPD_2in13_test();

    EPD_2IN13_Init(EPD_2IN13_PART);
    EPD_2IN13_Clear();
    DEV_Delay_ms(4000);



    Paint_NewImage(BlackImage, EPD_2IN13_WIDTH, EPD_2IN13_HEIGHT, 270, WHITE);
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_DrawString_EN(0, 0, "hello world", &Font24, WHITE, BLACK);
    EPD_2IN13_Display(BlackImage);
    DEV_Delay_ms(1000);
    Paint_DrawString_EN(0, 25, "hello world", &Font20, WHITE, BLACK);
    EPD_2IN13_Display(BlackImage);
    DEV_Delay_ms(1000);
    Paint_DrawString_EN(0, 45, "hello world", &Font16, WHITE, BLACK);
    EPD_2IN13_Display(BlackImage);
    DEV_Delay_ms(1000);
    Paint_DrawString_EN(0, 61, "hello world", &Font8, WHITE, BLACK);
    EPD_2IN13_Display(BlackImage);
    DEV_Delay_ms(1000);


    for(int i =0;i<0xffff;i++)
    {
        Paint_ClearWindows(0, 69, 100, 100, WHITE);
        EPD_2IN13_Display(BlackImage);
        DEV_Delay_ms(1000);
        Paint_DrawNum(0, 70, i, &Font24, BLACK, WHITE);
        EPD_2IN13_Display(BlackImage);
    }



    while(1);





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
        //SWO_PrintString("Hello world\r\n");
        //for (int i=0;i<0xfffff;i++);
        if (SET == USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
        {
            if ((SIZE_BUF_LIDAR_DATA-1) <= index)
            {
                index = 0;
            }
            else
            {
                uint8_t ch = USART_ReceiveData(USART1);
                lidarData[index] = ch;
                index++;
                ParsingDataLidar(ch);
            }
        }
    }

}

void ParsingDataLidar(uint8_t ch)
{
    static LIDAR_FRAME LIDAR_Frame = LIDAR_HEADER_LOW;
    static uint16_t distance = 0;
    static uint16_t strength = 0;
    static uint16_t temperature = 0;

    switch(LIDAR_Frame)
    {
        case LIDAR_HEADER_LOW:
            if (LIDAR_HEADER_BYTE == ch)
            {
                LIDAR_Frame = LIDAR_HEADER_HIGH;
            }
            break;
        case LIDAR_HEADER_HIGH:
            if (LIDAR_HEADER_BYTE == ch)
            {
                LIDAR_Frame = LIDAR_DISTANCE_LOW;
            }
            else
            {
                LIDAR_Frame = LIDAR_HEADER_LOW;
            }
            break;
        case LIDAR_DISTANCE_LOW:
            distance = ch;
            LIDAR_Frame = LIDAR_DISTANCE_HIGH;
            break;
        case LIDAR_DISTANCE_HIGH:
            distance |= (uint16_t)ch << 8;
            LIDAR_Frame = LIDAR_STRENGTH_LOW;
            break;
        case LIDAR_STRENGTH_LOW:
            strength = ch;
            LIDAR_Frame = LIDAR_STRENGTH_HIGH;
            break;
        case LIDAR_STRENGTH_HIGH:
            strength |= (uint16_t)ch << 8;
            LIDAR_Frame = LIDAR_TEMPERATURE_LOW;
            break;
        case LIDAR_TEMPERATURE_LOW:
            temperature = ch;
            LIDAR_Frame = LIDAR_TEMPERATURE_HIGH;
            break;
        case LIDAR_TEMPERATURE_HIGH:
            temperature |= (uint16_t)ch << 8;
            LIDAR_Frame = LIDAR_CRC;
            break;
        case LIDAR_CRC:
            LIDAR_Frame = LIDAR_HEADER_LOW;
            break;
        default:
            LIDAR_Frame = LIDAR_HEADER_LOW;
    }


}