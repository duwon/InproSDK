#ifndef __LORARADIO_H__
#define __LORARADIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "radio.h"


#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define RX_TIMEOUT_VALUE                            1000
#define TX_MAX_SIZE                                 200

typedef enum
{
    LOWPOWER,
    RX,
    RX_RUNNING,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_RUNNING,
    TX_TIMEOUT,
    TX_DONE,
    RX_DONE,
    STANDBY
}LoraState_t;

typedef enum
{
    TX_MODE=0,
    RX_MODE
}trxMode;

extern LoraState_t LoraState;

static RadioEvents_t RadioEvents; /* Radio events function pointer */
static const uint32_t RF_KR920_FREQUENCY[] = { 921900000, 922100000, 922300000, 922500000, 922700000, 922900000, 923100000, 923100000};

void Radio_Init(void);
void Radio_SetChannel(trxMode mode, uint8_t channel);
void Radio_Rx(void);
void Radio_Tx( uint8_t *buffer, uint8_t size );
void Radio_Resend(void);








#ifdef __cplusplus
}
#endif

#endif /* __LORARADIO_H__ */
