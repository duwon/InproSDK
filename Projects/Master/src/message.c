#include "message.h"
#include "vcom.h"


/* Private variables ---------------------------------------------------------*/
uartFIFO_TypeDef struControllerUartBuffer;
message_TypeDef struRecvControllerMessage;
uint8_t arrControllerTxDataBuffer[MESSAGE_MAX_SIZE - MESSAGE_UART_SIZE_WITHOUT_DATA];

bool messageInitialized = false;
/* Private functions ---------------------------------------------------------*/
void parsingControllerMessage(void);
void putControllerUartBuffer(uint8_t ucByte);
void controllerRxCpltCallback(UART_HandleTypeDef *UartHandle);

void initBuffer(volatile uartFIFO_TypeDef *buffer)
{
  buffer->count = 0;/* 버퍼에 저장된 데이터 갯수 초기화 */
  buffer->in = 0;   /* 버퍼 시작 인덱스 초기화*/
  buffer->out = 0;  /* 버퍼 끝 인덱스 초기화 */
}

ErrorStatus putByteToBuffer(volatile uartFIFO_TypeDef *buffer, uint8_t ch)
{
    /**
    1. 버퍼에 1Byte 데이터를 저장한다.
    **/
    if(buffer->count==UART_BUFFER_SIZE)  /* 데이터가 버퍼에 가득 찼으면 ERROR 리턴 */
        return ERROR;
    buffer->buff[buffer->in++]=ch;        /* 버퍼에 1Byte 저장 */
    buffer->count++;                      /* 버퍼에 저장된 갯수 1 증가 */
    if(buffer->in==UART_BUFFER_SIZE)     /* 시작 인덱스가 버퍼의 끝이면 */
        buffer->in=0;                     /* 시작 인덱스를 0부터 다시 시작 */
    return SUCCESS;
}

ErrorStatus getByteFromBuffer(volatile uartFIFO_TypeDef *buffer, uint8_t *ch)
{
    /**
    1. 버퍼에 1Byte 데이터를 읽는다
    **/
    if(buffer->count==0)                  /* 버퍼에 데이터가 없으면 ERROR 리턴 */
        return ERROR;
    *ch=buffer->buff[buffer->out];        /* 버퍼에서 1Byte 읽음 */
    buffer->buff[buffer->out++] = 0;
    buffer->count--;                      /* 버퍼에 저장된 데이터 갯수 1 감소 */
    if(buffer->out==UART_BUFFER_SIZE)    /* 끝 인덱스가 버퍼의 끝이면 */
        buffer->out=0;                    /* 끝 인덱스를 0부터 다시 시작 */
    return SUCCESS;
}

ErrorStatus isBufferEmpty(volatile uartFIFO_TypeDef buffer)
{
    /**
    1. 버퍼에 데이터가 있는지 확인한다.
    **/
    if(buffer.count==0)                   /* 버퍼에 데이터가 없으면 SUCCESS 리턴 */
        return SUCCESS;
    return ERROR;
}

void initMessage(message_TypeDef* messageFrame, void ( *parsingFunction) (void))
{
	/**
	1. 메시지 프레임 구조체를 초기화 한다.
	**/
	messageFrame->StartFlag = RESET;
	messageFrame->nextStage = START;
 	messageFrame->type = 0;
	messageFrame->length = 0;
	messageFrame->checksum = 0;
	messageFrame->parsing = parsingFunction;

}

