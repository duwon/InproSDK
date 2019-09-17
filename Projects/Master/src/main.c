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
//static  TimerEvent_t timerTx; /* Tx Timers objects */

/* Private function prototypes -----------------------------------------------*/
static void OnledEvent( void* context ); /* brief Function executed on when led timer elapses */
//static void OnTxEvent( void* context );
static void procLoraStage(void);

//static  TimerEvent_t timerTx1; /* Tx Timers objects */
//static  TimerEvent_t timerTx2; /* Tx Timers objects */
//static void OnTxEvetTest1(void* context);
//static void OnTxEvetTest2(void* context);

int main( void )
{
    HAL_Init( );
    SystemClock_Config( );

    DBG_Init( );
    HW_Init( ); 
	
    LPM_SetOffMode(LPM_APPLI_Id , LPM_Disable ); /*Disbale Stand-by mode*/

    LoraRadio_Init();
    LoraMessage_Init();
  
#ifdef _DEBUG_
    USBPRINT("\r\n\r\n\r\nSTART MASTER..... UID : 0x");
    for(int i=0; i<8; i++)
        USBPRINT("%X ",UID[i]);
    USBPRINT("   Master ID : 0x%x\r\n\r\n", MASTER_ID);
	
		/* Led Timers */
    TimerInit(&timerLed, OnledEvent);
    TimerSetValue(&timerLed, 1000);
    OnledEvent(&timerLed);
#endif

    InsertIDList(0, 0); 
    pyaloadTimerInit();
	Radio_Rx();
	
    while( 1 )
    {
        procLoraStage();  
        procPayloadData();
        procControllerMessage();
    }
}

static void procLoraStage(void)
{

    switch (LoraState)
    {
    case RX_DONE:

        Radio_Rx();
        

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
        Radio_Rx();
        LoraState = LOWPOWER;
        break;
    case TX_TIMEOUT:

        LoraState = LOWPOWER;
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
    //static uint8_t cntLED = 0;
    //USBPRINT("LED Toggle %d\r\n",cntLED++);
    TimerStart(&timerLed);
}

void payloadDataCallback(uint8_t rxSrcID, payloadPacket_TypeDef* payloadData)
{
    float temperature = 0;
    float humidity = 0;

    switch(payloadData->data[0])
    {
        case MTYPE_TEMP_HUMI:
            memcpy((void *)&temperature, (void *)&payloadData->data[0], 4);
            memcpy((void *)&humidity, (void *)&payloadData->data[4], 4);

            USBPRINT("SRC ID : %d, Temp: %.1f, Humi: %.f \r\n", rxSrcID, temperature, humidity);

            break;
        default:
            break;
    }
}
