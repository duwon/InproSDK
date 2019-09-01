#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "PayloadMessage.h"
#include "timeServer.h"
#include "hw.h"

#define BOOTING_INTERVAL_TIME               60000
#define BOOTING_START_TIME                  60000
static  TimerEvent_t timerBooting;          /* 부팅시 ID 받는 타이머. ID 받으면 종료 */


payloadPacket_TypeDef txPayloadData;        /* 송신용 Payload 버퍼 구조체 */
IDList_TypeDef IDList;                      /* 노드 아이디 정보 구조체 */


static uint8_t getPayloadLength(uint8_t msgID);
static void OnBootingEvent( void* context );
static uint8_t getIDInfo(search_type _type, uint8_t *value);



/**
  * @brief  Payload 데이터 처리 함수.
  * @param  None 
  * @retval None
  */
void procPayloadData(void)
{
    uint8_t tempSrcID;
    uint8_t tempRxPayloadBuffer[MESSAGE_MAX_PAYLOAD_SIZE];
    if (getMessagePayload((void *)&tempSrcID, tempRxPayloadBuffer) == SUCCESS)
    {
        switch (tempRxPayloadBuffer[0])
        {
        case ERROR:
            break;
        default:
            break;
        }
    }
}


/**
  * @brief  마스터모드의 Payload 데이터 처리 함수.
  * @param  None 
  * @retval None
  */
void procMasterMode(void)
{
    uint8_t tempSrcID;
    uint8_t tempRxPayloadBuffer[MESSAGE_MAX_PAYLOAD_SIZE];
    if (getMessagePayload((void *)&tempSrcID, tempRxPayloadBuffer) == SUCCESS)
    {
        uint32_t tempUID = 0;
        //PRINTF("SRC ID: %x, UID: %x",tempSrcID, tempRxPayloadBuffer[1]);
        
        switch (tempRxPayloadBuffer[0])
        {
        case MTYPE_REQUEST_ID:

            memcpy((void *)&tempUID, (void *)&tempRxPayloadBuffer[1], 4);
            if (InsertIDList(tempUID) != SUCCESS)
            {
            }

            destID = tempSrcID;
            uint8_t tempTxData;
            tempTxData = getIDInfo(SEARCH_UID, (uint8_t *)&tempUID);
            sendPayloadData(MTYPE_RESPONSE_ID, (void *)&tempTxData);
            PRINTF("tempTxData : %x\r\n",tempTxData);

            //

            for (int i = 0; i < IDList.count; i++)
                PRINTF("\r\n ID : %d, UID : %x\r\n", IDList.idInfo[i].id, IDList.idInfo[i].uid);

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
    memcpy(txPayloadData.data, data, txPayloadData.length-2);
    //PRINTF("txPayloadData.data %X %X %X, data %X %X %X\r\n",txPayloadData.data[0],txPayloadData.data[1],txPayloadData.data[2],data[0],data[1],data[2]);
    sendMessage((void *)&txPayloadData, txPayloadData.length);
}

/**
  * @brief  Payload 전송 시 MSG의 길이 찾음
  * @param  msgID: 
  * @retval Payload 길이(MsgID + Data). 해당 메시지 ID가 없으면 0
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
    uint32_t timerBootingInterval = (rand() % BOOTING_INTERVAL_TIME) + BOOTING_START_TIME; /* 랜덤함수에 문제 있음 */
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
    payloadPacket_TypeDef tempRxPayloadBuffer;
    if (getMessagePayload((void *)&tempSrc, (uint8_t *)&tempRxPayloadBuffer) == SUCCESS)
    {
        PRINTF("msg type: %X, length: %x, ID: %x \r\n",tempRxPayloadBuffer.msgID,tempRxPayloadBuffer.length,tempRxPayloadBuffer.data[0]);
        if (tempRxPayloadBuffer.msgID == MTYPE_RESPONSE_ID)
        {
            srcID = tempRxPayloadBuffer.data[0];
            TimerStop(&timerBooting);
            existGetID = true;
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
    sendPayloadData(MTYPE_REQUEST_ID, (void *)&UID_radom);

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
    if(_type == SEARCH_UID)
    {
        memcpy((void*)&searchUID, value, 4);
        for(int i=1; i<IDList.count; i++)
        {
            if(IDList.idInfo[i].uid == searchUID )
            {
                PRINTF("UID index: %d\r\n", i);
                return i;
            }
        }        
    }
    else
    {
        for(int i=1; i<IDList.count; i++)
        {
            if(IDList.idInfo[i].id == *value )
            {
                PRINTF("ID index: %d\r\n", i);
                return i;
            }
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
    if( getIDInfo(SEARCH_UID, (uint8_t *)&_uid) == 0)
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
