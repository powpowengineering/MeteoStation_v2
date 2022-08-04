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
//#include "usart_drv.h"
#include "string.h"
// drivers
#include "printf.h"
#include "stdlib.h"
#include "ftoa.h"
#include "core_mqtt.h"
#include "task_read_sensors.h"
#include "queue.h"
extern xQueueHandle xQueueMeasureData;


//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None.


//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

TaskHandle_t HandleTask_MQTT;



//**************************************************************************************************
// Declarations of local (private) data types
//**************************************************************************************************

// None.


//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

#define MQTT_UART_CH            (1U)
#define MQTT_BUF_SIZE           (1024U)
#define TASK_SENS_RD_SIZE_BUFF_PRINT    (64U)

static const int8_t MQTT_AT[] = {"AT\r"};
static const int8_t MQTT_AT_CIPSTATUS[] = {"AT+CIPSTATUS\r"};
static const int8_t MQTT_AT_CSTT[] = {"AT+CSTT=\"internet\"\r"};
static const int8_t MQTT_AT_CIICR[] = {"AT+CIICR\r"};
static const int8_t MQTT_AT_CIFSR[] = {"AT+CIFSR\r"};
//static const int8_t MQTT_AT_CIPSTART[] = {"AT+CIPSTART=\"TCP\",\"dev.rightech.io\",\"1883\"\r"};
static const int8_t MQTT_AT_CIPSTART[] = {"AT+CIPSTART=\"TCP\",\"m7.wqtt.ru\",\"12542\"\r"};
static const int8_t MQTT_AT_CIPSEND[] = {"AT+CIPSEND\r"};
static const int8_t MQTT_AT_CIPSEND_QTY_SEND[] = {"AT+CIPSEND?\r"};
static const int8_t MQTT_AT_CIPQSEND[] = {"AT+CIPQSEND?\r"};
static const int8_t MQTT_AT_CIPMODE[] = {"AT+CIPMODE=1\r"};
static const int8_t MQTT_AT_CIPMUX[] = {"AT+CIPMUX=0\r"};
static const int8_t MQTT_AT_SAPBR_3_1[] = {"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r"};
static const int8_t MQTT_AT_SAPBR_3_1_APN[] = {"AT+SAPBR=3,1,\"APN\",\"internet\"\r"};
static const int8_t MQTT_AT_SAPBR_1_1_[] = {"AT+SAPBR=1,1\r"};
static const int8_t MQTT_AT_SAPBR_2_1_[] = {"AT+SAPBR=2,1\r"};

static const char MQTT_TYPE[] = {"MQIsdp"};
static const char MQTT_CID[] = {"meteostation"};
static const char MQTT_TOPIC[] = {"base/state/temperature"};
static const char MQTT_MESSAGE[] = {"23"};
static const char MQTT_USER_NAME[] = {"u_I8KUVV"};
static const char MQTT_PSW[] = {"D1MoanFH"};


//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

TransportInterface_t transport;
MQTTFixedBuffer_t fixedBuffer;
MQTTContext_t mqttContext;
uint8_t buffer[MQTT_BUF_SIZE];
MQTTPublishInfo_t publishInfo;

// GSM uart handler
USART_HandleTypeDef stUartGSMHandler;



//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

static int SIM800send(void* socket_info, const void* buf, unsigned int count);

static int32_t MQTTCoreSIM800send(NetworkContext_t * pNetworkContext,
                                  const void * pBuffer,
                                  size_t bytesToSend);

static int32_t networkRecv( NetworkContext_t * pContext, void * pBuffer, size_t bytes );

static uint32_t getTimeStampMs(void);

// Callback function for receiving packets.
static void eventCallback(
        MQTTContext_t * pContext,
        MQTTPacketInfo_t * pPacketInfo,
        MQTTDeserializedInfo_t * pDeserializedInfo);

static TASK_SENSOR_READ_DATA ReceivedQueue;

// printf buffer.
static char bufferPrintf[TASK_SENS_RD_SIZE_BUFF_PRINT];

void TaskDelay(TickType_t ms);

