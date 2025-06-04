#ifndef   __APP_DATA_H__
#define   __APP_DATA_H__

#include "project.h"

#ifdef __APP_DATA_C__
	#define APP_DATA_EXT
#else
	#define APP_DATA_EXT extern
#endif

#pragma pack(push,1)
/////////////////////////////////////////
// Digital to Analog Output
// 0~1ch : ITV-1030 (max Pressure 0.5Mpa) 0~10V
// 2~4ch : Main Pump(1420-0501) x 1 , Sub Pump(1410-0211) x 2 
//         0~5V(Max Speed : 0~4,600rpm = 76rps), Tacho 18pulse/rev. max freq : 1,360pps)
// 5~7ch : 
/////////////////////////////////////////
//typedef struct DA_Data_t
//{
//    U16             out[4];
//}DA_Data_t;

typedef struct DA_Data_t
{
    //U16             inp[2];     // Inlet Pressure 1,2 [ITV-1030], 0 ~ 10V
    U16             inoutreg;     // Regurator [ITV-1030], 0 ~ 10V
    U16             mainreg;      // Regurator [ITV-1030], 0 ~ 10V
    
    U16             mpump[2];     // Main Pump1,2 [1410-0211], 24Vin Model, 0 ~ 5V
    U16             spump;        // Sub Pump  [1420-0501], 24Vin Model, 0 ~ 5V

    U16             spare[3];     // Spare x 3, 0 ~ 3.3V
}DA_Data_t;

typedef struct AD_Data_t
{
    //U16         rFB[2];       // Regurator
    U16         inoutreg;       // Regurator1
    U16         mainreg;        // Regurator2
    U16         InletPress;     // DP-102A     
    //U16         inlet;     
    U16         in[3];          // spare[3] PSE563-1
    U16         BatteryMonitor[2];
}AD_Data_t;

typedef struct AD_Offset_t
{
    U16         inoutreg;       // Regurator
    U16         mainreg;        // Regurator
    U16         InletPress;     // DP-102A 
  
    U16         spare[3];       // PSES63-1(SMC)
    U16         BattMoni[2];
}AD_Offset_t;


typedef union IO_Data_t
{
    struct
    {
   
    U32         P1InA     : 1;    // SOL 01    // 1 IN-A
    U32         P1InP     : 1;    // SOL 02    // 1 IN-P
    U32         P1OutA    : 1;    // SOL 03    // 1 OUT-A
    U32         P1OutP    : 1;    // SOL 04    // 1 OUT-P
    U32         P2InA     : 1;    // SOL 05    // 2 IN-A
    U32         P2InP     : 1;    // SOL 06    // 2 IN-P
    U32         P2OutA    : 1;    // SOL 07    // 2 OUT-A
    U32         P2OutP    : 1;    // SOL 08    // 2 OUT-P
    
    U32         P1SacA1   : 1;    // SOL 09    // 1 SAC-A1
    U32         P1SacA2   : 1;    // SOL 10    // 1 SAC-A2
    U32         P1SacP1   : 1;    // SOL 11    // 1 SAC-P1
    U32         P1SacP2   : 1;    // SOL 12    // 1 SAC-P2
    U32         P1SacP3   : 1;    // SOL 13    // 1 SAC-P3
    
    U32         P2SacA1   : 1;    // SOL 14    // 2 SAC-A1
    U32         P2SacA2   : 1;    // SOL 15    // 2 SAC-A2
    U32         P2SacP1   : 1;    // SOL 16    // 2 SAC-P1
    U32         P2SacP2   : 1;    // SOL 17    // 2 SAC-P2
    U32         P2SacP3   : 1;    // SOL 18    // 2 SAC-P3
    
    U32         BUZZER    : 1;
    U32         RLED      : 1;    // YLED
    
    U32         BLED      : 1;    // 
    U32         LCDPWR    : 1;    // Clear Display Power
    U32         GLED      : 1;    // FAN1
    U32         TOMASPUMP : 1;    // Tomas Pump PWR       // FAN2
    
    U32         Reserved : 8;    
    }bit;
    U32 dword;
}IO_Data_t;

typedef struct AppData_t
{
    U32         appRunTime;
    U32         fwVersion;
    U32         Buzzer;
    
    DA_Data_t   DA;
    AD_Data_t   AD;
    IO_Data_t   IO;
}AppData_t;

typedef enum {
  ER_NONE               = 0,
  ER_START              = 1,
  ER_EMERGENCY          = 2,
  ER_INLET_PRESS        = 3,
  ER_BLOOD_FLOW         = 4,
  ER_BUBBLE_DETECT      = 5,
  ER_LOW_BATTERY        = 6,
}AlarmList;

typedef union SystemError_t
{
    struct
    {
      U8   starton         : 1;
      U8   emeron          : 1;
      U8   inletpress      : 1;
      U8   outpress        : 1;
      U8   bloodflow       : 1;
      U8   lowbatt         : 1;
      U8   bubble          : 1;
      U8   disconnectflow  : 1; // SONOFlow disconnect error 2024.07.07
    }bit;
    
    U8 Error;

}SysError_t;

