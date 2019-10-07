#include <stdio.h>
#include <stdlib.h>
#include "hw.h"
#include "LoraRadio.h"
#include "util_console.h"
#include "LoraMessage.h"
#include "PayloadMessage.h"
#include "timeServer.h"

LoraState_t LoraState = LOWPOWER;
uint8_t resendMessage[TX_MAX_SIZE+1] = {0,};
static  TimerEvent_t resnedTimer; /* Tx Timers objects */

static void OnTxResend(void* context);

void OnTxDone( void ); /* brief Function to be executed on Radio Tx Done event */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ); /* brief Function to be executed on Radio Rx Done event */
void OnTxTimeout( void ); /* brief Function executed on Radio Tx Timeout event */
void OnRxTimeout( void ); /* brief Function executed on Radio Rx Timeout event */
void OnRxError( void ); /* brief Function executed on Radio Rx Error event */

void OnTxDone( void )
{
    Radio.Sleep( );
    LoraState = TX_DONE;
//    PRINTF("        OnTxDone\n\r");
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep();
    uint8_t errorCode = putMessageBuffer(&rxMessageBuffer, payload, size, rssi, snr);
    
    if(errorCode == NOT_MY_MESSAGE)
    {
        LoraState = RX_ERROR;
        return;
    }
    else if (errorCode != PUT_SUCCESS)
    {
#ifdef _DEBUG_
        PRINTF("Message Error Code %d\r\n", errorCode);
#endif
    }
    LoraState = RX_DONE;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    LoraState = TX_TIMEOUT;
#ifdef _DEBUG_
    //PRINTF("OnTxTimeout\n\r");
#endif
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    LoraState = RX_TIMEOUT;
#ifdef _DEBUG_    
    //PRINTF("OnRxTimeout\n\r");
#endif
}

void OnRxError( void )
{
    Radio.Sleep( );
    LoraState = RX_ERROR;
#ifdef _DEBUG_
    //PRINTF("OnRxError\n\r");
#endif
}

void LoraRadio_Init(void)
{
    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init(&RadioEvents);

    Radio.SetChannel(RF_KR920_FREQUENCY[0]);

    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    Radio.SetChannel(RF_KR920_FREQUENCY[1]);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    TimerInit(&resnedTimer, OnTxResend);
}

void Radio_SetChannel(trxMode mode, uint8_t channel)
{
    #ifdef MASTER_MODE
        Radio.SetChannel(RF_KR920_FREQUENCY[channel * 2 + !mode]);
    #else
        Radio.SetChannel(RF_KR920_FREQUENCY[channel * 2 + mode]);
    #endif
}

void Radio_Rx(void)
{
    LoraState = RX_RUNNING;
    Radio_SetChannel(RX_MODE,groupID);
    Radio.Rx(RX_TIMEOUT_VALUE);
}

void Radio_Tx( uint8_t *buffer, uint8_t size )
{
    LoraState = TX_RUNNING;
    Radio_SetChannel(TX_MODE,groupID);
    Radio.Send(buffer, size);

    memcpy(resendMessage, buffer, size);
    resendMessage[TX_MAX_SIZE] = size;
}

void Radio_Resend(void)
{
    TimerSetValue(&resnedTimer, (rand() % RESEND_INTERVAL_TIME) + RESEND_INTERVAL_TIME);
    TimerStart(&resnedTimer); 
}

static void OnTxResend(void* context)
{
    Radio_Tx(resendMessage, resendMessage[TX_MAX_SIZE]);
}
