#ifndef __LORARADIO_H__
#define __LORARADIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "radio.h"


#define RX_TIMEOUT_VALUE                            1000

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

extern States_t State;

static RadioEvents_t RadioEvents; /* Radio events function pointer */

void OnTxDone( void ); /* brief Function to be executed on Radio Tx Done event */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ); /* brief Function to be executed on Radio Rx Done event */
void OnTxTimeout( void ); /* brief Function executed on Radio Tx Timeout event */
void OnRxTimeout( void ); /* brief Function executed on Radio Rx Timeout event */
void OnRxError( void ); /* brief Function executed on Radio Rx Error event */
void RadioInit(void);












#ifdef __cplusplus
}
#endif

#endif /* __LORARADIO_H__ */