typedef struct SystemState_t
{
  U16 autorun    :1;
  U16 stop       :1;
  U16 emer       :1;
  
  U16 startswon  :1;
  U16 stopswon   :1;
  
  U16 reserved05 :1;
  U16 BuzzerOff  :1;
  U16 primingmode:1;     // mode set, alarm disable without Start error & Emergency error
  
  U16 bubblealarmstop:1; 
  
  U16 cyclerun   :1;
  U16 bfirststart:1;
  
  U16 chkinlerstart:1;  // Start to check inlet pressure error as set value is reached.
  U16 chkflowstart :1;  // Start to check blood flow error as set value is reached.
  
  U16 reserved:3; 
  
}SystemState_t;

typedef struct DeviceState_t
{
    SystemState_t  g;
    SysError_t  Err;
    
    // Cycle Run
    U8 Pump1CycState;
    U8 Pump2CycState;
    U16 Delay_P1;
    U16 Delay_P2;
    
    U16 RealBPSTime;
    U16 CalcBPS;
    
    U16 UpdateLogPeriod;
    
    // Start,Stop Button Cycle Run
    U8 StartCycState;
    U8 StartTimerCnt;
    
    U8 StopCycState;
    U8 StopTimerCnt;
    
}SysState_t;

typedef struct PumpRPM_t
{
    U16  mainpump1;
    U16  mainpump2;
    U16  subpump;
    
}PumpRPM_t;

// Upload Measured Data [Controller -> LCD]
typedef struct RealData_t
{
      U16        bpm;
      float      blood;
      float      inlet;              // Inlet Pressure [DP-102A]
      float      ventflow;
      S16        gphflow;            // Graph Data
      bool       bubbleexist;
      
      // Only Monitoring  
      float      inoutreg;
      float      mainreg;
      
}RealData_t;

// Bettery Power Status [Controller -> LCD]
typedef struct BattPower_t
{
    U8        ac;
    U8        batt1;
    U8        batt2;
}BattPower_t;

/*********************************************************************************************************************************************/
// bit0 : Bubble alarm within the last request interval, the bubble is larger as the defined limit,the measurement is continued.
// bit1 : Warning, the measurement is continued
// bit2 : not in use = 0
// bit3 : Reset flag: a restart has been performed within the last request interval
// bit4 : error during bubble detection within the last request interval, values of bubble detection and of flow measurement are not plausible
// bit5 : Error in flow measurement, the flow value is affected and not valid
// bit6 : Error in temperature measurement, Maybe compensated flow value is affected and not valid.
// bit7 : Serious device fault, measurement is halted
/*********************************************************************************************************************************************/
typedef union Sono_Flow_State_t
{
    struct
    {
        U8         bubblealarm    : 1;
        U8         warn           : 1;
        U8         notused        : 1;
        U8         resetflag      : 1;
        U8         errbubble      : 1;
        U8         errflow        : 1;
        U8         errtemp        : 1;
        U8         devicefault    : 1;
    }bit;
    U8 State;
}State_t;

typedef struct SonoFlowData_t
{
    float       flowlowval;     // ml/min every 1sec
    float       flow;           // LPM[l/min]  // average 60cnt
    float       flow_mlps;       // ul/sec
    float       bubblesize;         // size[mm] 
    
    State_t     ST;             // State[Alarm]
    
}FlowData_t;

typedef struct GlobalVariable_t
{
    float   TempLowFlow[20];      // average for 1sec : 1/50ms
    float   TempFlow[60];         // average for 1min
    float   TempFlowBufCnt;       // Buf save count
    BOOL    bFirstFullTempBuf;    // only one excute as cycle start.
    U8      FlowSenCommCnt;       // Monitor Rs485 Comm. Count[Connect or not]
}Global_t;

#pragma pack(pop)

APP_DATA_EXT AppData_t          AppData;
APP_DATA_EXT SysState_t         Sys;
APP_DATA_EXT AD_Offset_t        ADOffset;
APP_DATA_EXT PumpRPM_t          RPM;

APP_DATA_EXT RealData_t         RealData;
APP_DATA_EXT BattPower_t        PwrState;
APP_DATA_EXT FlowData_t         FlowVal;
APP_DATA_EXT Global_t           Global;

//APP_DATA_EXT float   TempLowFlow[20];      // average for 1sec : 1/50ms
//APP_DATA_EXT float   TempFlow[60];         // average for 1min
//APP_DATA_EXT float   TempFlowBufCnt;       // Buf save count
//APP_DATA_EXT BOOL    bFirstFullTempBuf;    // only one excute as cycle start.

APP_DATA_EXT U8 nControlTaskState;
APP_DATA_EXT void AppData_Init(void);


#endif
