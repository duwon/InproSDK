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

void RadioInit(void);












#ifdef __cplusplus
}
#endif

#endif /* __LORARADIO_H__ */
