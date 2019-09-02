#ifndef __PAYLOADMESSAGE_H__
#define __PAYLOADMESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "LoraMessage.h"

#define TOTAL_MSGID_CNT                 5

#define MTYPE_TESTMESSAGE1              0xFF
#define MTYPE_TESTMESSAGE2              0xFE
#define MTYPE_REQUEST_ID                0xA0
#define MTYPE_RESPONSE_ID               0xA1        /* get ID from Master */
#define MTYPE_TEMP_HUMI                 0xC0





typedef struct
{
    uint8_t    msgID;
    uint8_t     length; /* data 길이 */
    uint8_t     data[MESSAGE_MAX_PAYLOAD_SIZE-2];
} payloadPacket_TypeDef;



void procPayloadData(void);
void procMasterMode(void);
void sendPayloadData(uint8_t _destID, uint8_t msgID, uint8_t *data);
void pyaloadTimerInit(void);
void payloadTimerDeInit(void);
void OnBootingEvent( void* context );



static const uint8_t msgIDLength[TOTAL_MSGID_CNT][2] =
{
    {MTYPE_TESTMESSAGE1, 3},
    {MTYPE_TESTMESSAGE2, 12},
    {MTYPE_REQUEST_ID, 4},
    {MTYPE_RESPONSE_ID, 1},
    {MTYPE_TEMP_HUMI, 8}
};




#ifdef __cplusplus
}
#endif

#endif /* __CONTROLMESSAGE_H__ */