static void USART_PutString(const char* s);

static void USART_PutChar(const char character);


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
    uint16_t packetId;
    // Set transport interface members.
    //transport.pNetworkInterface = &someNetworkInterface;
    transport.send = MQTTCoreSIM800send;
    transport.recv = networkRecv;

    // Set buffer members.
    fixedBuffer.pBuffer = buffer;
    fixedBuffer.size = MQTT_BUF_SIZE;

    // Init GSM UART port
    __HAL_RCC_USART2_CLK_ENABLE();
    GPIO_InitTypeDef  GPIO_InitStruct = {0};

    // Configure the TX pin UART for GSM
    GPIO_InitStruct.Pin        = INIT_GSM_USART_TX_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLUP;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = INIT_GSM_USART_TX_AF;
    HAL_GPIO_Init(INIT_GSM_USART_TX_PORT, &GPIO_InitStruct);

    // Configure the RX pin UART for GSM
    GPIO_InitStruct.Pin        = INIT_GSM_USART_RX_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLUP;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = INIT_GSM_USART_RX_AF;
    HAL_GPIO_Init(INIT_GSM_USART_RX_PORT, &GPIO_InitStruct);


    stUartGSMHandler.Instance            = INIT_GSM_USART_NUM;
    stUartGSMHandler.Init.BaudRate       = 115200;
    stUartGSMHandler.Init.WordLength     = USART_WORDLENGTH_8B;
    stUartGSMHandler.Init.StopBits       = USART_STOPBITS_1;
    stUartGSMHandler.Init.Parity         = USART_PARITY_NONE;
    stUartGSMHandler.Init.Mode           = USART_MODE_TX_RX;

    HAL_USART_DeInit(&stUartGSMHandler);

    // Init UART GSM
    HAL_USART_Init(&stUartGSMHandler);

    // Init MQTT
    MQTT_Init( &mqttContext, &transport, getTimeStampMs, eventCallback, &fixedBuffer );

    MQTTConnectInfo_t connectInfo = { 0 };
    MQTTPublishInfo_t willInfo = { 0 };
    bool sessionPresent;

    // This is assumed to have been initialized before calling this function.
    MQTTContext_t * pContext;

    // True for creating a new session with broker, false if we want to resume an old one.
    connectInfo.cleanSession = true;

    // Client ID must be unique to broker. This field is required.
    connectInfo.pClientIdentifier = "meteostation";
    connectInfo.clientIdentifierLength = strlen( connectInfo.pClientIdentifier );

    // The following fields are optional.
    // Value for keep alive.
    connectInfo.keepAliveSeconds = 60;

    // Optional username and password.
    //connectInfo.pUserName = "u_I8KUVV";
    connectInfo.pUserName = NULL;
    //connectInfo.userNameLength = strlen( connectInfo.pUserName );
    connectInfo.userNameLength = 0;
    //connectInfo.pPassword = "D1MoanFH";
    connectInfo.pPassword = NULL;
    //connectInfo.passwordLength = strlen( connectInfo.pPassword );
    connectInfo.passwordLength = 0;

    // The last will and testament is optional, it will be published by the broker
    // should this client disconnect without sending a DISCONNECT packet.
    willInfo.qos = MQTTQoS0;
    willInfo.pTopicName = "base/state/humidity";
    willInfo.topicNameLength = strlen( willInfo.pTopicName );
    willInfo.pPayload = "100";
    willInfo.payloadLength = strlen( "100" );


  //  broker.socket_info = "SIM800";
  //  broker.send = SIM800send;

//    USART_PutString(MQTT_UART_CH, "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r");
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, "AT+SAPBR=3,1,\"APN\",\"internet\"\r");
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, "AT+SAPBR=1,1\r");
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, "AT+CIPSTART=\"TCP\",\"dev.rightech.io\",\"1883\"\r");
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, "AT+CIPSEND\r");
//    vTaskDelay(3000/portTICK_RATE_MS);



//    vTaskDelay(2000/portTICK_RATE_MS);
    TaskDelay(2000);
    // Whether some password is required or not
    USART_PutString("AT+CPIN?\r");

