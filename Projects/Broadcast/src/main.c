/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include "LoraRadio.h"
#include "LoraMessage.h"
#include "PayloadMessage.h"
#include "bsp.h"


#define TX_INTERVAL_TIME                            10000
#define NUMBER_RETRANSMISSION                       1

static  TimerEvent_t timerLed; /* Led Timers objects */
static  TimerEvent_t timerTx; /* Tx Timers objects */

/* Private function prototypes -----------------------------------------------*/
static void OnledEvent( void* context ); /* brief Function executed on when led timer elapses */
static void OnTxEvent( void* context );
static void mainTimerInit(void);
static void procLoraStage(void);
static void createUID(void);

static  TimerEvent_t timerTx1; /* Tx Timers objects */
static  TimerEvent_t timerTx2; /* Tx Timers objects */
static void OnTxEvetTest1(void* context);
static void OnTxEvetTest2(void* context);

int main( void )
{
    HAL_Init( );
    SystemClock_Config( );

    DBG_Init( );
    HW_Init( );  
    mainTimerInit( );
    pyaloadTimerInit();

    LPM_SetOffMode(LPM_APPLI_Id , LPM_Disable ); /*Disbale Stand-by mode*/

    RadioInit();

    initMessage();    
    createUID();


    while( 1 )
    {
        procLoraStage();  

        if(isMasterMode == true)
        {
            procMasterMode();
        }
        else if(existGetID == true)
        {
            procPayloadData();

            DISABLE_IRQ( );
            if (State == LOWPOWER) /* if an interupt has occured after __disable_irq, it is kept pending and cortex will not enter low power anyway */
            {
                LPM_EnterLowPower( );
            }
            else
            {
            }
            ENABLE_IRQ( );
        }        
    }
}

static void procLoraStage(void)
{
    static uint8_t cntTxSent = 0;

    switch (State)
    {
    case RX_DONE:
        if (isMasterMode == true)
        {
            Radio.Rx(RX_TIMEOUT_VALUE);
        }

#ifdef _DEBUG_
        LED_Toggle(LED1);
#endif
        State = LOWPOWER;
        break;
    case TX_DONE:
        Radio.Rx(RX_TIMEOUT_VALUE);
#ifdef _DEBUG_
        LED_Toggle(LED2);
#endif
        State = LOWPOWER;
        break;
    case RX_TIMEOUT:
    case RX_ERROR:
        if (isMasterMode == true)
        {
            Radio.Rx(RX_TIMEOUT_VALUE);
        }
        else if(cntTxSent < NUMBER_RETRANSMISSION)              /* 재전송 */
        {
            cntTxSent++;
            PRINTF("RESEND\r\n");
            OnTxEvent(NULL);
        }
        else
        {
            cntTxSent = 0;
        }
        

        State = LOWPOWER;
        break;
    case TX_TIMEOUT:

        State = LOWPOWER;
        break;
    case LOWPOWER:
    default:
        // Set low power
        break;
    }
}

static void mainTimerInit(void)
{
#ifdef _DEBUG_
    /* Led Timers */
    TimerInit(&timerLed, OnledEvent);
    TimerSetValue(&timerLed, 1000);
    TimerStart(&timerLed);

    /* Test Event 1*/
    TimerInit(&timerTx1, OnTxEvetTest1);
    TimerSetValue(&timerTx1, 6000);
    TimerStart(&timerTx1);

    /* Test Event 1*/
    TimerInit(&timerTx2, OnTxEvetTest2);
    TimerSetValue(&timerTx2, 9000);
    TimerStart(&timerTx2);    
#endif

    /* Tx Timers */
    TimerInit(&timerTx, OnTxEvent);
    TimerSetValue(&timerTx, TX_INTERVAL_TIME);
    TimerStart(&timerTx);
}

static void OnTxEvetTest1(void *context)
{
    uint8_t tempTxData[3] = {0,};
    static uint8_t cntTemp = 0;
    tempTxData[0] = cntTemp++;
    tempTxData[1] = rand();
    tempTxData[2] = (uint8_t)(rand() % cntTemp);

    sendPayloadData(MASTER_ID, MTYPE_TESTMESSAGE1, tempTxData);

    TimerStart(&timerTx1);
}

static void OnTxEvetTest2(void *context)
{
    uint8_t tempTxData[13] = {0,};
    static uint8_t cntTemp = 0;
    tempTxData[0] = cntTemp++;
    tempTxData[1] = rand();
    tempTxData[12] = rand() % cntTemp;

    sendPayloadData(MASTER_ID, MTYPE_TESTMESSAGE1, tempTxData);

    TimerStart(&timerTx2);
}

static void OnledEvent(void *context)
{
    //LED_Toggle( LED1 ) ;
    TimerStart(&timerLed);
}

/**
  * @brief  온/습도 센서 정보를 주기적으로 송신
  */
static void OnTxEvent(void *context)
{
    if ((BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_RESET) || (isMasterMode == true)) /* 버튼으로 마스터 모드 설정하거나, 컴파일시 MASTER_MODE 선언 시 */
    {
        srcID = MASTER_ID;
        isMasterMode = true;
        PRINTF("key pressed. ID:%d \r\n", srcID);
        payloadTimerDeInit();
        TimerStop(&timerTx);
        Radio.Rx(RX_TIMEOUT_VALUE);
        return;
    }

    sensor_t sensor_data;
    uint8_t tempTxData[8] = {0,};

    BSP_sensor_Read( &sensor_data );
    memcpy((void *)&tempTxData[0], (void *)&sensor_data.temperature, 4);
    memcpy((void *)&tempTxData[4], (void *)&sensor_data.humidity, 4);

    PRINTF("Temp : %.1f     Humi : %.1f   \r\n\r\n",sensor_data.temperature, sensor_data.humidity);
    sendPayloadData(MASTER_ID, MTYPE_TEMP_HUMI, tempTxData);

    TimerStart(&timerTx);
}

/**
  * @brief  온/습도 정보를 기준으로 랜덤 UID 생성
  */
static void createUID(void)
{
    /* create random uid */
    sensor_t sensor_data;
    BSP_sensor_Read( &sensor_data );
    srand(sensor_data.temperature * sensor_data.humidity);
    UID_random = rand();
    PRINTF("UID : %x\r\n", UID_random);
    InsertIDList(UID_random);  
}

