#include <stdio.h>
#include <string.h>
#include "controlMessage.h"
#include "radio.h"

struct _message_packet
{
    uint16_t    stx;
    uint8_t     version;
    uint8_t     dest;
    uint8_t     src;
    uint8_t     length;
    uint8_t     payload[24];
    uint8_t     checksum;
    uint8_t     etx;
};
struct _message_packet rxMessage, txMessage;

void initMessage(void)
{
    rxMessage.stx = 0xA5A5;
    rxMessage.etx = 0x04;
    rxMessage.length = 24;
    rxMessage.version = 0;

    memcpy(&txMessage, &rxMessage, sizeof(struct _message_packet));
}   

void parsingMessage(void)
{

}

void sendMessage(void)
{
    uint8_t txbuff[32];
    memcpy(txbuff, (void *)&txMessage, sizeof(txbuff));
    Radio.Send( txbuff, sizeof(txbuff) );
}

void putMessageBuffer(uint8_t *payload, uint16_t size)
{

}

static void calChecksum(void)
{

}
