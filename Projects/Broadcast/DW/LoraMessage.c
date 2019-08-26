#include <stdio.h>
#include <string.h>
#include "LoraMessage.h"
#include "radio.h"
#include "util_console.h"

messagePacket_TypeDef txMessage;
messageFIFO_TypeDef rxMessageBuffer;

uint8_t destID = MASTER_ID;
uint8_t srcID = 0;

static uint8_t calChecksum(uint8_t *messageData, uint8_t messageSize);
    
ErrorStatus getMessagePayload(uint8_t *rxData)
{
    messagePacket_TypeDef rxMessage;
    if (getMessageBuffer(&rxMessageBuffer, &rxMessage) == SUCCESS)
    {
        memcpy(rxData, rxMessage.payload, rxMessage.payloadSize);
    }
    else
    {
        return ERROR;
    }
    
    return SUCCESS;
}

void sendMessage(uint8_t *txData, uint8_t dataLength)
{
    if(dataLength > MESSAGE_MAX_PAYLOAD_SIZE)
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
    txMessage.checksum = calChecksum((uint8_t *)&txMessage, txMessageSize);

    memcpy(txMessageBuff, (void *)&txMessage, txMessageSize);

    txMessageBuff[txMessageSize-2] = txMessage.checksum;
    txMessageBuff[txMessageSize-1] = MESSAGE_ETX;

    Radio.Send( txMessageBuff, txMessageSize);
    for(int i=0; i<txMessageSize; i++)
        PRINTF("%x ", txMessageBuff[i]);
    PRINTF("\r\n");
}

void initMessage(void)
{
    txMessage.stx = 0xA5A5;
    txMessage.dest = MASTER_ID;
    txMessage.etx = 0x04;
    txMessage.version = 0;
    txMessage.payloadSize = MESSAGE_MAX_PAYLOAD_SIZE;
}   

uint8_t putMessageBuffer(volatile messageFIFO_TypeDef *buffer, uint8_t *data, uint16_t size)
{
    memcpy((void *)&buffer->buff[buffer->in], data, size);          /* 버퍼에 저장 */
    buffer->buff[buffer->in].checksum = data[size-2];

    PRINTF("buffer count : %d, data %d, %d \r\n",buffer->count,buffer->buff[buffer->in].payload[0], buffer->buff[buffer->in].payload[1]);
    for(int i=0; i<size; i++)
        PRINTF("%x ", data[i]);
    PRINTF("      %d byte\r\n",size);

    uint8_t error_code;
    if (buffer->count == MESSAGE_BUFFER_SIZE) /* 데이터가 버퍼에 가득 찼으면 ERROR 리턴 */
    {
        //PRINTF("buffer full \r\n");
        error_code = 0x01;
        return error_code;
    }
    else if (size > (MESSAGE_HEADER_SIZE + MESSAGE_MAX_PAYLOAD_SIZE))
    {
        //PRINTF("payload oversize \r\n");
        error_code = 0x02;
        return error_code;
    }
    else if (size != (buffer->buff[buffer->in].payloadSize + MESSAGE_HEADER_SIZE))
    {
        //PRINTF("different size\r\n");
        error_code = 0x03;
        return error_code;
    }
    else if (buffer->buff[buffer->in].checksum != calChecksum(data,size))
    {
        //PRINTF("calChecksum \r\n");
        error_code = 0x04;
        return error_code;
    }
    else if (buffer->buff[buffer->in].stx != 0xA5A5)
    {
        //PRINTF("not stx");
        error_code = 0x05;
        return error_code;
    }

    buffer->in++;
    buffer->count++;                                /* 버퍼에 저장된 갯수 1 증가 */
    if(buffer->in == MESSAGE_BUFFER_SIZE)           /* 시작 인덱스가 버퍼의 끝이면 */
    {
        buffer->in = 0;                             /* 시작 인덱스를 0부터 다시 시작 */
    }
    else
    {
    }
    return 0;

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

    memcpy(data, (void *)&buffer->buff[buffer->out], sizeof(buffer->buff));            /* 버퍼에서 읽음 */
    memset((void *)&buffer->buff[buffer->out++], 0, sizeof(buffer->buff));
    buffer->count--;                                /* 버퍼에 저장된 데이터 갯수 1 감소 */
    if (buffer->out == MESSAGE_BUFFER_SIZE)         /* 끝 인덱스가 버퍼의 끝이면 */
    {
        buffer->out = 0;                            /* 끝 인덱스를 0부터 다시 시작 */
    }
    else
    {
    }

    return SUCCESS;
}

static uint8_t calChecksum(uint8_t *messageData, uint8_t messageSize)
{
    uint8_t tempChecksum = 0;
    for(int i=0; i<(messageSize-2); i++)
    {
        tempChecksum ^= messageData[i];
    }

    return tempChecksum;
}
