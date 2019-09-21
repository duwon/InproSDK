#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "PayloadMessage.h"
#include "timeServer.h"

#define BOOTING_INTERVAL_TIME               2000    /* 전원 On 후 Master 응답 있을 때 까지 재전송 할 시간 */
#define BOOTING_START_TIME                  5000    /* 전원 On 후 Master에게 UID 전송을 시작 할 시간 */


static  TimerEvent_t timerBooting;              /* 부팅시 ID 받는 타이머. ID 받으면 종료 */
//static  TimerEvent_t timerDebugTest;            /* 시험용 타이머 */

static uint8_t getPayloadLength(uint8_t msgID); /* msgID의 data 길이 반환 */
static void OnBootingEvent(void *context);      /* ID 부여 받는 이벤트 */
static void sendMsgCtlPayloadData(uint8_t _destID, uint8_t msgID, uint8_t *data);

/**
  * @brief  사용자 payload data 송신
  * @param  _destID: 메시지를 전달 할 주소. Node의 경우 MASTER_ID
  * @param  data: 사용자 payload data
  * @param  data: payload data 크기
  * @retval None
  */
void sendPayloadData(uint8_t _destID, uint8_t *data, uint8_t dataSize)
{
    payloadPacket_TypeDef tempTxPayloadData;        /* 송신용 Payload 버퍼 구조체 */
    tempTxPayloadData.msgID = MTYPE_USER_DATA;
    tempTxPayloadData.length = dataSize;
    memcpy(tempTxPayloadData.data, data, tempTxPayloadData.length);

    sendMessage(_destID, (void *)&tempTxPayloadData, tempTxPayloadData.length);
}

static void sendAck(uint8_t _destID)
{
    payloadPacket_TypeDef tempTxPayloadData;        /* 송신용 Payload 버퍼 구조체 */
    tempTxPayloadData.msgID = MTYPE_ACK;
    tempTxPayloadData.length = 0;

    sendMessage(_destID, (void *)&tempTxPayloadData, tempTxPayloadData.length);
}

/**
  * @brief  사용자 payload data 송신
  * @param  _destID: 메시지를 전달 할 주소. Node의 경우 MASTER_ID
  * @param  msgID: payload data의 메시지 ID
  * @param  data: 메시지 ID에 해당하는 data
  * @retval None
  */
static void sendMsgCtlPayloadData(uint8_t _destID, uint8_t msgID, uint8_t *data)
{
    payloadPacket_TypeDef tempTxPayloadData;        /* 송신용 Payload 버퍼 구조체 */
    tempTxPayloadData.msgID = msgID;
    tempTxPayloadData.length = getPayloadLength(msgID);
    memcpy(tempTxPayloadData.data, data, tempTxPayloadData.length);

    sendMessage(_destID, (void *)&tempTxPayloadData, tempTxPayloadData.length);
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
#ifndef MASTER_MODE
    srand(HW_GetRandomSeed());
    uint32_t timerBootingInterval = (rand() % BOOTING_INTERVAL_TIME) + BOOTING_START_TIME;
    TimerInit(&timerBooting, OnBootingEvent);   
    TimerSetValue( &timerBooting, timerBootingInterval);
    TimerStart(&timerBooting );

    USBPRINT("Booting Time : %d ms \r\n",timerBootingInterval);
#endif

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
    sendMsgCtlPayloadData(MASTER_ID, MTYPE_RESPONSE_UID, UID);

    TimerSetValue(&timerBooting, (rand() % 30000) + BOOTING_INTERVAL_TIME);
    TimerStart(&timerBooting);
}


/**
  * @brief  마스터모드의 Payload 데이터 처리 함수.
  * @param  None 
  * @retval None
  */
void procPayloadData(void)
{
    uint8_t rxSrcID;
    payloadPacket_TypeDef rxPayload;


    if (getMessagePayload((void *)&rxSrcID, (uint8_t *)&rxPayload) == SUCCESS)
    {
#ifdef MASTER_MODE
        /* ACK */
        messagePacket_TypeDef nextMessage;
        if (existNextMessage(rxSrcID, &nextMessage) == true) /* 노드의 RX 구간에서 전송 할 메시지가 있으면 메시지 전송 */
        {
            sendPayloadData(rxSrcID, nextMessage.payload, nextMessage.payloadSize);
        }
        else if (rxPayload.msgID != MTYPE_REQUEST_ID) /* 전송 할 메시지가 없으면 payload 없이 메시지 전송. ACK 응답 */
        {
            sendAck(rxSrcID);
        }
#endif
        /* Payload 처리 */
        uint8_t tempUID[8] = {0,};
        switch (rxPayload.msgID) /* Payload 메시지 ID 판단 */
        {

        case MTYPE_RESPONSE_UID: /* Node의 UID 저장 */
            memcpy(tempUID, rxPayload.data, 8);
            if( getIDInfo(SEARCH_UID, (uint8_t *)&tempUID) == 0)
            {
                if (InsertIDList(rxSrcID, tempUID) != SUCCESS)
                {
                }
            }
            break;
        case MTYPE_USER_DATA:
            payloadDataCallback(rxSrcID, &rxPayload);
            break;
        case MTYPE_REQUEST_RESET:
            if(rxSrcID == MASTER_ID)
            {
                NVIC_SystemReset();
            }
            break;
        case MTYPE_ACK:
            USBPRINT("ACK OK\r\n");
            break;
        default:
            break;
        }
#if 0
        /* ID 재구성 */
        if(getIDInfo(SEARCH_ID, (void *)&rxSrcID) == 0) /* ID가 존재하지 않으면 다음 RX 구간에서 UID 요청 */
        {
            uint8_t txBuffer[2] = {MTYPE_REQUEST_UID, 2};
            insertNextMessage(rxSrcID, txBuffer,  txBuffer[1]);
        }

#ifdef _DEBUG_
        if( (rxPayload.msgID == MTYPE_REQUEST_ID) || (rxPayload.msgID == MTYPE_RESPONSE_UID) )
        {
            PRINTF("\r\n");
            for (int i = 0; i < IDList.count; i++)
                PRINTF("ID : %d, UID : %x\r\n", IDList.idInfo[i].id, IDList.idInfo[i].uid);
        }
#endif   
#endif     
    }
}
