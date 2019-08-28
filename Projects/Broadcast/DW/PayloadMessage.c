#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "PayloadMessage.h"
#include "timeServer.h"

#define BOOTING_INTERVAL_TIME                               5000
#define BOOTING_START_TIME                                  7000
static  TimerEvent_t timerBooting;


payloadPacket_TypeDef txPayloadData;
IDList_TypeDef IDList;
static uint8_t getPayloadLength(uint8_t msgID);
static void OnBootingEvent( void* context );

static uint8_t getIDInfo(search_type _type, uint8_t *value);
static ErrorStatus InsertIDList(uint32_t _uid);




void procPayloadData(void)
{
    uint8_t tempSrcID;
    uint8_t tempRxPayloadBuffer[MESSAGE_MAX_PAYLOAD_SIZE];
    if (getMessagePayload((void*)&tempSrcID, tempRxPayloadBuffer) == SUCCESS)
    {
			uint32_t tempUID = 0;
        switch (tempRxPayloadBuffer[0])
        {
        case MTYPE_REQUEST_ID:
            
            memcpy((void*)&tempUID, (void*)&tempRxPayloadBuffer[1], 4);
            if(InsertIDList(tempUID) != SUCCESS)
            {
                
            }
            destID = tempSrcID;
            uint8_t tempTxData;
            tempTxData = getIDInfo(SEARCH_UID, (uint8_t *)&tempUID);
            sendPayloadData(MTYPE_RESPONSE_ID, (void *)&tempTxData);

            //

            for(int i=1; i<IDList.count;i++)
                printf(" ID : %d, UID : %x", IDList.idInfo[i].id, IDList.idInfo[i].uid);

            //
            break;
        case ERROR:
            break;
        default:
            break;
        }
    }
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

void pyaloadTimerInit(void)
{
    uint32_t timerBootingInterval = (rand() % BOOTING_INTERVAL_TIME) + BOOTING_START_TIME;
    TimerInit(&timerBooting, OnBootingEvent);   
    TimerSetValue( &timerBooting, timerBootingInterval);
    TimerStart(&timerBooting );
}

void payloadTimerDeInit(void)
{
    TimerStop(&timerBooting);
}

void OnBootingEvent(void *context)
{
    static uint8_t numberTimesSent = 0;

    uint8_t tempSrc;
    uint8_t tempRxPayloadBuffer[MESSAGE_MAX_PAYLOAD_SIZE];
    if (getMessagePayload((void *)&tempSrc, tempRxPayloadBuffer) == SUCCESS)
    {
        if (tempRxPayloadBuffer[0] == MTYPE_RESPONSE_ID)
        {
            srcID = tempRxPayloadBuffer[1];
            TimerStop(&timerBooting);
            return;
        }
        else
        {
        }
    }
    else
    {
    }

    srcID = (rand() % (0xFF - 0x30)) + 0x30;

    numberTimesSent++;
    sendPayloadData(MTYPE_REQUEST_ID, (void *)&UID);

    uint32_t timerBootingInterval = (rand() % 30000) + BOOTING_INTERVAL_TIME;
    TimerSetValue(&timerBooting, timerBootingInterval);
    TimerStart(&timerBooting);
}

void assignNodeID(void)
{
    
}

/* 찾는 값의 배열 index을 리턴. 값이 없으면 0을 리턴 */
static uint8_t getIDInfo(search_type _type, uint8_t *value)
{
    uint32_t searchUID =0;

    memcpy((void*)&searchUID, value, 4);

    for(int i=1; i<IDList.count; i++)
    {
        if(IDList.idInfo[i].uid == searchUID )
        {
            return i;
        }else if(IDList.idInfo[i].id == *value )
        {
            return i;
        }
    }

    return 0;

}

ErrorStatus InsertIDList(uint32_t _uid)
{
    if( getIDInfo(SEARCH_UID, (uint8_t *)&_uid) != 0)
    {
        IDList.idInfo[IDList.count].id = IDList.count;
        IDList.idInfo[IDList.count].uid = _uid;
        IDList.count++;
        return SUCCESS;
    }
    else
    {
    }

    return ERROR;
    
}
