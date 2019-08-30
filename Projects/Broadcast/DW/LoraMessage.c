#include <stdio.h>
#include <string.h>
#include "LoraMessage.h"
#include "radio.h"
#include "util_console.h"

messagePacket_TypeDef txMessage;
messageFIFO_TypeDef rxMessageBuffer;

uint8_t destID = MASTER_ID; /* 마스트 ID */
uint8_t srcID = 0;          /* 노드 ID */
bool isMasterMode = false;  /* 마스터 모드 플래그 */

static uint8_t calChecksum(uint8_t *messageData, uint8_t messageSize);

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
    memcpy((void *)&txMessage.payload, txData, dataLength);
    txMessage.checksum = calChecksum((uint8_t *)&txMessage, txMessageSize); /* 송신 메시지 구조체 정보 완성 */

    memcpy(txMessageBuff, (void *)&txMessage, txMessageSize);  /* 송신 메시지 크기가 가변임으로 구조체의 체크섬과 ETX는 잘려서 복사됨 */

    txMessageBuff[txMessageSize - 2] = txMessage.checksum;
    txMessageBuff[txMessageSize - 1] = MESSAGE_ETX;

    Radio.Send(txMessageBuff, txMessageSize);
    for (int i = 0; i < txMessageSize; i++)
        PRINTF("%x ", txMessageBuff[i]);
    PRINTF("\r\n");
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
}

/**
  * @brief  Lora을 이용한 메시지 송신 함수
  * @param  txData: Payload 데이터
  * @param  dataLength: Payload 데이터 크기(byte)  
  * @retval None
  */
messageError_TypeDef putMessageBuffer(volatile messageFIFO_TypeDef *buffer, uint8_t *data, uint16_t size)
{
    memcpy((void *)&buffer->buff[buffer->in], data, size); /* 버퍼에 저장 */
    buffer->buff[buffer->in].checksum = data[size - 2];

    for(int i=0; i<size; i++)
        PRINTF("%x ", data[i]);
    PRINTF("      %d byte\r\n",size);

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

    buffer->in++;
    buffer->count++;                       /* 버퍼에 저장된 갯수 1 증가 */
    if (buffer->in == MESSAGE_BUFFER_SIZE) /* 시작 인덱스가 버퍼의 끝이면 */
    {
        buffer->in = 0; /* 시작 인덱스를 0부터 다시 시작 */
    }
    else
    {
    }
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
