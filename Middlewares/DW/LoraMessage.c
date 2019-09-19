#include <stdio.h>
#include <string.h>
#include "hw.h"
#include "LoraRadio.h"
#include "LoraMessage.h"

messagePacket_TypeDef txMessage;
messagePacket_TypeDef nextTxMessage[MAX_ID_LIST];
messageFIFO_TypeDef rxMessageBuffer;

IDList_TypeDef IDList;                      /* 노드 아이디 정보 구조체 */
#ifdef MASTER_MODE
uint8_t srcID = MASTER_ID;
#else
uint8_t srcID = DEVICE_ID;
#endif
uint8_t UID[8] = {0,};
uint8_t groupID = 0;
static uint8_t calChecksum(uint8_t *messageData, uint8_t messageSize);



/**
  * @brief  Lora의 수신된 메시지 버퍼에서 Payload data만 분리
  * @param  _srcID: 되돌려 받을 메시지 발신자 ID. 마스터 모드일 경우 사용됨
  * @param  rxData: 되돌려 받을 Payload Data
  * @retval SUCESS: 데이터 있음. ERROR: 데이터 없음
  */
ErrorStatus getMessagePayload(uint8_t *_srcID, uint8_t *paloadData)
{
    messagePacket_TypeDef rxMessage;

    if (getMessageBuffer(&rxMessageBuffer, &rxMessage) == SUCCESS)
    {
        memcpy(paloadData, rxMessage.payload, rxMessage.payloadSize);
        *_srcID = rxMessage.src;
    }
    else
    {
        return ERROR;
    }

    return SUCCESS;
}

/**
  * @brief  Lora을 이용한 메시지 송신 함수
  * @param  txData: Payload Data
  * @param  dataLength: Payload 데이터 크기(byte)  
  * @retval None
  */
void sendMessage(uint8_t _destID, uint8_t *paloadData, uint8_t payloadSize)
{
    if (payloadSize > MESSAGE_MAX_PAYLOAD_SIZE)
    {
        //PRINTF("Error. Max data size is %d\r\n",MESSAGE_MAX_PAYLOAD_SIZE)
        return;
    }

    uint8_t txMessageSize = MESSAGE_HEADER_SIZE + payloadSize;
    uint8_t txMessageBuff[MESSAGE_SIZE];

    txMessage.dest = _destID;
    txMessage.src = srcID;
    memcpy((void*)&txMessage.msgID, paloadData, payloadSize + 2);
    txMessage.checksum = calChecksum((uint8_t *)&txMessage, txMessageSize); /* 송신 메시지 구조체 정보 완성 */

    memcpy(txMessageBuff, (void *)&txMessage, txMessageSize);  /* 송신 메시지 크기가 가변임으로 구조체의 체크섬과 ETX는 잘려서 복사됨 */

    txMessageBuff[txMessageSize - 2] = txMessage.checksum;
    txMessageBuff[txMessageSize - 1] = MESSAGE_ETX;

    Radio_Tx(txMessageBuff, txMessageSize);

#ifdef _DEBUG_    
    USBPRINT("%d [TX] ", HW_RTC_GetTimerValue());
    for (int i = 0; i < txMessageSize; i++)
        USBPRINT("%X ", txMessageBuff[i]);
    USBPRINT("\r\n");
#endif
}

void LoraMessage_Init(void)
{
    txMessage.stx = MESSAGE_STX;
    txMessage.dest = MASTER_ID;
    txMessage.etx = MESSAGE_ETX;
    txMessage.payloadSize = MESSAGE_MAX_PAYLOAD_SIZE;

    rxMessageBuffer.in = 0;
    rxMessageBuffer.out = 0;
    rxMessageBuffer.count = 0;

    HW_GetUniqueId(UID);
}

/**
  * @brief  Lora을 이용한 메시지 송신 함수
  * @param  txData: Payload 데이터
  * @param  dataLength: Payload 데이터 크기(byte)  
  * @retval None
  */
