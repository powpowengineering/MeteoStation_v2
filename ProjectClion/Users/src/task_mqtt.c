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
#include "core_mqtt.h"


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
#define MQTT_BUF_SIZE           (1024U)

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
    willInfo.pTopicName = "base/state/temperature";
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

    vTaskDelay(10000/portTICK_RATE_MS);
    // Whether some password is required or not
    USART_PutString(MQTT_UART_CH, "AT+CPIN?\r");
    vTaskDelay(2000/portTICK_RATE_MS);

    // Received signal strength
    USART_PutString(MQTT_UART_CH, "AT+CSQ\r");
    vTaskDelay(2000/portTICK_RATE_MS);

    // The tegistration of the ME
    USART_PutString(MQTT_UART_CH, "AT+CREG?\r");
    vTaskDelay(2000/portTICK_RATE_MS);

    // GPRS Service's status
    USART_PutString(MQTT_UART_CH, "AT+CGATT?\r");
    vTaskDelay(2000/portTICK_RATE_MS);

    // Start task and set APN
    USART_PutString(MQTT_UART_CH, "AT+CSTT=\"internet\"\r");
    vTaskDelay(2000/portTICK_RATE_MS);

    // Bring up wireless connection
    USART_PutString(MQTT_UART_CH, "AT+CIICR\r");
    vTaskDelay(2000/portTICK_RATE_MS);

    // Get local IP address
    USART_PutString(MQTT_UART_CH, "AT+CIFSR\r");
    vTaskDelay(2000/portTICK_RATE_MS);



//    USART_PutString(MQTT_UART_CH, "AT+CIPSEND\r");
//    vTaskDelay(3000/portTICK_RATE_MS);


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
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPMODE);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPMUX);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSTATUS);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CSTT);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIICR);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIFSR);
//    vTaskDelay(2000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSTART);
//    vTaskDelay(5000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND_QTY_SEND);
//    vTaskDelay(1000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPQSEND);
//    vTaskDelay(1000/portTICK_RATE_MS);
//
//    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND);
//    vTaskDelay(3000/portTICK_RATE_MS);

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

//    mqtt_init(&broker, MQTT_CID);
//    mqtt_init_auth(&broker, MQTT_USER_NAME, MQTT_PSW);
//    mqtt_connect(&broker);
    //USART_PutChar(MQTT_UART_CH,0x1a);
    //vTaskDelay(2000/portTICK_RATE_MS);

    //USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND);
    //vTaskDelay(1000/portTICK_RATE_MS);
  //  mqtt_publish(&broker, MQTT_TOPIC,MQTT_MESSAGE, 0);
  //  USART_PutChar(MQTT_UART_CH,0x1a);
  //  vTaskDelay(3000/portTICK_RATE_MS);

//    MQTT_Connect( &mqttContext, &connectInfo, &willInfo, 100, &sessionPresent );
//    USART_PutChar(MQTT_UART_CH,0x1a);
//    vTaskDelay(3000/portTICK_RATE_MS);

    USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSTATUS);
    vTaskDelay(2000/portTICK_RATE_MS);

    float cnt = 0;
    char buf[30];
    while(1)
    {

        // Start up connection
        USART_PutString(MQTT_UART_CH, "AT+CIPSTART=\"TCP\",\"dev.rightech.io\",\"1883\"\r");
        //USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSTART);
        vTaskDelay(8000/portTICK_RATE_MS);

        USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSTATUS);
        vTaskDelay(2000/portTICK_RATE_MS);

        if (MQTTNotConnected == mqttContext.connectStatus)
        {
            USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND);
            vTaskDelay(2000/portTICK_RATE_MS);
            MQTT_Connect( &mqttContext, &connectInfo, &willInfo, 100, &sessionPresent );
            USART_PutChar(MQTT_UART_CH,0x1a);
            vTaskDelay(2000/portTICK_RATE_MS);

            ftoa(cnt, buf, 1);
            packetId = MQTT_GetPacketId( &mqttContext );
            publishInfo.qos = MQTTQoS0;
            publishInfo.pTopicName = "base/state/meteostation";
            publishInfo.topicNameLength = strlen( publishInfo.pTopicName );
            publishInfo.pPayload = buf;
            publishInfo.payloadLength = strlen( buf );
            USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND);
            vTaskDelay(2000/portTICK_RATE_MS);
            MQTT_Publish( &mqttContext, &publishInfo, packetId );
            USART_PutChar(MQTT_UART_CH,0x1a);
            vTaskDelay(2000/portTICK_RATE_MS);

            USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND);
            vTaskDelay(2000/portTICK_RATE_MS);
            MQTT_Disconnect( &mqttContext );
            USART_PutChar(MQTT_UART_CH,0x1a);
            vTaskDelay(2000/portTICK_RATE_MS);
            cnt += 0.1F;

            USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSTATUS);
            vTaskDelay(2000/portTICK_RATE_MS);
        }



//        USART_PutString(MQTT_UART_CH, MQTT_AT_CIPSEND);
//        vTaskDelay(3000/portTICK_RATE_MS);
//        MQTT_Disconnect( &mqttContext);
//        USART_PutChar(MQTT_UART_CH,0x1a);
        vTaskDelay(3000/portTICK_RATE_MS);
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

static int32_t MQTTCoreSIM800send(NetworkContext_t * pNetworkContext,
                              const void * pBuffer,
                              size_t bytesToSend)
{
    USART_PutRAWData(MQTT_UART_CH, pBuffer, bytesToSend);
    //USART_PutChar(MQTT_UART_CH,0x1a);
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

//****************************************** end of file *******************************************







