#ifndef __PAYLOADMESSAGE_H__
#define __PAYLOADMESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "LoraMessage.h"

#define TOTAL_MSGID_CNT                 4
#define MTYPE_TESTMESSAGE               0xFF
#define MTYPE_REQUEST_ID                0xA0
#define MTYPE_RESPONSE_ID               0xA1        /* get ID from Master */
#define MTYPE_TEMP_HUMI                 0xC0





typedef struct
{
    uint8_t    msgID;
    uint8_t     length; /* data 길이 */
    uint8_t     data[MESSAGE_MAX_PAYLOAD_SIZE-2];
} payloadPacket_TypeDef;

typedef struct
{
    uint8_t id;
    uint32_t uid;
    uint32_t cntRequestID;
} IDInfo_TypeDef;

typedef struct
{
    uint8_t count;
    IDInfo_TypeDef idInfo[MAX_ID_LIST];
} IDList_TypeDef;

typedef enum
{
    SEARCH_ID = 0,
    SEARCH_UID
} search_type;


void procPayloadData(void);
void procMasterMode(void);
void sendPayloadData(uint8_t msgID, uint8_t *data);
void pyaloadTimerInit(void);
void payloadTimerDeInit(void);
void OnBootingEvent( void* context );
ErrorStatus InsertIDList(uint32_t _uid);


static const uint8_t msgIDLength[TOTAL_MSGID_CNT][2] =
{
    {MTYPE_TESTMESSAGE, 3},
    {MTYPE_REQUEST_ID, 4},
    {MTYPE_RESPONSE_ID, 1},
    {MTYPE_TEMP_HUMI, 8}
};




#ifdef __cplusplus
}
#endif

#endif /* __CONTROLMESSAGE_H__ */