messageError_TypeDef putMessageBuffer(volatile messageFIFO_TypeDef *buffer, uint8_t *data, uint16_t size,  int16_t rssi, int8_t snr)
{
    memcpy((void *)&buffer->buff[buffer->in], data, size); /* 버퍼에 저장 */
    buffer->buff[buffer->in].checksum = data[size - 2];

    if (buffer->buff[buffer->in].dest != srcID) /* 내 ID의 메시지가 아니면 ERROR 1 리턴 */
    {
        return NOT_MY_MESSAGE;
    }
    else if (buffer->count == MESSAGE_BUFFER_SIZE) /* 데이터가 버퍼에 가득 찼으면 ERROR 2 리턴 */
    {
        //PRINTF("buffer full \r\n");
        return BUFFER_FULL;
    }
    else if (size > (MESSAGE_HEADER_SIZE + MESSAGE_MAX_PAYLOAD_SIZE))
    {
        //PRINTF("payload oversize \r\n");
        return OVERSIZE_MESSAGE;
    }
    else if (size != (buffer->buff[buffer->in].payloadSize + MESSAGE_HEADER_SIZE))
    {
        //PRINTF("different size\r\n");
        return DIFFERENT_MESSAGE_SIZE;
    }
    else if (buffer->buff[buffer->in].checksum != calChecksum(data, size))
    {
        //PRINTF("calChecksum \r\n");
        return CHECKSUM_FAIL;
    }
    else if (buffer->buff[buffer->in].stx != 0xA5A5)
    {
        //PRINTF("not stx");
        return NOT_STX;
    }

    buffer->buff[buffer->in].rssi = rssi;
    buffer->buff[buffer->in].snr = snr;

    buffer->in++;
    buffer->count++;                       /* 버퍼에 저장된 갯수 1 증가 */
    if (buffer->in == MESSAGE_BUFFER_SIZE) /* 시작 인덱스가 버퍼의 끝이면 */
    {
        buffer->in = 0; /* 시작 인덱스를 0부터 다시 시작 */
    }
    else
    {
    }

#ifdef MASTER_MODE

    for(int i=0; i<size; i++)
    PRINTF("%c", data[i]);
        

    #ifdef _DEBUG_
        USBPRINT("%d [RX] ", HW_RTC_GetTimerValue());
        for(int i=0; i<size; i++)
           USBPRINT("%x ", data[i]);
        USBPRINT("      %d byte  SRC: %X, payloadSize: %X, RSSI: %ddBm, SNR: %d    \r\n",size,buffer->buff[buffer->in-1].src,buffer->buff[buffer->in-1].payloadSize,buffer->buff[buffer->in-1].rssi,buffer->buff[buffer->in-1].snr);
    #endif
#else
    #ifdef _DEBUG_
        USBPRINT("%d [RX] ", HW_RTC_GetTimerValue());
        for(int i=0; i<size; i++)
            USBPRINT("%X ", data[i]);
        USBPRINT("SRC: %X\r\n",buffer->buff[buffer->in-1].src);
    #endif
#endif

    return PUT_SUCCESS;
}

ErrorStatus getMessageBuffer(volatile messageFIFO_TypeDef *buffer, messagePacket_TypeDef *data)
{
    if (buffer->count == 0) /* 버퍼에 데이터가 없으면 ERROR 리턴 */
    {
        return ERROR;
    }
    else
    {
    }

    memcpy(data, (void *)&buffer->buff[buffer->out], MESSAGE_SIZE); /* 버퍼에서 읽음 */
    memset((void *)&buffer->buff[buffer->out++], 0, MESSAGE_SIZE);
    buffer->count--;                        /* 버퍼에 저장된 데이터 갯수 1 감소 */
    if (buffer->out == MESSAGE_BUFFER_SIZE) /* 끝 인덱스가 버퍼의 끝이면 */
    {
        buffer->out = 0; /* 끝 인덱스를 0부터 다시 시작 */
    }
    else
    {
    }

    return SUCCESS;
}

static uint8_t calChecksum(uint8_t *messageData, uint8_t messageSize)
{
    uint8_t tempChecksum = 0;
    for (int i = 0; i < (messageSize - 2); i++)
    {
        tempChecksum ^= messageData[i];
    }

    return tempChecksum;
}

/**
  * @brief  메시지 수신 시 다음에 보낼 데이터가 있는지 확인
  * @param  _id: 메시지가 있는지 검색 할 Node ID
  * @param  Message: 리턴될 메시지 구조체 
  * @retval 해당 ID의 보낼 메시지가 있으면 true
  */
