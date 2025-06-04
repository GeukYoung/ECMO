#define _CONTROL_TASK_C_
    #include "ControlTask.h"
#undef  _CONTROL_TASK_C_

#include "project.h"

#include "ITIMER.h"
//#include "DAC8564.h"
#include "DAC8568.h"
#include "ADC108S022.h"
#include "MCP23S08.h"
#include "Buzzer.h"

typedef enum {
	INIT = 0,
        CHK_ST_SW = 1,
        
	AUTO = 3,
} State;

static void ControlTask(void const * argument);

void ControlTask_Init(void)
{
    osThreadDef(Control, ControlTask, osPriorityBelowNormal, 0, 128);
    osThreadCreate (osThread(Control), NULL);  
}

static void ControlTask(void const * argument)  // 2msec
{
    U32 ledTick = 0;
    U32 chkStSwTick = 0;
    
    nControlTaskState = 0;
    bool bStartOn = 0;


    ledTick = HAL_GetTick();
    chkStSwTick  = HAL_GetTick();
    
    AppData.IO.bit.LCDPWR = false;
//    AppData.IO.dword = 0x00000000;
//    MCP23S08_SetOut(AppData.IO.dword);
     
    while(1)
    {   
        switch(nControlTaskState)
        {
          case INIT:
            {
                if(HAL_GetTick() - ledTick > 10000)
                {
                    ledTick = HAL_GetTick();
                    bStartOn = Start_On();
                    
                    // Init DA Out & I/O
                    ADDAInit();
                    MCP23S08_Init();
                    
                    ADOffset.inoutreg = 200;      //ADC108S022_GetData(ADC_CH0);
                    ADOffset.mainreg = 200;       //ADC108S022_GetData(ADC_CH1);
                    ADOffset.InletPress = 200;    //ADC108S022_GetData(ADC_CH2);
                    
                    Sys.Pump1CycState = 0;
                    Sys.Pump2CycState = 0;
                    AppData.IO.bit.LCDPWR = true;
                    AppData.IO.bit.BLED = false;
                    AppData.IO.bit.GLED = false;
                    AppData.IO.bit.RLED = false;
                    nControlTaskState = CHK_ST_SW;
                }
            }
            break;
            
          case CHK_ST_SW:
            {
                if(HAL_GetTick() - chkStSwTick > 1000)
                {
                    chkStSwTick = HAL_GetTick();
                    AppData.IO.bit.LCDPWR = true;
                    Sys.g.startswon = Start_On();
                    if(Sys.g.startswon && bStartOn)
                    {
                        Sys.Err.bit.starton = true;
                    }
                    else
                    {
                        bStartOn = false;
                        AppData.IO.bit.BUZZER = false;
                        Sys.Err.bit.starton = false;
                        AppData.IO.bit.RLED = false;
                        AppData.IO.bit.BLED = true;
                        
                        // Init DA Out & I/O
                        ADDAInit();
                        MCP23S08_Init();
                        
                        // Read AD Offset
                        ADOffset.spare[0] = ADC108S022_GetData(ADC_CH3);
                        ADOffset.spare[1] = ADC108S022_GetData(ADC_CH4);
                        ADOffset.spare[2] = ADC108S022_GetData(ADC_CH5);
                        MCP23S08_SetOut(AppData.IO.dword);
                        nControlTaskState = AUTO;
                    }
                } 
            }
            break;
              
          case AUTO:
            {
                if(HAL_GetTick() - ledTick > 1000)
                {
                    ledTick = HAL_GetTick();
                    LED_Toggle(STATUS1);
                }
                CheckError();  
                Seq();
            }
            break;
        }

        ReadWrite();
        vTaskDelay(1);
    }
}







