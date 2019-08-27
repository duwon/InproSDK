#ifndef __PAYLOADMESSAGE_H__
#define __PAYLOADMESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "LoraMessage.h"

#define TOTAL_MSGID_CNT           2
#define MTYPE_REQUEST_ID        0xA0
#define MTYPE_GETID             0xA1        /* get ID from Master */






typedef struct
{
    uint16_t    msgID;
    uint8_t     length;
    uint8_t     data[MESSAGE_MAX_PAYLOAD_SIZE-2];
} payloadPacket_TypeDef;



void procPayloadData(void);
void sendPayloadData(uint8_t msgID, uint8_t *data);


#ifdef __cplusplus
}
#endif

#endif /* __CONTROLMESSAGE_H__ */
