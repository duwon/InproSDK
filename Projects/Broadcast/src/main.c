/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include "LoraMessage.h"
#include "PayloadMessage.h"
#include "bsp.h"

#define RF_FREQUENCY                                920000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
    TX_DONE,
    RX_DONE
}States_t;

#define RX_TIMEOUT_VALUE                            1000
#define TX_INTERVAL_TIME                            20000

States_t State = LOWPOWER;

static  TimerEvent_t timerLed; /* Led Timers objects */
static  TimerEvent_t timerTx; /* Tx Timers objects */

/* Private function prototypes -----------------------------------------------*/
static RadioEvents_t RadioEvents; /* Radio events function pointer */

void OnTxDone( void ); /* brief Function to be executed on Radio Tx Done event */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ); /* brief Function to be executed on Radio Rx Done event */
void OnTxTimeout( void ); /* brief Function executed on Radio Tx Timeout event */
void OnRxTimeout( void ); /* brief Function executed on Radio Rx Timeout event */
void OnRxError( void ); /* brief Function executed on Radio Rx Error event */
static void RadioInit(void);
static void OnledEvent( void* context ); /* brief Function executed on when led timer elapses */
static void OnTxEvent( void* context );
static void mainTimerInit(void);
static void procLoraStage(void);

int main( void )
{
    HAL_Init( );
    SystemClock_Config( );

    DBG_Init( );
    HW_Init( );  
    mainTimerInit( );
    pyaloadTimerInit();

    LPM_SetOffMode(LPM_APPLI_Id , LPM_Disable ); /*Disbale Stand-by mode*/

    RadioInit();
    Radio.Rx( RX_TIMEOUT_VALUE );

    initMessage();    


    /* create random uid */
    sensor_t sensor_data;
    BSP_sensor_Read( &sensor_data );
    srand(sensor_data.temperature * sensor_data.humidity);
    UID_radom = rand();
    PRINTF("UID : %x\r\n", UID_radom);
    InsertIDList(UID_radom);


    while( 1 )
    {
        procLoraStage();  

        if(isMasterMode == true)
        {
            procMasterMode();
        }
        else if(existGetID == true)
        {
            procPayloadData();


#ifdef _DEBUG_
            //Message Test   
            uint8_t tempSrcID;
            uint8_t tempRxBuffer[MESSAGE_MAX_PAYLOAD_SIZE] = {0,};
            if(getMessagePayload((void*)&tempSrcID, tempRxBuffer) == SUCCESS)
            {
                if(tempRxBuffer[0] == MTYPE_TESTMESSAGE)
                {
                    PRINTF("ID : %d  T : %d\r\n",tempRxBuffer[1], tempRxBuffer[2]);
                }
            }
#endif

            DISABLE_IRQ( );
            if (State == LOWPOWER) /* if an interupt has occured after __disable_irq, it is kept pending and cortex will not enter low power anyway */
            {
                LPM_EnterLowPower( );
            }
            else
            {
            }
            ENABLE_IRQ( );
        }        
    }
}

void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX_DONE;
    PRINTF("        OnTxDone\n\r");
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    messagePacket_TypeDef nextMessage;
    destID = payload[4];

    uint8_t errorCode = putMessageBuffer(&rxMessageBuffer, payload, size, rssi, snr);
    if( errorCode != PUT_SUCCESS)
    {
        PRINTF("Error code %d\r\n", errorCode);
    }/*
    else if((existNextMessage(destID, &nextMessage) == true) && (isMasterMode == true))
    {
        sendMessage(nextMessage.payload, nextMessage.payloadSize);
    }
    else if(isMasterMode == true)
    {
        sendMessage(payload,size);
    }*/

    State = RX_DONE;

    //PRINTF("OnRxDone\n\r");
    //PRINTF("RssiValue=%d dBm, SnrValue=%d\n\r", rssi, snr);
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;

    PRINTF("OnTxTimeout\n\r");
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    State = RX_TIMEOUT;
    PRINTF("OnRxTimeout\n\r");
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
    PRINTF("OnRxError\n\r");
}

static void RadioInit(void)
{
    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init(&RadioEvents);

    Radio.SetChannel(RF_FREQUENCY);

    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
}

static void OnledEvent(void *context)
{
    //LED_Toggle( LED1 ) ;
    TimerStart(&timerLed);
}

static void OnTxEvent(void *context)
{
    if (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_RESET)
    {
        srcID = MASTER_ID;
        isMasterMode = true;
        PRINTF("key pressed. ID:%d \r\n", srcID);
        payloadTimerDeInit();
    }

    if (isMasterMode == true)
    {
        TimerStop(&timerTx);
    }
    else
    {
        TimerStart(&timerTx);
    }

    sensor_t sensor_data;
    uint8_t tempTxData[8] = {0,};

    BSP_sensor_Read( &sensor_data );
    memcpy((void *)&tempTxData[0], (void *)&sensor_data.temperature, 4);
    memcpy((void *)&tempTxData[4], (void *)&sensor_data.humidity, 4);

    PRINTF("Temp : %.1f     Humi : %.1f   \r\n\r\n",sensor_data.temperature, sensor_data.humidity);
    sendPayloadData(MTYPE_TEMP_HUMI, tempTxData);



}

static void mainTimerInit(void)
{
#ifdef _DEBUG_
    /* Led Timers */
    TimerInit(&timerLed, OnledEvent);
    TimerSetValue(&timerLed, 1000);
    TimerStart(&timerLed);
#endif

    /* Tx Timers */
    TimerInit(&timerTx, OnTxEvent);
    TimerSetValue(&timerTx, TX_INTERVAL_TIME);
    TimerStart(&timerTx);
}

static void procLoraStage(void)
{
    switch (State)
    {
    case RX_DONE:
        if (isMasterMode == true)
        {
            Radio.Rx(RX_TIMEOUT_VALUE);
        }

#ifdef _DEBUG_
        LED_Toggle(LED1);
#endif
        State = LOWPOWER;
        break;
    case TX_DONE:
        Radio.Rx(RX_TIMEOUT_VALUE);
#ifdef _DEBUG_
        LED_Toggle(LED2);
#endif
        State = LOWPOWER;
        break;
    case RX_TIMEOUT:
    case RX_ERROR:
        if (isMasterMode == true)
        {
            Radio.Rx(RX_TIMEOUT_VALUE);
        }

        State = LOWPOWER;
        break;
    case TX_TIMEOUT:

        State = LOWPOWER;
        break;
    case LOWPOWER:
    default:
        // Set low power
        break;
    }
}
