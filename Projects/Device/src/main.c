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

static  TimerEvent_t timerLed; /* Led Timers objects */
static  TimerEvent_t timerTx; /* Tx Timers objects */

/* Private function prototypes -----------------------------------------------*/
static void OnledEvent( void* context ); /* brief Function executed on when led timer elapses */
static void OnTxEvent( void* context );
static void procLoraStage(void);
static void mainTimerInit(void);

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

    LPM_SetOffMode(LPM_APPLI_Id , LPM_Disable ); /*Disbale Stand-by mode*/

    Radio_Init();
    Message_Init();
    
    pyaloadTimerInit();
    mainTimerInit();

    while( 1 )
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

#ifdef _DEBUG_
        LED_Toggle(LED1);
#endif
        LoraState = LOWPOWER;
        break;
    case TX_DONE:
        Radio_Rx();
#ifdef _DEBUG_
        LED_Toggle(LED2);
#endif
        LoraState = LOWPOWER;
        break;
    case RX_TIMEOUT:
    case RX_ERROR:

        
        if(cntTxSent < NUMBER_RETRANSMISSION)              /* Node 모드에서 재전송 */
        {
            cntTxSent++;
            PRINTF("RESEND\r\n");
            Radio_Resend();
        }
        else
        {
            cntTxSent = 0;
        }
        

        LoraState = LOWPOWER;
        break;
    case TX_TIMEOUT:

        LoraState = LOWPOWER;
        break;\
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
    for(int i=0; i<8; i++)
        PRINTF("%X ",UID[i]);
    PRINTF("   Device ID : 0x%x\r\n\r\n", DEVICE_ID);

    /* Led Timers */
    TimerInit(&timerLed, OnledEvent);
    TimerSetValue(&timerLed, 1000);
    OnledEvent(&timerLed);

    /* Test Event 1*/
    TimerInit(&timerTx1, OnTxEvetTest1);
    TimerSetValue(&timerTx1, 6000);
    TimerStart(&timerTx1);

    /* Test Event 2*/
    TimerInit(&timerTx2, OnTxEvetTest2);
    TimerSetValue(&timerTx2, 7000);
    TimerStart(&timerTx2);    
#endif

    /* Tx Timers */
    TimerInit(&timerTx, OnTxEvent);
    TimerSetValue(&timerTx, TX_INTERVAL_TIME);
    TimerStart(&timerTx);
}

static void OnTxEvetTest1(void *context)
{
    if((LoraState == TX_RUNNING) || (LoraState == RX_RUNNING))
    {
        TimerSetValue(&timerTx1, 100);
        TimerStart(&timerTx1);
    }
    else
    {
        uint8_t tempTxData[3] = {0,};
        static uint8_t cntTemp = 0;
        tempTxData[0] = 0xC1;
        tempTxData[1] = cntTemp++;
        tempTxData[2] = rand();
        sendPayloadData(MASTER_ID, MTYPE_TESTMESSAGE1, tempTxData);

        TimerSetValue(&timerTx1, 2300);
        TimerStart(&timerTx1);        
    }
    


}

static void OnTxEvetTest2(void *context)
{
    if((LoraState == TX_RUNNING) || (LoraState == RX_RUNNING))
    {
        TimerSetValue(&timerTx2, 100);
        TimerStart(&timerTx2);
    }
    else
    {
        uint8_t tempTxData[24] = {0,};
        static uint8_t cntTemp = 0;
        tempTxData[0] = 0xC2;
        tempTxData[1] = cntTemp++;
        tempTxData[23] = rand();
        sendPayloadData(MASTER_ID, MTYPE_TESTMESSAGE2, tempTxData);

        TimerSetValue(&timerTx2, 3000);
        TimerStart(&timerTx2);
    }
}

static void OnledEvent(void *context)
{
    if (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_RESET) 
    {
        PRINTF("key pressed. \r\n");
        srcID++;
    }

    //LED_Toggle( LED1 ) ;
    TimerStart(&timerLed);
}

static void OnTxEvent(void *context)
{
    if((LoraState == TX_RUNNING) || (LoraState == RX_RUNNING))
    {
        TimerSetValue(&timerTx, 100);
        TimerStart(&timerTx);
    }
    else
    {
        sensor_t sensor_data;
        uint8_t tempTxData[8] = {0,};

        BSP_sensor_Read( &sensor_data );
        memcpy((void *)&tempTxData[0], (void *)&sensor_data.temperature, 4);
        memcpy((void *)&tempTxData[4], (void *)&sensor_data.humidity, 4);

        PRINTF("%d [INFO] ", HW_RTC_GetTimerValue());
        PRINTF("Temp : %.1f     Humi : %.1f   \r\n",sensor_data.temperature, sensor_data.humidity);
        sendPayloadData(MASTER_ID, MTYPE_TEMP_HUMI, tempTxData);

        TimerSetValue(&timerTx, TX_INTERVAL_TIME);
        TimerStart(&timerTx);
    }
}

void payloadDataCallback(uint8_t rxSrcID, payloadPacket_TypeDef* payloadData)
{

    switch(payloadData->msgID)
    {
        case MTYPE_TEMP_HUMI:


            break;
        default:
            break;
    }
}


