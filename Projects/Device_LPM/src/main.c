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
static void enterLowPowerMode(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
	
		//LPM_Test();
    HW_Init();

    LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable); /*Disbale Stand-by mode*/

    LoraRadio_Init();
    LoraMessage_Init();

    mainTimerInit();

		BSP_LED_On(LED1);
		HAL_Delay(2000);
    enterLowPowerMode();
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
    PRINTF("\r\n\r\n\r\nSTART Device....... UID : 0x");
    for (int i = 0; i < 8; i++)
        PRINTF("%X ", UID[i]);
    PRINTF("   Device ID : 0x%x\r\n\r\n", DEVICE_ID);
#endif

    /* Tx Timers */
    TimerInit(&timerTx, OnTxEvent);
    TimerSetValue(&timerTx, TX_INTERVAL_TIME);
    TimerStart(&timerTx);
}

#define RTC_ASYNCH_PREDIV    0x7C
#define RTC_SYNCH_PREDIV     0x0127
void enterLowPowerMode(void)
{
	RTC_HandleTypeDef RTCHandle;
  /* Configure the system Power */
	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	HAL_PWREx_EnableUltraLowPower();

	/* Enable the fast wake up from Ultra low power mode */
	HAL_PWREx_EnableFastWakeUp();

	/* Configure RTC */
	RTCHandle.Instance = RTC;
	/* Set the RTC time base to 1s */
	/* Configure RTC prescaler and RTC data registers as follow:
		- Hour Format = Format 24
		- Asynch Prediv = Value according to source clock
		- Synch Prediv = Value according to source clock
		- OutPut = Output Disable
		- OutPutPolarity = High Polarity
		- OutPutType = Open Drain */
	RTCHandle.Init.HourFormat = RTC_HOURFORMAT_24;
	RTCHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
	RTCHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
	RTCHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
	RTCHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	RTCHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if(HAL_RTC_Init(&RTCHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler(); 
	}

  /* Check and handle if the system was resumed from StandBy mode */ 
  if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  {
    /* Clear Standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); 
  }
  
  /* The Following Wakeup sequence is highly recommended prior to each Standby mode entry
    mainly  when using more than one wakeup source this is to not miss any wakeup event.
    - Disable all used wakeup sources,
    - Clear all related wakeup flags, 
    - Re-enable all used wakeup sources,
    - Enter the Standby mode.
  */
  /* Disable all used wakeup sources*/
  HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle);
  
  /* Re-enable all used wakeup sources*/
  /* ## Setting the Wake up time ############################################*/
  /*  RTC Wakeup Interrupt Generation:
    Wakeup Time Base = (RTC_WAKEUPCLOCK_RTCCLK_DIV /(LSI))
    Wakeup Time = Wakeup Time Base * WakeUpCounter 
      = (RTC_WAKEUPCLOCK_RTCCLK_DIV /(LSI)) * WakeUpCounter
      ==> WakeUpCounter = Wakeup Time / Wakeup Time Base
  
    To configure the wake up timer to 4s the WakeUpCounter is set to 0x1FFF:
    RTC_WAKEUPCLOCK_RTCCLK_DIV = RTCCLK_Div16 = 16 
    Wakeup Time Base = 16 /(~39.000KHz) = ~0,410 ms
    Wakeup Time = ~4s = 0,410ms  * WakeUpCounter
      ==> WakeUpCounter = ~4s/0,410ms = 9750 = 0x2616 */
	uint32_t WakeUpCounter = TX_INTERVAL_TIME * 24.39;
  HAL_RTCEx_SetWakeUpTimer_IT(&RTCHandle, WakeUpCounter, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
  
  /* Clear all related wakeup flags */
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  
  /* Enter the Standby mode */
  HAL_PWR_EnterSTANDBYMode();
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



