//**************************************************************************************************
// @Module        TASK_GSM
// @Filename      task_GSM.c
//--------------------------------------------------------------------------------------------------
// @Platform      STM32L4
//--------------------------------------------------------------------------------------------------
// @Compatible    STM32l476, STM32L452
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the TASK_GSM functionality.
//
//                Abbreviations:
//                  None.
//
//                Global (public) functions:
//                  None.
//
//                Local (private) functions:
//                  None.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    XXX         First release.
//**************************************************************************************************



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Native header
#include "task_GSM.h"

// Get mqtt stack
#include "core_mqtt.h"

// Get record manager interface
#include "record_manager.h"

#include "printf.h"
#include "string.h"
#include "ftoa.h"



//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// Task handler
TaskHandle_t TASK_GSM_hHandlerTask;



//**************************************************************************************************
// Declarations of local (private) data types
//**************************************************************************************************

// None.



//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

// Mutex delay
#define TASK_GSM_MUTEX_DELAY                (1000U)

// Size of send buffer
#define TASK_GSM_SIZE_OF_SEND_BUF           (0x800U)

// Size of printf buffer
#define TASK_GSM_SIZE_BUFF_PRINT            (64U)



//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// Buf for record
static uint8_t TASK_GSM_aDataRecord[RECORD_MAN_SIZE_OF_RECORD_BYTES];

// Transport interface for mqtt
static TransportInterface_t transport;

// Buffer passed to MQTT library.
static MQTTFixedBuffer_t fixedBuffer;

// A struct representing an MQTT connection.
static MQTTContext_t MQTT_Context;

// Send buffer.
static uint8_t TASK_GSM_SendBuffer[TASK_GSM_SIZE_OF_SEND_BUF];

// MQTT PUBLISH packet parameters.
static MQTTPublishInfo_t publishInfo;

// Current record
static RECORD_MAN_TYPE_RECORD RECORD_MAN_aRecord;

// Printf buffer
static char TASK_GSM_aBufferPrintf[TASK_GSM_SIZE_BUFF_PRINT];

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
// Declarations of local (private) functions
//**************************************************************************************************

// Send message to GSM module
static int32_t TASK_GSM_SendMessage(NetworkContext_t * pNetworkContext,
                                    const void * pBuffer,
                                    size_t bytesToSend);

// Receive message from GSM module
static int32_t TASK_GSM_ReceiveMessage(NetworkContext_t * pContext,
                                       void * pBuffer,
                                       size_t bytes);

// Get time stamp in ms
static uint32_t TASK_GSM_GetTimeStampMs(void);

// Callback function for receiving packets.
static void TASK_GSM_EventCallback(MQTTContext_t * pContext,
                                   MQTTPacketInfo_t * pPacketInfo,
                                   MQTTDeserializedInfo_t * pDeserializedInfo);

// Put string to GSM module
static void TASK_GSM_PutString(const char* s);

// Task delay
static void TASK_GSM_Delay(TickType_t ms);

