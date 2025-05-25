#define __TASK_INIT_C__
    #include "TaskInit.h"
#undef  __TASK_INIT_C__

#include "LED.h"
#include "Buzzer.h"
#include "DIO.h"

#include "IUART.h"
#include "ITIMER.h"
#include "II2C.h"
//#include "DAC8564.h"
#include "DAC8568.h"
#include "ADC108S022.h"
#include "MCP23S08.h"

#include "ControlTask.h"
#include "SerialCommTask.h"

static void SysMoniter_Task(void const * argument);
static void SysMoniter_TaskInit(void);
static void Hardware_Init(void);

void Task_Init(void)
{  
    Hardware_Init();
    
    AppParam_Init();
    //IUART_Init(&AppParam.rs232, &AppParam.rs485A, &AppParam.rs485B);
    IUART_Init(&AppParam.rs232, &AppParam.rs485A, &AppParam.rs232S1, &AppParam.rs232S2);
    
    SysMoniter_TaskInit();
    ControlTask_Init();
    SerialCommTask_Init();
    
    osKernelStart();
}

static void Hardware_Init(void)
{
    LED_Init();
    DIO_Init();
    BUZZER_Init();
    II2C_Init();
    DAC8568_Init();
    ADC108S022_Init();
    MCP23S08_Init();    
}

static void SysMoniter_TaskInit(void)
{ 
    osThreadDef(SYSTEM_MONITOR, SysMoniter_Task, osPriorityLow, 0, 128);
    osThreadCreate (osThread(SYSTEM_MONITOR), NULL);  
}

static void SysMoniter_Task(void const * argument)
{ 
    static U32 FreeHeapSize;
    U32 monitorTick = 0;
    
    while(1)
    {
        if(HAL_GetTick() - monitorTick > 1000)
        {
            monitorTick = HAL_GetTick();            
            FreeHeapSize = xPortGetFreeHeapSize();

            //LED_Toggle(STATUS0);
            LED_OnOff(STATUS0, 1);
        }
        
        vTaskDelay(1);
    }
}
