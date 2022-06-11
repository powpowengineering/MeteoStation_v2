//**************************************************************************************************
// @Module        TASK_TEST_FLASH
// @Filename      task_test_flash.c
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
#include "task_mqtt.h"
#include "usart_drv.h"
#include "string.h"
// drivers
#include "printf.h"
#include "stdlib.h"
#include "ftoa.h"
#include "libemqtt.h"
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

#define MQTT_UART_CH            (1U)
static const int8_t MQTT_AT[] = {"AT\r"};
static const int8_t MQTT_AT_CIPSTATUS[] = {"AT+CIPSTATUS\r"};
static const int8_t MQTT_AT_CSTT[] = {"AT+CSTT=\"internet\"\r"};
static const int8_t MQTT_AT_CIICR[] = {"AT+CIICR\r"};
static const int8_t MQTT_AT_CIFSR[] = {"AT+CIFSR\r"};
static const int8_t MQTT_AT_CIPSTART[] = {"AT+CIPSTART=\"TCP\",\"dev.rightech.io\",\"1883\"\r"};
//static const int8_t MQTT_AT_CIPSTART[] = {"AT+CIPSTART=\"TCP\",\"m7.wqtt.ru\",\"12542\"\r"};
static const int8_t MQTT_AT_CIPSEND[] = {"AT+CIPSEND\r"};
static const int8_t MQTT_AT_CIPSEND_QTY_SEND[] = {"AT+CIPSEND?\r"};
static const int8_t MQTT_AT_CIPQSEND[] = {"AT+CIPQSEND?\r"};
static const int8_t MQTT_AT_CIPMODE[] = {"AT+CIPMODE=0\r"};
static const int8_t MQTT_AT_CIPMUX[] = {"AT+CIPMUX=0\r"};
static const int8_t MQTT_AT_SAPBR_3_1[] = {"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r"};
static const int8_t MQTT_AT_SAPBR_3_1_APN[] = {"AT+SAPBR=3,1,\"APN\",\"internet\"\r"};
static const int8_t MQTT_AT_SAPBR_1_1_[] = {"AT+SAPBR=1,1\r"};
static const int8_t MQTT_AT_SAPBR_2_1_[] = {"AT+SAPBR=2,1\r"};

static const char MQTT_TYPE[] = {"MQIsdp"};
static const char MQTT_CID[] = {"meteostation"};
static const char MQTT_TOPIC[] = {"base/state/temperature"};
static const char MQTT_MESSAGE[] = {"+23"};
static const char MQTT_USER_NAME[] = {"u_I8KUVV"};
static const char MQTT_PSW[] = {"D1MoanFH"};

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// Mqtt packet
int8_t MQTT_AT_CONNECT[64];
// broker MQTT
mqtt_broker_handle_t broker;


//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

static int SIM800send(void* socket_info, const void* buf, unsigned int count);



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************


//**************************************************************************************************
// @Function      vTaskMQTT()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void vTaskMQTT(void *pvParameters)
{
    broker.socket_info = "SIM800";
    broker.send = SIM800send;

//    USART_PutString(MQTT_UART_CH, MQTT_AT_SAPBR_3_1);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_SAPBR_3_1_APN);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_SAPBR_1_1_);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_SAPBR_2_1_);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//
    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPMODE);
    vTaskDelay(2000/portTICK_RATE_MS);

    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPMUX);
    vTaskDelay(2000/portTICK_RATE_MS);

    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSTATUS);
    vTaskDelay(2000/portTICK_RATE_MS);

    USART_PutString(MQTT_UART_CH, MQTT_AT_CSTT);
    vTaskDelay(2000/portTICK_RATE_MS);

    USART_PutString(MQTT_UART_CH, MQTT_AT_CIICR);
    vTaskDelay(2000/portTICK_RATE_MS);

    USART_PutString(MQTT_UART_CH, MQTT_AT_CIFSR);
    vTaskDelay(2000/portTICK_RATE_MS);

    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSTART);
    vTaskDelay(5000/portTICK_RATE_MS);

    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND_QTY_SEND);
    vTaskDelay(1000/portTICK_RATE_MS);

    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPQSEND);
    vTaskDelay(1000/portTICK_RATE_MS);

    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND);
    vTaskDelay(3000/portTICK_RATE_MS);



