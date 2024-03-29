//**************************************************************************************************
// @Module        MAIN
// @Filename      main.c
//--------------------------------------------------------------------------------------------------
// @Platform      STM32
//--------------------------------------------------------------------------------------------------
// @Compatible    STM32L151
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the AM2305 functionality.
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

// drivers
#include "OneWire.h"
//#include "usart_drv.h"
#include "Init.h"
#include "ds18b20.h"
#include "am2305_drv.h"
#include "record_manager.h"
#include "ftoa.h"
#include "printf.h"
#include "term-srv.h"
// Freertos
//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
#include "FreeRTOSConfig.h"
#include "cmsis_os.h"

#include "W25Q_drv.h"
// Include task_sensors_read interface
#include "task_read_sensors.h"
// Include task_test_flash interface
//#include "task_test_flash.h"

//#include "task_test_EE.h"
#include "task_master.h"
#include "task_GSM.h"
#include "task_terminal.h"



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

typedef enum
{
    THREAD_TEST_FLASH = 0,
    THREAD_TEST_FLASH_WITH_EE,
    THREAD_TEST_EE,
    THREAD_TEST_MQTT,
    THREAD_MASTER,
    THREAD_READ_SENSORS,
    TREAD_GSM
} Thread_TypeDef;



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************
#define PRINTF_DISABLE_SUPPORT_FLOAT
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#define TLM_CHANNEL                     (0)



//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// None.



//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// System clock configuration
static void SystemClock_Config(void);



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      main()
//--------------------------------------------------------------------------------------------------
// @Description   Main function.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void main(void)
{
    static RTC_TimeTypeDef sTime;
    HAL_Init();

    /* Configure the system clock to 80 MHz */
    SystemClock_Config();

    Init();

    // Power GSM OFF
    HAL_GPIO_WritePin(INIT_DC_GSM_PORT, INIT_DC_GSM_PIN, GPIO_PIN_SET);

    RECORD_MAN_Init();
    // Init OneWire
    ONE_WIRE_init();
    AM2305_Init();
//    USART_init();

    RECORD_MAN_xMutex = xSemaphoreCreateMutex();


    xTaskCreate(vTaskReadSensors,"vTaskReadSensors",TASK_SEN_R_STACK_DEPTH,\
                TASK_SEN_R_PARAMETERS,\
                TASK_SEN_R_PRIORITY,&TASK_READ_SEN_hHandlerTask);

    xTaskCreate(vTaskMaster,"vTaskMaster",TASK_MASTER_STACK_DEPTH,\
                TASK_MASTER_PARAMETERS,\
                TASK_MASTER_PRIORITY,NULL);

    xTaskCreate(vTaskGSM,"vTaskGSM",TASK_GSM_STACK_DEPTH,\
                TASK_GSM_PARAMETERS,\
                TASK_GSM_PRIORITY,&TASK_GSM_hHandlerTask);

    xTaskCreate(vTaskTerminal,"vTaskTERMINAL",TASK_TERMINAL_STACK_DEPTH,\
                TASK_TERMINAL_PARAMETERS,\
                TASK_TERMINAL_PRIORITY,&TASK_TERMINAL_hHandlerTask);

    // Blocking task
    vTaskSuspend( TASK_READ_SEN_hHandlerTask );

    // Blocking task
    vTaskSuspend( TASK_GSM_hHandlerTask );

    /* Start scheduler */
    printf("Start scheduler\r\n");
    osKernelStart();

    while(1)
    {
//        HAL_USART_Transmit(&UsartTLMHandle, dataTest, sizeof(dataTest),1000);
//        HAL_Delay(1000);
    }
}// end of main



//**************************************************************************************************
// @Function      _putchar()
//--------------------------------------------------------------------------------------------------
// @Description   Put char function used by printf.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void _putchar(char character)
{
    HAL_USART_Transmit(&UsartTLMHandle, &character, 1,1000);
//    INIT_TerminalSend(&character, 1);
}// end of _putchar



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      SystemClock_Config()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    /* MSI is enabled after System reset, activate PLL with MSI as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLP = 7;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        /* Initialization Error */
        while(1);
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        /* Initialization Error */
        while(1);
    }
} // end of SystemClock_Config()


//****************************************** end of file *******************************************