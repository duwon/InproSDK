#ifndef __CONTROLMESSAGE_H__
#define __CONTROLMESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
	
    
    
#define MESSAGE_BUFFER_SIZE         10          /* 메시지 버퍼 크기 */
#define MESSAGE_HEADER_SIZE         8
#define MESSAGE_MAX_PAYLOAD_SIZE    64          /* 메시지 payload 크기 */
#define MESSAGE_SIZE                (MESSAGE_HEADER_SIZE + MESSAGE_MAX_PAYLOAD_SIZE)
#define MESSAGE_STX                 0xA5A5
#define MESSAGE_ETX                 0x04

#define MASTER_ID                   0
#define UID                         *(__IO uint32_t *)UID_BASE
typedef struct
{
    uint16_t    stx;
    uint8_t     version;
    uint8_t     dest;
    uint8_t     src;
    uint8_t     payloadSize;
    uint8_t     payload[MESSAGE_MAX_PAYLOAD_SIZE];
    uint8_t     checksum;
    uint8_t     etx;
} messagePacket_TypeDef;

typedef struct {
    uint8_t in;
    uint8_t out;
    uint8_t count;
    messagePacket_TypeDef buff[MESSAGE_BUFFER_SIZE];
} messageFIFO_TypeDef;





extern messageFIFO_TypeDef rxMessageBuffer;;
extern uint8_t srcID;


void initMessage(void);
ErrorStatus getMessagePayload(uint8_t *data);
void sendMessage(uint8_t *txData, uint8_t dataLength);
ErrorStatus putMessageBuffer(volatile messageFIFO_TypeDef *buffer, uint8_t *data, uint16_t size);
ErrorStatus getMessageBuffer(volatile messageFIFO_TypeDef *buffer, messagePacket_TypeDef *data);









#ifdef __cplusplus
}
#endif

#endif /* __CONTROLMESSAGE_H__ */
