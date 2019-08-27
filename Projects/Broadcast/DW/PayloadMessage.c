#include <stdio.h>
#include <string.h>
#include "PayloadMessage.h"


const uint8_t msgIDLength[TOTAL_MSGID_CNT][2] =
{
    {MTYPE_REQUEST_ID, 6}
    {MTYPE_GETID, 6}
};
payloadPacket_TypeDef txPayloadData;

static uint8_t getPayloadLength(uint8_t msgID);

void procPayloadData(void)
{

}

void sendPayloadData(uint8_t msgID, uint8_t *data)
{
    txPayloadData.msgID = msgID;
    txPayloadData.length = getPayloadLength(msgID);
    memcpy(txPayloadData.data, data, txPayloadData.length-1);

    sendMessage((void *)&txPayloadData, txPayloadData.length);
}

static uint8_t getPayloadLength(uint8_t msgID)
{
    for (int i = 0; i < TOTAL_MSGID_CNT; i++)
    {
        if (msgID == msgIDLength[i][0])
        {
            return msgIDLength[i][1];
        }
        else
        {
        }
    }
    return 0;
}