TaskDelay(2000);
    // Received signal strength
    USART_PutString( "AT+CSQ\r");
TaskDelay(2000);
    // The tegistration of the ME
    USART_PutString( "AT+CREG?\r");
TaskDelay(2000);
    // GPRS Service's status
    USART_PutString( "AT+CGATT?\r");
TaskDelay(2000);
    // Start task and set APN
    USART_PutString( "AT+CSTT=\"internet\"\r");
TaskDelay(2000);
    // Bring up wireless connection
    USART_PutString( "AT+CIICR\r");
TaskDelay(2000);
    // Get local IP address
    USART_PutString( "AT+CIFSR\r");
TaskDelay(2000);


//    USART_PutString( "AT+CIPSEND\r");
//    vTaskDelay(3000/portTICK_RATE_MS);


//    USART_PutString( MQTT_AT_SAPBR_3_1);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_SAPBR_3_1_APN);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_SAPBR_1_1_);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_SAPBR_2_1_);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//
//    USART_PutString( MQTT_AT_CIPMODE);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_CIPMUX);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_CIPSTATUS);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_CSTT);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_CIICR);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_CIFSR);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_CIPSTART);
//    vTaskDelay(5000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_CIPSEND_QTY_SEND);
//    vTaskDelay(1000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_CIPQSEND);
//    vTaskDelay(1000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_CIPSEND);
//    vTaskDelay(3000/portTICK_RATE_MS);

//
//    USART_PutString( MQTT_AT_CIPSTART);
//    vTaskDelay(5000/portTICK_RATE_MS);
//
//    USART_PutString( MQTT_AT_CIPSEND);
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
//    USART_PutRAWData( MQTT_AT_CONNECT,strlen(MQTT_CID)+12+2);
//    //USART_PutChar( 0x1a);
//
//    //USART_PutString( MQTT_AT_CIPSEND);
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
//    //USART_PutRAWData( MQTT_AT_CONNECT, 4+strlen(MQTT_TOPIC) + strlen(MQTT_MESSAGE) );
//    USART_PutRAWData( MQTT_AT_CONNECT, 4+strlen(MQTT_TOPIC) + 1 );
//
//    USART_PutChar( 0x1a);

//    mqtt_init(&broker, MQTT_CID);
//    mqtt_init_auth(&broker, MQTT_USER_NAME, MQTT_PSW);
//    mqtt_connect(&broker);
    //USART_PutChar(0x1a);
    //vTaskDelay(2000/portTICK_RATE_MS);

    //USART_PutString( MQTT_AT_CIPSEND);
    //vTaskDelay(1000/portTICK_RATE_MS);
  //  mqtt_publish(&broker, MQTT_TOPIC,MQTT_MESSAGE, 0);
  //  USART_PutChar(0x1a);
  //  vTaskDelay(3000/portTICK_RATE_MS);

