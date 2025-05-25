#ifndef   __APP_PARAM_H__
#define   __APP_PARAM_H__

#include "project.h"
#include "II2C.h"

#ifdef __APP_PARAM_C__
	#define APP_PARAM_EXT
#else
	#define APP_PARAM_EXT extern
#endif

typedef enum
{
    BAUDRATE_9600 = 0,
    BAUDRATE_19200,
    BAUDRATE_38400,
    BAUDRATE_57600,
    BAUDRATE_115200,
    BAUDRATE_230400,
}BuadRate_t;

typedef enum
{
    PARITY_NONE = 0,
    PARITY_ODD,
    PARITY_EVEN,
}Parity_t;

typedef enum
{
    STOPBIT_1 = 0,
    STOPBIT_2,
}StopBit_t;

#pragma pack(push,1)

typedef struct RS485_t
{
    U8 baudrate;
    U8 parity;
    U8 stopbit;
    U8 rsvd;
}RS_t;

typedef struct AppParam_t
{
    U32         buzzerEnable;
    RS_t        rs232;
    RS_t        rs485A;
    RS_t        rs232S1;
    RS_t        rs232S2;    
    U32         factorySetCount;
}AppParam_t;

// Set Parameter  [LCD - > Controller]
typedef struct AlarmLimit_t
{
    U16        bloodflow;
    U16        inletpress;
}ArmLimit_t;

typedef union SetParam_t
{
    struct
    {
      U16        bpm;
      U16        ejection;
      U16        delay;
      U16        inoutpress;
      U16        mainpress;
      U16        ventflow;
      
      ArmLimit_t alarm;
    }val;
    U16 Param[8];
    
}SetParam_t;

#pragma pack(pop)

APP_PARAM_EXT AppParam_t AppParam;
APP_PARAM_EXT SetParam_t SetParam;

APP_PARAM_EXT void AppParam_Init(void);
APP_PARAM_EXT void AppParam_Save(void);
APP_PARAM_EXT void AppParam_Load(void);
APP_PARAM_EXT void AppParam_FactorySet(void);

APP_PARAM_EXT void SetParam_Save(void);
APP_PARAM_EXT void SetParam_Load(SetParam_t* p);

#endif
