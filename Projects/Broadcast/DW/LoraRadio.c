#include "LoraRadio.h"
#include "util_console.h"
#include "LoraMessage.h"
#include "PayloadMessage.h"



#define RF_FREQUENCY                                920000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

States_t State = LOWPOWER;


void OnTxDone( void ); /* brief Function to be executed on Radio Tx Done event */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ); /* brief Function to be executed on Radio Rx Done event */
void OnTxTimeout( void ); /* brief Function executed on Radio Tx Timeout event */
void OnRxTimeout( void ); /* brief Function executed on Radio Rx Timeout event */
void OnRxError( void ); /* brief Function executed on Radio Rx Error event */

void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX_DONE;
    PRINTF("        OnTxDone\n\r");
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep();
    uint8_t errorCode = putMessageBuffer(&rxMessageBuffer, payload, size, rssi, snr);
    
    if(errorCode == NOT_MY_MESSAGE)
    {
        State = RX_ERROR;
        return;
    }
    else if (errorCode != PUT_SUCCESS)
    {
        PRINTF("Error code %d\r\n", errorCode);
    }
    State = RX_DONE;
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

void RadioInit(void)
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