//    MQTT_Connect( &mqttContext, &connectInfo, &willInfo, 100, &sessionPresent );
//    USART_PutChar(0x1a);
//    vTaskDelay(3000/portTICK_RATE_MS);


    while(1)
    {
        if (pdPASS == xQueueReceive( xQueueMeasureData, &ReceivedQueue,  10 / portTICK_RATE_MS ))
        {
            USART_PutString( MQTT_AT_CIPSTATUS);
            TaskDelay(2000);
            // Start up connection
            USART_PutString( "AT+CIPSTART=\"TCP\",\"dev.rightech.io\",\"1883\"\r");
            //USART_PutString( MQTT_AT_CIPSTART);
            TaskDelay(8000);

            USART_PutString( MQTT_AT_CIPSTATUS);
            TaskDelay(2000);

            if (MQTTNotConnected == mqttContext.connectStatus)
            {
                USART_PutString( MQTT_AT_CIPSEND);
               TaskDelay(2000);
                MQTT_Connect(&mqttContext, &connectInfo, &willInfo, 100, &sessionPresent);
                USART_PutChar( 0x1a);
               TaskDelay(2000);

                ftoa(ReceivedQueue.temperature, bufferPrintf, 3);
                packetId = MQTT_GetPacketId(&mqttContext);
                publishInfo.qos = MQTTQoS0;
                publishInfo.pTopicName = "base/state/temperature";
                publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
                publishInfo.pPayload = bufferPrintf;
                publishInfo.payloadLength = strlen(bufferPrintf);
                USART_PutString( MQTT_AT_CIPSEND);
               TaskDelay(4000);
                MQTT_Publish(&mqttContext, &publishInfo, packetId);
                USART_PutChar( 0x1a);
               TaskDelay(4000);

                ftoa(ReceivedQueue.humidity, bufferPrintf, 3);
                packetId = MQTT_GetPacketId(&mqttContext);
                publishInfo.qos = MQTTQoS0;
                publishInfo.pTopicName = "base/state/humidityyyy";
                publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
                publishInfo.pPayload = bufferPrintf;
                publishInfo.payloadLength = strlen(bufferPrintf);
                USART_PutString( MQTT_AT_CIPSEND);
               TaskDelay(4000);
                MQTT_Publish(&mqttContext, &publishInfo, packetId);
                USART_PutChar( 0x1a);
               TaskDelay(4000);

                ftoa(ReceivedQueue.pressure, bufferPrintf, 3);
                packetId = MQTT_GetPacketId(&mqttContext);
                publishInfo.qos = MQTTQoS0;
                publishInfo.pTopicName = "base/state/pressureeee";
                publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
                publishInfo.pPayload = bufferPrintf;
                publishInfo.payloadLength = strlen(bufferPrintf);
                USART_PutString( MQTT_AT_CIPSEND);
                TaskDelay(4000);
                MQTT_Publish(&mqttContext, &publishInfo, packetId);
                USART_PutChar( 0x1a);
                TaskDelay(4000);

                USART_PutString( MQTT_AT_CIPSEND);
               TaskDelay(4000);
                MQTT_Disconnect(&mqttContext);
                USART_PutChar( 0x1a);
               TaskDelay(2000);


                USART_PutString( MQTT_AT_CIPSTATUS);
               TaskDelay(2000);
            }
        }
        // Blocking MQTT task
       //TaskDelay(1000);
        vTaskSuspend( HandleTask_MQTT );

    }
}// end of vTaskMQTT



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

static int SIM800send(void* socket_info, const void* buf, unsigned int count)
{
//    USART_PutRAWData( buf, count);
    //USART_PutChar(0x1a);
    HAL_USART_Transmit(&stUartGSMHandler,
                       buf,
                       count,
                    10000);

    return count;
}

static int32_t MQTTCoreSIM800send(NetworkContext_t * pNetworkContext,
                              const void * pBuffer,
                              size_t bytesToSend)
{
//    USART_PutRAWData( pBuffer, bytesToSend);
    //USART_PutChar(0x1a);
    HAL_USART_Transmit(&stUartGSMHandler,
                       pBuffer,
                       bytesToSend,
                       10000);

    return bytesToSend;
}

static int32_t networkRecv( NetworkContext_t * pContext, void * pBuffer, size_t bytes )
{
    return 0;
}

// Callback function for receiving packets.
static void eventCallback(
                            MQTTContext_t * pContext,
                            MQTTPacketInfo_t * pPacketInfo,
                            MQTTDeserializedInfo_t * pDeserializedInfo)
{

}

static uint32_t getTimeStampMs(void)
{
    return 0;
}

void TaskDelay(TickType_t ms)
{
    TickType_t currentTick = xTaskGetTickCount();

    while ((xTaskGetTickCount() - currentTick) < ms);
}

static void USART_PutString(const char* s)
{
    uint32_t len = strlen(s);

    if (0 < len)
    {
        HAL_USART_Transmit(&stUartGSMHandler,
                           s,
                           len,
                           10000);

    }

}// end of USART_PutString()

static void USART_PutChar( const char character)
{
    HAL_USART_Transmit(&stUartGSMHandler,
                       &character,
                       1,
                       10000);
}



//****************************************** end of file *******************************************








