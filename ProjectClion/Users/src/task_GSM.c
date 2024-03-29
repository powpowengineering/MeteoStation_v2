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

// Get eeprom interface
#include "eeprom_emulation.h"

#include "printf.h"
#include "string.h"
#include "ftoa.h"
#include "Init.h"



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
#define TASK_GSM_MUTEX_DELAY                (0U)

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

// Put char function
static void TASK_GSM_PutChar(const char character);

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
static void TASK_GSM_SendMQTTMessage(RECORD_MAN_TYPE_RECORD stRecord);



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
    uint32_t nNextRecord = 0U;

    // Set transport interface members.
    transport.send = TASK_GSM_SendMessage;
    transport.recv = TASK_GSM_ReceiveMessage;

    // Set buffer members.
    fixedBuffer.pBuffer = TASK_GSM_SendBuffer;
    fixedBuffer.size = TASK_GSM_SIZE_OF_SEND_BUF;

    for(;;)
    {

        // Attempt get mutex
        if (pdTRUE == xSemaphoreTake(RECORD_MAN_xMutex, TASK_GSM_MUTEX_DELAY))
        {
            // Power GSM ON
            HAL_GPIO_WritePin(INIT_DC_GSM_PORT, INIT_DC_GSM_PIN, GPIO_PIN_RESET);

            // Get the next - 1 record
            EMEEP_Load(RECORD_MAN_VIR_ADR32_NEXT_RECORD,
                       (U8*)&nNextRecord,
                       RECORD_MAN_SIZE_VIR_ADR);

            if (0 != nNextRecord)
            {
                nNextRecord -= 1U;
            }
            else
            {
                DoNothing();
            }

            // Load record
            if (RESULT_OK == RECORD_MAN_Load(nNextRecord,
                                             TASK_GSM_aDataRecord,
                                              &nQtyBytes))
            {
                printf("Record Load OK\r\n");
                printf("Sending data to the server...\r\n");
                TASK_GSM_SendMQTTMessage(*(RECORD_MAN_TYPE_RECORD*)TASK_GSM_aDataRecord);
            }
            else
            {
                printf("Record Load error\r\n");
            }


            // Update last record number
            EMEEP_Store(RECORD_MAN_VIR_ADR32_LAST_RECORD,
                        (U8*)&nNextRecord,
                        RECORD_MAN_SIZE_VIR_ADR);

            // Return mutex
            xSemaphoreGive(RECORD_MAN_xMutex);
        }
        else
        {
            printf("TASK_GSM: Mutex of record manager is busy\r\n");
        }

        // Power GSM OFF
        HAL_GPIO_WritePin(INIT_DC_GSM_PORT, INIT_DC_GSM_PIN, GPIO_PIN_SET);

        // Blocking task GSM
        vTaskSuspend(TASK_GSM_hHandlerTask);
//        vTaskDelay(1000/portTICK_RATE_MS);
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
static void TASK_GSM_SendMQTTMessage(RECORD_MAN_TYPE_RECORD stRecord)
{
    uint16_t packetId;

    TASK_GSM_Delay(8000);

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
//    connectInfo.pClientIdentifier = "meteostation";
    connectInfo.pClientIdentifier = SECRET_MQTT_CLIENT_ID;
    connectInfo.clientIdentifierLength = strlen(connectInfo.pClientIdentifier);

    // The following fields are optional.
    // Value for keep alive.
    connectInfo.keepAliveSeconds = 60;
    // Optional username and password.
    connectInfo.pUserName = SECRET_MQTT_USERNAME;
//    connectInfo.pUserName = NULL;
    connectInfo.userNameLength = strlen( connectInfo.pUserName );
//    connectInfo.userNameLength = 0;
    connectInfo.pPassword = SECRET_MQTT_PASSWORD;
//    connectInfo.pPassword = NULL;
    connectInfo.passwordLength = strlen( connectInfo.pPassword );
//    connectInfo.passwordLength = 0;

    // The last will and testament is optional, it will be published by the broker
    // should this client disconnect without sending a DISCONNECT packet.
    willInfo.qos = MQTTQoS0;
//    willInfo.pTopicName = "base/state/humidity";
    willInfo.pTopicName = "channels/1843720/publish/fields/field1";
    willInfo.topicNameLength = strlen( willInfo.pTopicName );
    willInfo.pPayload = "25";
    willInfo.payloadLength = strlen( "100" );

    TASK_GSM_PutString( MQTT_AT_CIPSTATUS);
    TASK_GSM_Delay(2000);

    // Start up connection
//    TASK_GSM_PutString( "AT+CIPSTART=\"TCP\",\"dev.rightech.io\",\"1883\"\r");
    TASK_GSM_PutString( "AT+CIPSTART=\"TCP\",\"mqtt3.thingspeak.com\",\"1883\"\r");

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

        TASK_GSM_PutChar( 0x1a);
        TASK_GSM_Delay(2000);

        ftoa(stRecord.fTemperature, TASK_GSM_aBufferPrintf, 3);
        packetId = MQTT_GetPacketId(&MQTT_Context);
        publishInfo.qos = MQTTQoS0;
        publishInfo.pTopicName = "channels/1851639/publish/fields/field1";
        publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
        publishInfo.pPayload = TASK_GSM_aBufferPrintf;
        publishInfo.payloadLength = strlen(TASK_GSM_aBufferPrintf);
        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(4000);
        MQTT_Publish(&MQTT_Context, &publishInfo, packetId);
        TASK_GSM_PutChar( 0x1a);
        TASK_GSM_Delay(4000);

        ftoa(stRecord.fHumidity, TASK_GSM_aBufferPrintf, 3);
        packetId = MQTT_GetPacketId(&MQTT_Context);
        publishInfo.qos = MQTTQoS0;
        publishInfo.pTopicName = "channels/1851639/publish/fields/field2";
        publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
        publishInfo.pPayload = TASK_GSM_aBufferPrintf;
        publishInfo.payloadLength = strlen(TASK_GSM_aBufferPrintf);
        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(4000);
        MQTT_Publish(&MQTT_Context, &publishInfo, packetId);
        TASK_GSM_PutChar( 0x1a);
        TASK_GSM_Delay(4000);

        ftoa(stRecord.fPressure, TASK_GSM_aBufferPrintf, 3);
        packetId = MQTT_GetPacketId(&MQTT_Context);
        publishInfo.qos = MQTTQoS0;
        publishInfo.pTopicName = "channels/1851639/publish/fields/field3";
        publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
        publishInfo.pPayload = TASK_GSM_aBufferPrintf;
        publishInfo.payloadLength = strlen(TASK_GSM_aBufferPrintf);
        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(4000);
        MQTT_Publish(&MQTT_Context, &publishInfo, packetId);
        TASK_GSM_PutChar( 0x1a);
        TASK_GSM_Delay(4000);

        ftoa(stRecord.fBatteryVoltage, TASK_GSM_aBufferPrintf, 3);
        packetId = MQTT_GetPacketId(&MQTT_Context);
        publishInfo.qos = MQTTQoS0;
        publishInfo.pTopicName = "channels/1851639/publish/fields/field4";
        publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
        publishInfo.pPayload = TASK_GSM_aBufferPrintf;
        publishInfo.payloadLength = strlen(TASK_GSM_aBufferPrintf);
        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(4000);
        MQTT_Publish(&MQTT_Context, &publishInfo, packetId);
        TASK_GSM_PutChar( 0x1a);
        TASK_GSM_Delay(4000);

        ftoa(stRecord.fWindSpeed, TASK_GSM_aBufferPrintf, 3);
        packetId = MQTT_GetPacketId(&MQTT_Context);
        publishInfo.qos = MQTTQoS0;
        publishInfo.pTopicName = "channels/1851639/publish/fields/field5";
        publishInfo.topicNameLength = strlen(publishInfo.pTopicName);
        publishInfo.pPayload = TASK_GSM_aBufferPrintf;
        publishInfo.payloadLength = strlen(TASK_GSM_aBufferPrintf);
        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(4000);
        MQTT_Publish(&MQTT_Context, &publishInfo, packetId);
        TASK_GSM_PutChar( 0x1a);
        TASK_GSM_Delay(4000);

        TASK_GSM_PutString( MQTT_AT_CIPSEND);
        TASK_GSM_Delay(4000);
        MQTT_Disconnect(&MQTT_Context);
        TASK_GSM_PutChar( 0x1a);
        TASK_GSM_Delay(2000);

        TASK_GSM_PutString( MQTT_AT_CIPSTATUS);
        TASK_GSM_Delay(2000);

    }

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
    HAL_UART_Transmit(&UartGSMHandler,
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
        HAL_UART_Transmit(&UartGSMHandler,
                           s,
                           len,
                           10000);

    }

}// end of TASK_GSM_PutString()



//**************************************************************************************************
// @Function      TASK_GSM_PutChar()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
static void TASK_GSM_PutChar(const char character)
{
    HAL_UART_Transmit(&UartGSMHandler,
                       &character,
                       1,
                       10000);
} // end of TASK_GSM_PutChar()




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