// Send MQTT message to server
static void TASK_GSM_SendMQTTMessage(void);



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      vTaskGSM()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void vTaskGSM(void *pvParameters)
{
    uint32_t nQtyBytes = 0U;
    uint16_t packetId;
    MQTTContext_t* pContext;

    // Set transport interface members.
    transport.send = TASK_GSM_SendMessage;
    transport.recv = TASK_GSM_ReceiveMessage;

    // Set buffer members.
    fixedBuffer.pBuffer = TASK_GSM_SendBuffer;
    fixedBuffer.size = TASK_GSM_SIZE_OF_SEND_BUF;

    // Init MQTT
    MQTT_Init( &MQTT_Context,
               &transport,
               TASK_GSM_GetTimeStampMs,
               TASK_GSM_EventCallback,
               &fixedBuffer);
    MQTTConnectInfo_t connectInfo = { 0 };
    MQTTPublishInfo_t willInfo = { 0 };
    bool sessionPresent;

    // True for creating a new session with broker, false if we want to resume an old one.
    connectInfo.cleanSession = true;

    // Client ID must be unique to broker. This field is required.
    connectInfo.pClientIdentifier = "meteostation";
    connectInfo.clientIdentifierLength = strlen(connectInfo.pClientIdentifier);

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

    TASK_GSM_PutString( MQTT_AT_CIPSTATUS);
    TASK_GSM_Delay(2000);

    // Start up connection
    TASK_GSM_PutString( "AT+CIPSTART=\"TCP\",\"dev.rightech.io\",\"1883\"\r");

    //TASK_GSM_PutString( MQTT_AT_CIPSTART);
    TASK_GSM_Delay(8000);

    TASK_GSM_PutString( MQTT_AT_CIPSTATUS);
    TASK_GSM_Delay(2000);

    if (MQTTNotConnected == MQTT_Context.connectStatus)
    {
        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(2000);
        MQTT_Connect(&MQTT_Context,
                     &connectInfo,
                     &willInfo,
                     100,
                     &sessionPresent);

        USART_PutChar( 0x1a);
        TASK_GSM_Delay(2000);

        ftoa(ReceivedQueue.temperature, TASK_GSM_aBufferPrintf, 3);
        packetId = MQTT_GetPacketId(&MQTT_Context);
        publishInfo.qos = MQTTQoS0;
        publishInfo.pTopicName = "base/state/temperature";
        publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
        publishInfo.pPayload = TASK_GSM_aBufferPrintf;
        publishInfo.payloadLength = strlen(TASK_GSM_aBufferPrintf);
        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(4000);
        MQTT_Publish(&MQTT_Context, &publishInfo, packetId);
        USART_PutChar( 0x1a);
        TASK_GSM_Delay(4000);

        ftoa(ReceivedQueue.humidity, TASK_GSM_aBufferPrintf, 3);
        packetId = MQTT_GetPacketId(&MQTT_Context);
        publishInfo.qos = MQTTQoS0;
        publishInfo.pTopicName = "base/state/humidityyyy";
        publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
        publishInfo.pPayload = TASK_GSM_aBufferPrintf;
        publishInfo.payloadLength = strlen(TASK_GSM_aBufferPrintf);
        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(4000);
        MQTT_Publish(&MQTT_Context, &publishInfo, packetId);
        USART_PutChar( 0x1a);
        TASK_GSM_Delay(4000);

        ftoa(ReceivedQueue.pressure, TASK_GSM_aBufferPrintf, 3);
        packetId = MQTT_GetPacketId(&MQTT_Context);
        publishInfo.qos = MQTTQoS0;
        publishInfo.pTopicName = "base/state/pressureeee";
        publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
        publishInfo.pPayload = TASK_GSM_aBufferPrintf;
        publishInfo.payloadLength = strlen(TASK_GSM_aBufferPrintf);
        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(4000);
        MQTT_Publish(&MQTT_Context, &publishInfo, packetId);
        USART_PutChar( 0x1a);
        TASK_GSM_Delay(4000);

        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(4000);
        MQTT_Disconnect(&MQTT_Context);
        USART_PutChar( 0x1a);
        TASK_GSM_Delay(2000);


        TASK_GSM_PutString( MQTT_AT_CIPSTATUS);
        TASK_GSM_Delay(2000);
    }

    // Blocking MQTT task
    //TASK_GSM_Delay(1000);
    vTaskSuspend( HandleTask_MQTT );


    for(;;)
    {
        // Attempt get mutex
        if (pdTRUE == xSemaphoreTake(RECORD_MAN_xMutex, TASK_GSM_MUTEX_DELAY))
        {
            vTaskDelay(5000/portTICK_RATE_MS);
            // Store record
            if (RESULT_OK == RECORD_MAN_Load(0,
                                             TASK_GSM_aDataRecord,
                                              &nQtyBytes))
            {
                printf("Record Load OK\r\n");
                TASK_GSM_SendMQTTMessage();
            }
            else
            {
                printf("Record Load error\r\n");
            }

            // Return mutex
            xSemaphoreGive(RECORD_MAN_xMutex);
        }
        else
        {
            printf("TASK_GSM: Mutex of record manager is busy\r\n");
        }


        vTaskDelay(1000/portTICK_RATE_MS);
    }
} // end of vTaskGSM()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      TASK_GSM_SendMQTTMessage()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_GSM_SendMQTTMessage(void)
{

} // end of TASK_GSM_SendMQTTMessage()



//**************************************************************************************************
// @Function      TASK_GSM_SendMessage()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         If no data is transmitted over the network due to a full TX buffer and
//                no network error has occurred, this MUST return zero as the return value.
//                A zero return value SHOULD represent that the send operation can be retried by
//                calling the API function. Zero MUST NOT be returned if a network disconnection
//                has occurred.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   The number of bytes sent or a negative value to indicate error.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static int32_t TASK_GSM_SendMessage(NetworkContext_t * pNetworkContext,
                                    const void * pBuffer,
                                    size_t bytesToSend)
{
    HAL_USART_Transmit(&stUartGSMHandler,
                       pBuffer,
                       bytesToSend,
                       10000);

    return bytesToSend;
} // end of TASK_GSM_SendMessage()



//**************************************************************************************************
// @Function      TASK_GSM_ReceiveMessage()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static int32_t TASK_GSM_ReceiveMessage(NetworkContext_t * pContext,
                                        void * pBuffer,
                                        size_t bytes)
{
    return 0;
} // end of TASK_GSM_ReceiveMessage()



//**************************************************************************************************
// @Function      TASK_GSM_GetTimeStampMs()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static uint32_t TASK_GSM_GetTimeStampMs(void)
{
    return 0;
} // end of TASK_GSM_GetTimeStampMs()



//**************************************************************************************************
// @Function      TASK_GSM_EventCallback()
//--------------------------------------------------------------------------------------------------
// @Description   Callback function for receiving packets.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_GSM_EventCallback(MQTTContext_t * pContext,
                                   MQTTPacketInfo_t * pPacketInfo,
                                   MQTTDeserializedInfo_t * pDeserializedInfo)
{

} // end of TASK_GSM_EventCallback()



//**************************************************************************************************
// @Function      TASK_GSM_PutString()
//--------------------------------------------------------------------------------------------------
// @Description   Callback function for receiving packets.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_GSM_PutString(const char* s)
{
    uint32_t len = strlen(s);

    if (0 < len)
    {
        HAL_USART_Transmit(&stUartGSMHandler,
                           s,
                           len,
                           10000);

    }

}// end of TASK_GSM_PutString()



//**************************************************************************************************
// @Function      TASK_GSM_Delay()
//--------------------------------------------------------------------------------------------------
// @Description   Callback function for receiving packets.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_GSM_Delay(TickType_t ms)
{
    TickType_t currentTick = xTaskGetTickCount();

    while ((xTaskGetTickCount() - currentTick) < ms);
} // end of TASK_GSM_Delay()

//****************************************** end of file *******************************************
