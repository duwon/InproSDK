/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "hw.h"
#include "LoraMessage.h"

#define debug_printf(...)                   USB_Send(__VA_ARGS__)

#define UART_BUFFER_SIZE                    ((uint8_t) 200U)            /* 송수신 버퍼 최대 크기, MAX 255 */
#define MESSAGE_MAX_SIZE                    (MESSAGE_SIZE)              /* 메시지 최대 크기 */
#define MESSAGE_UART_HEADER_SIZE            (MESSAGE_HEADER_SIZE - 2)   /* 메시지 시작부터 Length까지의 크기 */
#define MESSAGE_UART_SIZE_WITHOUT_DATA      (MESSAGE_HEADER_SIZE)       /* 데이터 크기를 제외한 나머지 메시지 크기 (Header + Checksum + ETX) */

/* Message frame define --------------------------------*/
#define MESSAGE_UART_STX                     0xA5                       /* UART MSG STX */
#define MESSAGE_UART_ETX                     0x04                       /* UART MSG ETX */


typedef enum
{
  START = 0,
  MESSAGETYPE = 1,
  DATA = 2,
  PARSING = 3,
  CHECKSUM = 4,
  SEND = 5,
  RESEND = 6,
  STOP = 7,
  WRITE = 8,
  END = 9
} MessageStage;

typedef struct {
	uint8_t in;
	uint8_t out;
	uint8_t count;
	uint8_t buff[UART_BUFFER_SIZE];
	uint8_t ch;
} uartFIFO_TypeDef;

typedef struct {
  FlagStatus StartFlag;
  MessageStage nextStage;
  uint8_t index;
  uint8_t dest;
  uint8_t src;
  uint8_t type;
  uint8_t length;   /* 메시지 전체 길이 */
  uint8_t datasize; /* Payload size */
  uint8_t data[MESSAGE_MAX_SIZE];
  uint8_t checksum;
  ErrorStatus checksumStatus;
  void ( *parsing) (void);
} message_TypeDef;



void procControllerMessage(void);

/*}*/
#ifdef __cplusplus
}
#endif

#endif /* __MESSAGE_H__ */
