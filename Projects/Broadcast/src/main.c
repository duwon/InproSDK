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


#define TX_INTERVAL_TIME                            20000

static  TimerEvent_t timerLed; /* Led Timers objects */
static  TimerEvent_t timerTx; /* Tx Timers objects */

/* Private function prototypes -----------------------------------------------*/
static void OnledEvent( void* context ); /* brief Function executed on when led timer elapses */
static void OnTxEvent( void* context );
static void mainTimerInit(void);
static void procLoraStage(void);
static uint32_t createUID(void);

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


#ifdef _DEBUG_
            //Message Test   
            uint8_t tempSrcID;
            uint8_t tempRxBuffer[MESSAGE_MAX_PAYLOAD_SIZE] = {0,};
            if(getMessagePayload((void*)&tempSrcID, tempRxBuffer) == SUCCESS)
            {
                if(tempRxBuffer[0] == MTYPE_TESTMESSAGE)
                {
                    PRINTF("ID : %d  T : %d\r\n",tempRxBuffer[1], tempRxBuffer[2]);
                }
            }
#endif

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
#endif

    /* Tx Timers */
    TimerInit(&timerTx, OnTxEvent);
    TimerSetValue(&timerTx, TX_INTERVAL_TIME);
    TimerStart(&timerTx);
}

static void OnledEvent(void *context)
{
    //LED_Toggle( LED1 ) ;
    TimerStart(&timerLed);
}

static void OnTxEvent(void *context)
{
    if (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_RESET)
    {
        srcID = MASTER_ID;
        isMasterMode = true;
        PRINTF("key pressed. ID:%d \r\n", srcID);
        payloadTimerDeInit();
    }

    if (isMasterMode == true)
    {
        TimerStop(&timerTx);
    }
    else
    {
        TimerStart(&timerTx);
    }

    sensor_t sensor_data;
    uint8_t tempTxData[8] = {0,};

    BSP_sensor_Read( &sensor_data );
    memcpy((void *)&tempTxData[0], (void *)&sensor_data.temperature, 4);
    memcpy((void *)&tempTxData[4], (void *)&sensor_data.humidity, 4);

    PRINTF("Temp : %.1f     Humi : %.1f   \r\n\r\n",sensor_data.temperature, sensor_data.humidity);
    sendPayloadData(MTYPE_TEMP_HUMI, tempTxData);



}

static uint32_t createUID(void)
{
    /* create random uid */
    sensor_t sensor_data;
    BSP_sensor_Read( &sensor_data );
    srand(sensor_data.temperature * sensor_data.humidity);
    UID_radom = rand();
    PRINTF("UID : %x\r\n", UID_radom);
    InsertIDList(UID_radom);
}