void procPacketAnalysis(message_TypeDef* messageFrame, volatile uartFIFO_TypeDef* buffer)
{
	/**
	1. 수신 버퍼의 데이터를 1개씩 읽어와 분석한다.
	 - START : 메시지의 STX을 찾기, STX을 찾을 때 까지 수신 버퍼를 1바이트씩 버리기
	 - MESSAGETYPE : MSG ID 분석하여 메시지 크기 할당
	 - DATA : 수신버퍼에서 메시지 프레임으로 데이터 복사
	 - CHECKSUM : 수신된 메시지의 체크섬 확인
	 - PARSING : 메시지 프레임이 정상이면 initMessage에서 등록된 콜백 함수 호출
	**/

	switch(messageFrame->nextStage)
	{
		case START :
		{
            if( ( buffer->buff[buffer->out] == MESSAGE_UART_STX) && (buffer->buff[buffer->out+1] == MESSAGE_UART_STX)) /* 수신버퍼에 STX가 2바이트 연속되면 */
            {
                if(ERROR == getByteFromBuffer(buffer,(void *)&messageFrame->data[0]))                   /* 수신버퍼의 1번째 바이트 읽어 메시지 프레임 데이터에 넣기 */
                {
                    return;
                }
                if(ERROR == getByteFromBuffer(buffer,(void *)&messageFrame->data[1]))                   /* 수신버퍼의 2번째 바이트 읽어 메시지 프레임 데이터에 넣기 */
                {
                    return;
                }
                messageFrame->nextStage = MESSAGETYPE;                              /* 메시지 프레임이 시작되었음으로 MSG ID 찾는 단계로 넘어가기 */
            }
            else if(buffer->count > 2)
			{
				if(( buffer->buff[buffer->out] == MESSAGE_UART_STX) && (buffer->buff[buffer->out+1] == MESSAGE_UART_STX))
					break;
				getByteFromBuffer(buffer,(void *)&buffer->ch);
                #ifdef _DEBUG_
				    debug_printf("%x ",buffer->ch);
                #endif
			}	
			break;			
		}	        
		case MESSAGETYPE :
		{
            if(buffer->count >= 4)                                               /* 수신버퍼에 4개 이상의 데이터가 있으면 */
            {
                getByteFromBuffer(buffer,(void *)&messageFrame->data[2]);       /* 1. 수신버퍼의 3번째 바이트 읽어 메시지 프레임 데이터에 넣음 */
                getByteFromBuffer(buffer,(void *)&messageFrame->data[3]);       /* 1. 수신버퍼의 4번째 바이트 읽어 메시지 프레임 데이터에 넣음 */
                getByteFromBuffer(buffer,(void *)&messageFrame->data[4]);       /* 1. 수신버퍼의 5번째 바이트 읽어 메시지 프레임 데이터에 넣음 */                
                getByteFromBuffer(buffer,(void *)&messageFrame->data[5]);       /* 1. 수신버퍼의 6번째 바이트 읽어 메시지 프레임 데이터에 넣음 */
                messageFrame->type = messageFrame->data[4];                     /* 2. MSG ID 할당 */
                messageFrame->datasize = messageFrame->data[5];                   /* 3. MSG ID에 해당하는 MSG 크기 할당 */
                messageFrame->length = (messageFrame->datasize + MESSAGE_UART_SIZE_WITHOUT_DATA);            /* 4. 2Byte(STX 및 MSG ID)을 제외한 MSG DATA 크기 할당 */

                #ifdef _DEBUG_
                    debug_printf("\r\nMessage Start Type: %x, length: %d, DataSize: %d\r\n",messageFrame->type, messageFrame->length, messageFrame->datasize);
                #endif

                if(messageFrame->length != 0)                                   /* 5-1. 메시지 길이가 0이 아니면 */
                {
                    messageFrame->nextStage = DATA;                             /* DATA (체크섬 및 ETX) 찾는 단계로 넘어가기 */
                }
                else                                                            /* 5-2. 메시지 길이가 0이면 */
                {
                    messageFrame->nextStage = START;                            /* START 단계로 돌아가기 */
                }
            }
            break;
		}
		case DATA :
		{
            if(buffer->count >= messageFrame->datasize)                         /* 수신버퍼에 MSG DATA 크기 이상의 데이터가 있으면 */
            {
                for(int i=0; i < messageFrame->datasize; i++)                   /* 수신버퍼의 데이터를 데이터 프레임에 복사 */
                {

                    getByteFromBuffer(buffer,(void *)&buffer->ch);
                    messageFrame->data[i+MESSAGE_UART_HEADER_SIZE] = buffer->ch;
                    #ifdef _DEBUG_
                        debug_printf("%x ",buffer->ch);
                    #endif
                }
			}
			break;
		}		
		case CHECKSUM :
		{
            if (buffer->count >= 2)
            {
                getByteFromBuffer(buffer,(void *)&messageFrame->data[messageFrame->length-2]);       /* 체크섬 읽기 */
                getByteFromBuffer(buffer,(void *)&messageFrame->data[messageFrame->length-1]);       /* ETX 읽기 */
    
                if (messageFrame->data[messageFrame->length - 1] == MESSAGE_UART_ETX) /* 메시지 끝 바이트가 ETX로 끝나면 */
                {
                    messageFrame->checksum = messageFrame->data[messageFrame->length - 2]; /* 체크섬 값을 프레임의 체크섬 변수에 저장하고 */
                }
                else /* 메시지 끝 바이트가 ETX가 아니면 */
                {
                    messageFrame->nextStage = START; /* START 단계로 돌아가기 */
                    return;
                }

                uint8_t checksum = 0; /* 체크섬 계산을 위한 지역 변수 선언 */

                for (int i = 0; i < messageFrame->length - 2; i++) /* 2번째 바이트부터 체크섬 앞 바이트까지 더하기 */
                {
                    checksum ^= messageFrame->data[i];
                }

                if (messageFrame->checksum == checksum) /* 체크섬 값이 정상이면 */
                {
                    messageFrame->checksumStatus = SUCCESS; /* 메시지의 체크섬 상태는 정상 입력하고 */
                    messageFrame->nextStage = PARSING;      /* Parsing 단계로 넘어가기 */
                }
                else /* 체크섬 값이 정상이 아니면 */
                {
                    messageFrame->checksumStatus = ERROR; /* 메시지의 체크섬 상태는 에러로 입력하고 */
                    #ifdef _DEBUG_
                        debug_printf(" Cheksum Error Type: %x, Rev.: %x, Cal: %x\r\n", messageFrame->type, messageFrame->checksum, checksum);
                    #endif

                    messageFrame->nextStage = START; /* START 단계로 돌아가기 */
                }
            }
            break;
		}	
		case PARSING :
		{
			messageFrame->parsing();                                    /* initMessage 함수에서 등록된 callback 함수를 호출하고 */
			messageFrame->nextStage = START;                            /* 해당 메시지 데이터를 처리 후 START 단계로 돌아감 */
			break;
		}											
		default :
			break;
	}

}

