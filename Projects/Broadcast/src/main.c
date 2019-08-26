/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include "LoraMessage.h"

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

#define RX_TIMEOUT_VALUE                            3000
#define BUFFER_SIZE                                 32          // Define the payload size here

uint8_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
uint8_t BufferRx[BUFFER_SIZE];

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
static void TimeServerInit(void);
static void procLoraStage(void);

int main( void )
{
    HAL_Init( );
    SystemClock_Config( );

    DBG_Init( );
    HW_Init( );  
    TimeServerInit( );

    LPM_SetOffMode(LPM_APPLI_Id , LPM_Disable ); /*Disbale Stand-by mode*/

    RadioInit();
    Radio.Rx( RX_TIMEOUT_VALUE );

    while( 1 )
    {
        procLoraStage();
        //parseMessage();        /* call message parsing function */

        DISABLE_IRQ( );
        if (State == LOWPOWER) /* if an interupt has occured after __disable_irq, it is kept pending and cortex will not enter low power anyway */
        {
            #ifndef LOW_POWER_DISABLE
                LPM_EnterLowPower( );
            #endif
        }
        else
        {
        }
        ENABLE_IRQ( );
    }
}

void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX_DONE;
    PRINTF("OnTxDone\n\r");
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    putMessageBuffer(&rxMessageBuffer, payload, size);
    
    RssiValue = rssi;
    SnrValue = snr;
    State = RX_DONE;

    PRINTF("OnRxDone\n\r");
    PRINTF("RssiValue=%d dBm, SnrValue=%d\n\r", rssi, snr);
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

static void OnledEvent( void* context )
{
  //LED_Toggle( LED1 ) ; 

  TimerStart(&timerLed );
}

static void OnTxEvent( void* context )
{
  static uint8_t cntTx = 0;
  uint8_t tempBuffer[5] = {0,};
  tempBuffer[0] = cntTx++;
  sendMessage(tempBuffer,1);

  //LED_Toggle( LED2 ) ;

  TimerStart(&timerTx );
}

static void TimeServerInit(void)
{
    /* Led Timers */
    TimerInit(&timerLed, OnledEvent);   
    TimerSetValue( &timerLed, 1000);

    TimerStart(&timerLed );


    /* Tx Timers */
    TimerInit(&timerTx, OnTxEvent);   
    TimerSetValue( &timerTx, 5000);

    TimerStart(&timerTx );
}

static void procLoraStage(void)
{
    switch (State)
    {
    case RX_DONE:
        Radio.Rx(RX_TIMEOUT_VALUE);

        LED_Toggle(LED1);
        PRINTF("%s\r\n", BufferRx);
        State = LOWPOWER;
        break;
    case TX_DONE:
        Radio.Rx(RX_TIMEOUT_VALUE);

        LED_Toggle(LED2);
        State = LOWPOWER;
        break;
    case RX_TIMEOUT:
    case RX_ERROR:
        Radio.Rx(RX_TIMEOUT_VALUE);

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
