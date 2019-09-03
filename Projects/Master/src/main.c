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
#define NUMBER_RETRANSMISSION                       1

static  TimerEvent_t timerLed; /* Led Timers objects */
static  TimerEvent_t timerTx; /* Tx Timers objects */

/* Private function prototypes -----------------------------------------------*/
static void OnledEvent( void* context ); /* brief Function executed on when led timer elapses */
static void OnTxEvent( void* context );
static void mainTimerInit(void);
static void procLoraStage(void);

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
  
#ifdef _DEBUG_
    /* Led Timers */
    TimerInit(&timerLed, OnledEvent);
    TimerSetValue(&timerLed, 1000);
    OnledEvent(&timerLed);
    PRINTF("\r\n\r\n\r\nSTART MASTER..... UID : 0x%x   Master ID : 0x%x\r\n\r\n",UID, MASTER_ID);
#endif

    InsertIDList(0, UID); 
    pyaloadTimerInit();

    while( 1 )
    {
        procLoraStage();  
        procPayloadMessage();
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
        else if(cntTxSent < NUMBER_RETRANSMISSION)              /* Node 모드에서 재전송 */
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

static void OnledEvent(void *context)
{
    //LED_Toggle( LED1 ) ;
    TimerStart(&timerLed);
}
