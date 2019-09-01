#include <stdio.h>
#include <string.h>
#include "LoraMessage.h"
#include "radio.h"
#include "util_console.h"

messagePacket_TypeDef txMessage;
messagePacket_TypeDef nextTxMessage[MAX_ID_LIST];
messageFIFO_TypeDef rxMessageBuffer;

uint8_t destID = MASTER_ID; /* 마스터 ID */
uint8_t srcID = 50;          /* 노드 ID */
bool isMasterMode = false;  /* 마스터 모드 플래그 */
bool existGetID = false;    /* 새로 부여 받은 ID가 존재? */
uint32_t UID_radom;         /* UID 랜덤값. 임시 사용. */

static uint8_t calChecksum(uint8_t *messageData, uint8_t messageSize);

/**
  * @brief  Lora의 수신된 메시지 버퍼에서 Payload data만 분리
  * @param  _srcID: 메시지 발신자 ID. 마스터 모드일 경우 사용됨
  * @param  rxData: Payload Data  
  * @retval SUCESS: 데이터 있음. ERROR: 데이터 없음
  */
ErrorStatus getMessagePayload(uint8_t *_srcID, uint8_t *rxData)
{
    messagePacket_TypeDef rxMessage;

    if (getMessageBuffer(&rxMessageBuffer, &rxMessage) == SUCCESS)
    {
        memcpy(rxData, rxMessage.payload, rxMessage.payloadSize);
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
  * @param  txData: 
  * @param  dataLength: Payload 데이터 크기(byte)  
  * @retval None
  */
void sendMessage(uint8_t *txData, uint8_t dataLength)
{
    if (dataLength > MESSAGE_MAX_PAYLOAD_SIZE)
    {
        //PRINTF("Error. Max data size is %d\r\n",MESSAGE_MAX_PAYLOAD_SIZE)
        return;
    }

    uint8_t txMessageSize = MESSAGE_HEADER_SIZE + dataLength;
    uint8_t txMessageBuff[txMessageSize];

    txMessage.dest = destID;
    txMessage.src = srcID;
    txMessage.payloadSize = dataLength;
    memcpy(txMessage.payload, txData, dataLength);
    txMessage.checksum = calChecksum((uint8_t *)&txMessage, txMessageSize); /* 송신 메시지 구조체 정보 완성 */
    //PRINTF("PAYLOAD : %X %X %X\r\n",txMessage.payload[0],txMessage.payload[1],txMessage.payload[2]);
    memcpy(txMessageBuff, (void *)&txMessage, txMessageSize);  /* 송신 메시지 크기가 가변임으로 구조체의 체크섬과 ETX는 잘려서 복사됨 */

    txMessageBuff[txMessageSize - 2] = txMessage.checksum;
    txMessageBuff[txMessageSize - 1] = MESSAGE_ETX;

    Radio.Send(txMessageBuff, txMessageSize);
    for (int i = 0; i < txMessageSize; i++)
        PRINTF("%x ", txMessageBuff[i]);
    //PRINTF("\r\n");
}

void initMessage(void)
{
    txMessage.stx = MESSAGE_STX;
    txMessage.dest = MASTER_ID;
    txMessage.etx = MESSAGE_ETX;
    txMessage.version = 0;
    txMessage.payloadSize = MESSAGE_MAX_PAYLOAD_SIZE;

    rxMessageBuffer.in = 0;
    rxMessageBuffer.out = 0;
    rxMessageBuffer.count = 0;

    //sendMessage(0,3);   /* random 구현을 위한 쓰래기 패킷 전송. 수신 측 RX Done 시 rand() 함수 호출됨 */
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

    //for(int i=0; i<size; i++)
    //    PRINTF("%x ", data[i]);
    //PRINTF("      %d byte    ",size);

    if ((buffer->buff[buffer->in].dest != srcID) && (isMasterMode != true)) /* 내 ID의 메시지가 아니거나 마스터 모드가 아니면 ERROR 1 리턴 */
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

#ifdef _DEBUG_
    for(int i=0; i<size; i++)
        PRINTF("%x ", data[i]);
    PRINTF("      %d byte    ",size);
    PRINTF("SRC: %X, ",buffer->buff[buffer->in-1].src);
    PRINTF("payloadSize: %X, ",buffer->buff[buffer->in-1].payloadSize);
    PRINTF("RSSI: %ddBm, ",buffer->buff[buffer->in-1].rssi);
    PRINTF("SNR: %d    \r\n",buffer->buff[buffer->in-1].snr);
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
  * @param  nextMessage: 메시지 구조체 
  * @retval 해당 ID의 보낼 메시지가 있으면 true
  */
bool existNextMessage(uint8_t _id, messagePacket_TypeDef *nextMessage)
{
    for (int i = 0; i < MAX_ID_LIST; i++)
    {
        if (nextTxMessage[i].dest == _id)
        {
            memcpy((void *)&nextMessage, (void *)&nextTxMessage[i], sizeof(nextTxMessage[i]));
            memset((void *)&nextTxMessage[i], 0, sizeof(nextTxMessage[i]));
            return true;
        }
    }

    return false;
}