bool existNextMessage(uint8_t _id, messagePacket_TypeDef *Message)
{
    if(_id >= MAX_ID_LIST)
    {
        return false;
    }
    if (nextTxMessage[_id].dest == _id) /* 수신 측(NODE) ID */
    {
        memcpy(Message, (void *)&nextTxMessage[_id], sizeof(nextTxMessage[_id]));
        memset((void *)&nextTxMessage[_id], 0, sizeof(nextTxMessage[_id]));
        return true;
    }
    else
    {
    }

    return false;
}

/**
  * @brief  메시지 수신 시 다음에 보낼 데이터를 저장
  * @param  _destID: 메시지를 저장할 Node ID
  * @param  txData: 저장할 메시지 (Payload Data)
  * @param  dataLength: 저장할 메시지의 크기
  * @retval 해당 ID의 보낼 메시지가 존재하면 false
  */
bool insertNextMessage(uint8_t _destID, uint8_t *txData, uint8_t dataLength)
{
    if (nextTxMessage[_destID].dest == 0) /* 값이 존재하지 않으면 */
    {
        uint8_t txMessageSize = MESSAGE_HEADER_SIZE + dataLength;

        nextTxMessage[_destID].dest = _destID;
        nextTxMessage[_destID].src = srcID;
        nextTxMessage[_destID].payloadSize = dataLength;
        memcpy(nextTxMessage[_destID].payload, txData, dataLength);
        nextTxMessage[_destID].checksum = calChecksum((uint8_t *)&nextTxMessage[_destID], txMessageSize);
        nextTxMessage[_destID].etx = MESSAGE_ETX;
			
        return true;
    }
    else
    {
    }
    return false;
}


/**
  * @brief  ID List에서 값을 찾아 배열의 해당 Index을 리턴
  * @param  _type: 찾을 조건
  *         @arg SEARCH_ID: ID 찾기
            @arg SEARCH_UID: UID 찾기
  * @retval 값이 위치한 배열의 Index. 찾는 값이 없으면 0.
  */
uint8_t getIDInfo(search_type _type, uint8_t *value)
{
    if (_type == SEARCH_UID)
    {
        for (int i = 1; i < MAX_ID_LIST; i++)
        {
            if (*IDList.idInfo[i].uid == *value)
            {
                //PRINTF("UID index: %d\r\n", i);
                return i;
            }
        }
    }
    else
    {
        for (int i = 1; i < MAX_ID_LIST; i++)
        {
            if (IDList.idInfo[i].id == *value)
            {
                //PRINTF("ID index: %d\r\n", i);
                return i;
            }
        }
    }

    return 0;
}

/**
  * @brief  ID List에 저장
  * @param  _id: ID 값, ID 값이 0이면 ID 자동 부여
  * @param  _uid: UID 값
  * @retval UID가 ID List에 없으면 ID와 UID을 저장하고 SUCESS리턴.
  */
ErrorStatus InsertIDList(uint8_t _id, uint8_t *_uid)
{
    uint8_t UIDExist = getIDInfo(SEARCH_UID, (uint8_t *)&_uid);
    if (IDList.count >= MAX_ID_LIST)
    {
        return ERROR;
    }
    else if ((_id != 0) && (UIDExist == 0))
    {
        if(IDList.idInfo[_id].id == 0)
        IDList.idInfo[_id].id = _id;
        *IDList.idInfo[_id].uid = *_uid;
        IDList.count++;

        return SUCCESS;
    }
    else if (UIDExist == 0)
    {
        uint8_t assignID = 0;

        for (int i = 0; i < MAX_ID_LIST; i++)
        {
            if (*IDList.idInfo[i].uid == 0)
            {
                assignID = i;
                break;
            }
        }

        IDList.idInfo[assignID].id = assignID;
        *IDList.idInfo[assignID].uid = *_uid;
        IDList.count++;

        return SUCCESS;
    }
    else
    {
    }

    return ERROR;
}

/**
  * @brief  ID List에서 삭제
  * @param  _uid: UID 값
  * @retval ID가 있으면 삭제하고 SUCCESS 리턴.
  */
ErrorStatus DeleteIDList(uint8_t _id)
{
    uint8_t existID = getIDInfo(SEARCH_ID, (uint8_t *)&_id);
    if (existID != 0)
    {
        memset((void *)&IDList.idInfo[existID], 0, sizeof(IDList.idInfo[existID]));
        IDList.count--;
        return SUCCESS;
    }
    else
    {
    }
    return ERROR;
}