void initControllerMessage(void)
{
    /**
    1. 컴퓨터와 통신하기 위한 수신 버퍼 구조체 및 메시지 구조체를 초기화 한다.
    2. 컴퓨터에서 수신된 UART 인터럽트 발생시 호출될 함수를 등록한다.
    3. 정상적인 메시지 프레임 수신 시 호출 될 함수를 등록 한다.
    **/
    //initCallbackPutBufferFunction(COM_CONTROLLER, putControllerUartBuffer);   /* COM2에서 UART 인터럽트 발생시 호출될 함수를 등록 */
    vcom_RxInit(controllerRxCpltCallback, &struControllerUartBuffer.ch);  
    initBuffer(&struControllerUartBuffer);                                      /* sContollerUartBuffer 수신버퍼 구조체 초기화 */
    initMessage(&struRecvControllerMessage, parsingControllerMessage);          /* struRecvControllerMessage 수신 메시지 구조체 초기화 및 메시지 파싱 함수 등록*/
    debug_printf("ControllerMessage Init.\r\n");
}
void controllerRxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    //debug_printf("%c",struControllerUartBuffer.ch);
    putControllerUartBuffer(struControllerUartBuffer.ch);
    HAL_UART_Receive_DMA(UartHandle,&struControllerUartBuffer.ch,1);
}

void putControllerUartBuffer(uint8_t ucByte)
{
    /**
    1. UART 인터럽트 발생 시 호출되어 수신버퍼에 1Byte을 저장한다.
    **/    
    putByteToBuffer(&struControllerUartBuffer,ucByte);
}

void procControllerMessage(void)
{
    /**
    1. 수신버퍼에 저장된 데이터가 정상적인 메시지 프레임인지 확인한다.
    **/
    if (messageInitialized)
    {
        procPacketAnalysis(&struRecvControllerMessage, &struControllerUartBuffer);
    }
    else
    {
        initControllerMessage();
        messageInitialized = true;
    }
}

void sendMessageToController(uint8_t messageType, uint8_t *Data, uint8_t size)
{
    /**
    1. 송신 메시지의 체크섬을 계산한다.
    2. 메시지를 송신한다.
    **/

    uint8_t txBuffer[MESSAGE_MAX_SIZE] = {0,};
    uint8_t checksum = 0;										/* 체크섬 계산을 위한 지역 변수 선언 */

    if(size >= MESSAGE_MAX_SIZE)
    {
        debug_printf("Message length over buffer size. TYPE:%x\r\n",messageType);
        return;
    }
		
    txBuffer[0] = MESSAGE_UART_STX;
    txBuffer[1] = MESSAGE_UART_STX;
    txBuffer[2] = messageType;
    txBuffer[3] = size;

    if(size > MESSAGE_UART_SIZE_WITHOUT_DATA)
    {
        memcpy((void *)&txBuffer[4], (void *)Data, size-MESSAGE_UART_SIZE_WITHOUT_DATA);
    }

    for(int i = 0; i < (size - 2); i++)			                /* 1번째 바이트부터 체크섬 앞 바이트까지 XOR */
    {
        checksum ^= txBuffer[i];
    }
    txBuffer[size-1] = MESSAGE_UART_ETX;

    //HAL_UART_Transmit(&huart2, txBuffer, size, 0xFFFF);
}

void parsingControllerMessage(void)
{
    /**
    1. 수신 메시지가 정상적인 메시지 프레임이면 호출된다.
    2. 메시지 ID에 해당한 데이터를 처리/응답한다.
    **/     
    switch(struRecvControllerMessage.type)
    {
        case 0x0 :
        {
            debug_printf("Write Register\r\n");
            break;
        }      
        default :
            debug_printf("Type None\r\n");
            break;
    }
}
