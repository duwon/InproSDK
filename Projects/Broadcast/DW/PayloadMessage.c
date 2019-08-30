#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "PayloadMessage.h"
#include "timeServer.h"

#define BOOTING_INTERVAL_TIME               5000
#define BOOTING_START_TIME                  7000
static  TimerEvent_t timerBooting;          /* 부팅시 ID 받는 타이머. ID 받으면 종료 */


payloadPacket_TypeDef txPayloadData;        /* 송신용 Payload 버퍼 구조체 */
IDList_TypeDef IDList;                      /* 노드 아이디 정보 구조체 */


static uint8_t getPayloadLength(uint8_t msgID);
static void OnBootingEvent( void* context );

static uint8_t getIDInfo(search_type _type, uint8_t *value);
static ErrorStatus InsertIDList(uint32_t _uid);



/**
  * @brief  Payload 데이터 처리 함수.
  * @param  None 
  * @retval None
  */
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

/**
  * @brief  Payload 전송 시 MSG의 길이 찾음
  * @param  msgID: 
  * @retval Payload 길이. 해당 메시지 ID가 없으면 0
  */
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

/**
  * @brief  Master에서 ID을 받는 부팅 시퀀스
  * @param  [IN] 
  * @retval None
  */
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

/**
  * @brief  ID List에서 값을 찾아 배열의 해당 Index을 리턴
  * @param  _type: 찾을 조건
  *         @arg SEARCH_ID: ID 찾기
            @arg SEARCH_UID: UID 찾기
  * @retval 값이 위치한 배열의 Index. 찾는 값이 없으면 0.
  */
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

/**
  * @brief  ID List에 저장
  * @param  _uid: UID 값
  * @retval UID가 ID List에 없으면 ID와 UID을 저장하고 SUCESS리턴.
  */
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
