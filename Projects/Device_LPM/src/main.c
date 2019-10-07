/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "LoraRadio.h"
#include "LoraMessage.h"
#include "PayloadMessage.h"
#include "bsp.h"
#include "vcom.h"

#define TX_INTERVAL_TIME                            10000
#define NUMBER_RETRANSMISSION                       3

static  TimerEvent_t timerTx; /* Tx Timers objects */

/* Private function prototypes -----------------------------------------------*/
static void OnTxEvent( void* context );
static void procLoraStage(void);
static void mainTimerInit(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    HW_Init();

    LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable); /*Disbale Stand-by mode*/

    LoraRadio_Init();
    LoraMessage_Init();

    mainTimerInit();

    while (1)
    {
        procLoraStage();
        procPayloadData();
    }
}

static void procLoraStage(void)
{
    static uint8_t cntTxSent = 0;

    switch (LoraState)
    {
    case RX_DONE:
        cntTxSent = 0;
        LoraState = LOWPOWER;
        break;
    case TX_DONE:
        Radio_Rx();
        LoraState = LOWPOWER;
        break;
    case RX_TIMEOUT:
    case RX_ERROR:
        if (cntTxSent < NUMBER_RETRANSMISSION) /* 재전송 */
        {
            cntTxSent++;
            PRINTF("%d Resend\r\n", HW_RTC_GetTimerValue());
            Radio_Resend();
        }
        else
        {
            PRINTF("%d Send Fail\r\n", HW_RTC_GetTimerValue());
            cntTxSent = 0;
        }
        LoraState = LOWPOWER;
        break;
    case TX_TIMEOUT:
        LoraState = LOWPOWER;
        break;
    case STANDBY:
    case LOWPOWER:
    default:
        // Set low power
        break;
    }
}

static void mainTimerInit(void)
{
#ifdef _DEBUG_
    PRINTF("\r\n\r\n\r\nSTART Device..... UID : 0x");
    for (int i = 0; i < 8; i++)
        PRINTF("%X ", UID[i]);
    PRINTF("   Device ID : 0x%x\r\n\r\n", DEVICE_ID);
#endif

    /* Tx Timers */
    TimerInit(&timerTx, OnTxEvent);
    TimerSetValue(&timerTx, TX_INTERVAL_TIME);
    TimerStart(&timerTx);
}

static void OnTxEvent(void *context)
{
    if ((LoraState == TX_RUNNING) || (LoraState == RX_RUNNING)) /* 메시지가 송신/수신 중일 경우 100ms 후 다시 실행 */
    {
        TimerSetValue(&timerTx, 100);
        TimerStart(&timerTx);
    }
    else
    {
        sensor_t sensor_data;
        uint8_t tempTxData[9] = {
            0,
        };

        BSP_sensor_Read(&sensor_data);
        tempTxData[0] = MTYPE_TEMP_HUMI;
        memcpy((void *)&tempTxData[1], (void *)&sensor_data.temperature, 4);
        memcpy((void *)&tempTxData[5], (void *)&sensor_data.humidity, 4);
#ifdef _DEBUG_
        PRINTF("%d [INFO] ", HW_RTC_GetTimerValue());
        PRINTF("Temp : %.1f     Humi : %.1f   \r\n", sensor_data.temperature, sensor_data.humidity);
#endif
        sendPayloadData(MASTER_ID, tempTxData, sizeof(tempTxData));
        TimerSetValue(&timerTx, TX_INTERVAL_TIME);
        TimerStart(&timerTx);
    }
}

void payloadDataCallback(uint8_t rxSrcID, payloadPacket_TypeDef *payloadData)
{

    char *stringPrint = malloc(sizeof(char) * payloadData->length);

    switch (payloadData->data[0])
    {
    case 0x01: //USBPRINT
        strcpy(stringPrint, (char *)&payloadData->data[1]);
        PRINTF("Received string from master: %s \r\n", stringPrint);
        free(stringPrint);
        break;
    case 0x02: //LED1 ON Control
        HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET);
        break;
    case 0x03: //LED1 OFF Control
        HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET);
        break;
    case 0x10:
        PRINTF("Received User Data : %x\r\n", payloadData->data[1]);
        break;
    default:
        PRINTF("User data type none\r\n");
        break;
    }
}
