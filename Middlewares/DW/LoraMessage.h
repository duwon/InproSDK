#ifndef __LORAMESSAGE_H__
#define __LORAMESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include <stdbool.h>

#define         UID1                                 ( 0x1FF80050 )
#define         UID2                                 ( 0x1FF80054 )
#define         UID3                                 ( 0x1FF80064 )   
    
#define MESSAGE_BUFFER_SIZE         10          /* 메시지 버퍼 크기 */
#define MESSAGE_HEADER_SIZE         15
#define MESSAGE_MAX_PAYLOAD_SIZE    49          /* 메시지 payload 크기 */
#define MESSAGE_SIZE                (MESSAGE_HEADER_SIZE + MESSAGE_MAX_PAYLOAD_SIZE)
#define MESSAGE_STX                 0xA5A55A5A
#define MESSAGE_ETX                 0x04

#define MASTER_ID                   0x12345678
#define UID                         (uint32_t)( ( *( uint32_t* )UID1 ) ^ ( *( uint32_t* )UID2 ) ^ ( *( uint32_t* )UID3 ) )


#define MAX_ID_LIST                 50

typedef struct {
    uint32_t    stx;
    uint32_t    dest;
    uint32_t    src;
    uint8_t     payloadSize;
    uint8_t     payload[MESSAGE_MAX_PAYLOAD_SIZE];
    uint8_t     checksum;
    uint8_t     etx;
    int16_t     rssi;
    uint8_t     snr;
} messagePacket_TypeDef;

typedef struct {
    uint8_t in;
    uint8_t out;
    uint8_t count;
    messagePacket_TypeDef buff[MESSAGE_BUFFER_SIZE];
} messageFIFO_TypeDef;

typedef enum {
    PUT_SUCCESS = 0,
    NOT_MY_MESSAGE = 1,
    BUFFER_FULL,
    OVERSIZE_MESSAGE,
    DIFFERENT_MESSAGE_SIZE,
    CHECKSUM_FAIL,
    NOT_STX
} messageError_TypeDef;

typedef struct {
    uint8_t id;
    uint32_t uid;
    uint32_t cntRequestID;
} IDInfo_TypeDef;

typedef struct {
    uint8_t count;
    IDInfo_TypeDef idInfo[MAX_ID_LIST];
} IDList_TypeDef;

typedef enum
{
    SEARCH_ID = 0,
    SEARCH_UID
} search_type;

extern messageFIFO_TypeDef rxMessageBuffer;
extern IDList_TypeDef IDList;

void Message_Init(void);
ErrorStatus getMessagePayload(uint8_t *_srcID, uint8_t *rxData);
void sendMessage(uint8_t _destID, uint8_t *txData, uint8_t dataLength);
messageError_TypeDef putMessageBuffer(volatile messageFIFO_TypeDef *buffer, uint8_t *data, uint16_t size,  int16_t rssi, int8_t snr);
ErrorStatus getMessageBuffer(volatile messageFIFO_TypeDef *buffer, messagePacket_TypeDef *data);

bool existNextMessage(uint8_t _id, messagePacket_TypeDef *nextMessage);
bool insertNextMessage(uint8_t _destID, uint8_t *txData, uint8_t dataLength);

ErrorStatus InsertIDList(uint8_t _id, uint32_t _uid);
ErrorStatus DeleteIDList(uint8_t _id);
uint8_t getIDInfo(search_type _type, uint8_t *value);



#ifdef __cplusplus
}
#endif

#endif /* __LORAMESSAGE_H__ */
