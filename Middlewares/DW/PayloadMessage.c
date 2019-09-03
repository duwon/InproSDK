#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "PayloadMessage.h"
#include "timeServer.h"
#include "hw.h"

#define BOOTING_INTERVAL_TIME               5000
#define BOOTING_START_TIME                  5000


static  TimerEvent_t timerBooting;              /* 부팅시 ID 받는 타이머. ID 받으면 종료 */
static  TimerEvent_t timerDebugTest;            /* 시험용 타이머 */

static uint8_t getPayloadLength(uint8_t msgID); /* msgID의 data 길이 반환 */
static void OnBootingEvent(void *context);      /* ID 부여 받는 이벤트 */
static void OnDebugTestEvent(void *context);    /* 시험용 이벤트 */

/**
  * @brief  payload data 송신
  * @param  _destID: 메시지를 전달 할 주소. Node의 경우 MASTER_ID
  * @param  msgID: payload data의 메시지 ID
  * @param  data: 메시지 ID에 해당하는 data
  * @retval None
  */
void sendPayloadData(uint8_t _destID, uint8_t msgID, uint8_t *data)
{
    payloadPacket_TypeDef tempTxPayloadData;        /* 송신용 Payload 버퍼 구조체 */
    tempTxPayloadData.msgID = msgID;
    tempTxPayloadData.length = getPayloadLength(msgID);
    memcpy(tempTxPayloadData.data, data, tempTxPayloadData.length);

    sendMessage(_destID, (void *)&tempTxPayloadData, tempTxPayloadData.length + 2);
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

static void OnDebugTestEvent(void *context)
{

}

/**
  * @brief  마스터모드의 Payload 데이터 처리 함수.
  * @param  None 
  * @retval None
  */
void procPayloadMessage(void)
{
    float temperature = 0;
    float humidity = 0;
    uint8_t rxSrcID;
    payloadPacket_TypeDef rxPayload;
    messagePacket_TypeDef nextMessage;

    if (getMessagePayload((void *)&rxSrcID, (uint8_t *)&rxPayload) == SUCCESS)
    {
        /* ACK */
        if ((existNextMessage(rxSrcID, &nextMessage) == true) && (isMasterMode == true)) /* 노드의 RX 구간에서 전송 할 메시지가 있으면 메시지 전송 */
        {
            sendMessage(rxSrcID, nextMessage.payload, nextMessage.payloadSize);
        }
        else if ((rxPayload.msgID != MTYPE_REQUEST_ID) && (isMasterMode == true)) /* 전송 할 메시지가 없으면 payload 없이 메시지 전송. ACK 응답 */
        {
            sendMessage(rxSrcID, 0, 0);
        }

        /* Payload 처리 */
        uint32_t tempUID = 0;
        switch (rxPayload.msgID) /* Payload 메시지 ID 판단 */
        {
        case MTYPE_REQUEST_ID: /* Node에 ID 부여 */

            memcpy((void *)&tempUID, rxPayload.data, 4);
            if (InsertIDList(0, tempUID) != SUCCESS)
            {
            }

            uint8_t tempTxData;
            tempTxData = getIDInfo(SEARCH_UID, (uint8_t *)&tempUID);
            sendPayloadData(rxSrcID, MTYPE_RESPONSE_ID, (void *)&tempTxData);
            PRINTF("tempTxData : %x\r\n", tempTxData);
            break;
        case MTYPE_RESPONSE_UID: /* Node의 UID 저장 */
            memcpy((void *)&tempUID, rxPayload.data, 4);
            if( getIDInfo(SEARCH_UID, (uint8_t *)&tempUID) == 0)
            {
                if (InsertIDList(rxSrcID, tempUID) != SUCCESS)
                {
                }
            }
            break;
        case MTYPE_TEMP_HUMI:
            memcpy((void *)&temperature, (void *)&rxPayload.data[0], 4);
            memcpy((void *)&humidity, (void *)&rxPayload.data[4], 4);

            PRINTF("SRC ID : %d, Temp: %.1f, Humi: %.f \r\n", rxSrcID, temperature, humidity);

            break;
        case ERROR:
            break;
        default:
            break;
        }

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
    }
}
