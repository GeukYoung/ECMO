#ifndef   _SERIAL_COMM_TASK_H_
#define   _SERIAL_COMM_TASK_H_

#include "project.h"

#include "IUART.h"
#include "SerialComm.h"
#include "AppParam.h"

#ifdef _SERIAL_COMM_TASK_C_
	#define SERIAL_COMM_TASK_EXT
#else
	#define SERIAL_COMM_TASK_EXT extern
#endif

// FSM State[Update Data Sequence]
enum  {
    // Update Real Value
    MAIN_G_EN       = 0,        // Mmain screen graph enable
    MAIN_BPM           ,
    MAIN_BLOOD         ,
    MAIN_INLET         ,
    MAIN_VENT          ,
    MAIN_GRAPH         ,
    MAIN_BUBBLE_CHK    ,
    
    LOG            = 50,        // Log update 1s, 10s, 1min, 10min, 30min, 1hr
    
    PARA_G_AVR_EN  = 70,
    PARA_G_AVR_VAL     ,        // 60cnt average flow data
    PARA_G_LOW_D_EN    ,
    PARA_G_LOW_D_VAL   ,        // flow low-data data every 1sec
    PARA_G_AVR_TEXT    ,        // SetText 60cnt average data
    PARA_G_LOW_D_TEXT  ,        // SetText low-data
    
    // Update Power State
    START_ST      = 100,
    AC_ST              ,
    BATT1_ST           ,
    BATT2_ST           ,
    END_ST             ,
    
    
    MAX,
};

SERIAL_COMM_TASK_EXT void SerialCommTask_Init(void);

SERIAL_COMM_TASK_EXT void (*SerialComm_RS485A_WriteBytes)  (U8* data,U16 size);
//SERIAL_COMM_TASK_EXT void (*SerialComm_RS485B_WriteBytes)  (U8* data,U16 size);
SERIAL_COMM_TASK_EXT void (*SerialComm_RS232S1_WriteBytes)  (U8* data,U16 size);
SERIAL_COMM_TASK_EXT void (*SerialComm_RS232_WriteBytes)   (U8* data,U16 size);
SERIAL_COMM_TASK_EXT void (*SerialComm_RS232S2_WriteBytes)  (U8* data,U16 size);

#endif