//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSTART);
//    vTaskDelay(5000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND);
//    vTaskDelay(3000/portTICK_RATE_MS);

//    // Connect mqtt
//    MQTT_AT_CONNECT[0] = 0x10;
//    MQTT_AT_CONNECT[1] = strlen(MQTT_CID)+12;
//    MQTT_AT_CONNECT[2] = 0x00;
//    MQTT_AT_CONNECT[3] = strlen(MQTT_TYPE);
//    MQTT_AT_CONNECT[4] = 'M';//MQIsdp
//    MQTT_AT_CONNECT[5] = 'Q';
//    MQTT_AT_CONNECT[6] = 'I';
//    MQTT_AT_CONNECT[7] = 's';
//    MQTT_AT_CONNECT[8] = 'd';
//    MQTT_AT_CONNECT[9] = 'p';
//    MQTT_AT_CONNECT[10] = 0x03; // Protocol version number
//    MQTT_AT_CONNECT[11] = 0x02; // Clean session flag
//    MQTT_AT_CONNECT[12] = 0x00; // Keep Alive timer MSB
//    MQTT_AT_CONNECT[13] = 0x0A; // Keep Alive timer LSB
//
//    memcpy(&MQTT_AT_CONNECT[14],MQTT_CID, strlen(MQTT_CID));
//
//    USART_PutRAWData(MQTT_UART_CH, MQTT_AT_CONNECT,strlen(MQTT_CID)+12+2);
//    //USART_PutChar(MQTT_UART_CH, 0x1a);
//
//    //USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND);
//    //vTaskDelay(3000/portTICK_RATE_MS);
//
//    // pub mqtt
//    MQTT_AT_CONNECT[0] = 0x30; // Fixed header
//    //MQTT_AT_CONNECT[1] = strlen(MQTT_TOPIC) + strlen(MQTT_MESSAGE)+2; // Remaining length
//    MQTT_AT_CONNECT[1] = strlen(MQTT_TOPIC) + 1+2; // Remaining length
//    MQTT_AT_CONNECT[2] = 0x0;// Len Topic name MSB
//    MQTT_AT_CONNECT[3] = strlen(MQTT_TOPIC);// Len Topic name LSB
//    memcpy(&MQTT_AT_CONNECT[4],MQTT_TOPIC, strlen(MQTT_TOPIC));// Topic name
//    memcpy(&MQTT_AT_CONNECT[4+strlen(MQTT_TOPIC)],MQTT_MESSAGE, strlen(MQTT_MESSAGE)); // Message
//    //MQTT_AT_CONNECT[4+strlen(MQTT_TOPIC)] = 23; // Message
//
//    //USART_PutRAWData(MQTT_UART_CH, MQTT_AT_CONNECT, 4+strlen(MQTT_TOPIC) + strlen(MQTT_MESSAGE) );
//    USART_PutRAWData(MQTT_UART_CH, MQTT_AT_CONNECT, 4+strlen(MQTT_TOPIC) + 1 );
//
//    USART_PutChar(MQTT_UART_CH, 0x1a);

    mqtt_init(&broker, MQTT_CID);
    //mqtt_init_auth(&broker, MQTT_USER_NAME, MQTT_PSW);
    mqtt_connect(&broker);
    //USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND);
    mqtt_publish(&broker, MQTT_TOPIC,MQTT_MESSAGE, 0);
    USART_PutChar(MQTT_UART_CH,0x1a);

    while(1)
    {
      //  mqtt_publish(&broker, MQTT_TOPIC,MQTT_MESSAGE, 0);
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}// end of vTaskMQTT



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

static int SIM800send(void* socket_info, const void* buf, unsigned int count)
{
    USART_PutRAWData(MQTT_UART_CH, buf, count);
    //USART_PutChar(MQTT_UART_CH,0x1a);
    return count;
}

//****************************************** end of file *******************************************








