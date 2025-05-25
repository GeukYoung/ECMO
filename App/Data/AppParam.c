#define __APP_PARAM_C__
    #include "AppParam.h"
#undef  __APP_PARAM_C__

#include "Version.h"

#define CONFIRM_PARAM ((U32)0xabcd0002)


#define CONFIRM_BASE_ADDRESS            (0)
#define APP_PARAM_BASE_ADDRESS          (16)
#define SET_PARAM_BASE_ADDRESS          (64)

static void SetConfirmNumber(U32 value);
static U32 GetConfirmNumber(void);

static U32 ConfirmNumber;

void AppParam_Init(void)
{
    ConfirmNumber = GetConfirmNumber();
    AppParam_Load();
    SetParam_Load(&SetParam);

    if(ConfirmNumber != CONFIRM_PARAM)
    {
        AppParam.factorySetCount = 0;
        AppParam_FactorySet();
    }
}

void AppParam_Save(void)
{
    II2C_WriteByte(APP_PARAM_BASE_ADDRESS, sizeof(AppParam_t), (U8*)&AppParam);
}

void AppParam_Load(void)
{
    II2C_ReadByte(APP_PARAM_BASE_ADDRESS, sizeof(AppParam_t), (U8*)&AppParam);
}


// Save [U16 SetParam -> U8 eeprom] 
void SetParam_Save(void)     
{
    U8* pVal = (U8*)malloc(sizeof(SetParam_t));
    for(int i = 0; i < 8; i++)
    {
      // little endian
      pVal[i * 2]        = (U8)SetParam.Param[i];
      pVal[i * 2 + 1]    = (U8)(SetParam.Param[i] >> 8 & 0xff);
    }
    
    II2C_WriteByte(SET_PARAM_BASE_ADDRESS, sizeof(SetParam_t), (U8*)pVal);
    free(pVal);
}

// Load [U8 eeprom -> U16 SetParam] 
void SetParam_Load(SetParam_t* p)     
{
    U8* pU8Val = (U8*)malloc(sizeof(SetParam_t));
    
    II2C_ReadByte(SET_PARAM_BASE_ADDRESS, sizeof(SetParam_t), (U8*)pU8Val);
    
     for(U8 i = 0; i < 8; i++)
    {
      // little endian
      p->Param[i]        = (U16)(pU8Val[i * 2] | (pU8Val[i * 2 + 1] << 8));
    }   
    free(pU8Val);
}

void AppParam_FactorySet(void)
{
    SetConfirmNumber(CONFIRM_PARAM);
    
    AppParam.buzzerEnable = 1;

    AppParam.rs232.baudrate = 4;         // BAUDRATE_115200 UART3
    AppParam.rs232.parity = 0;
    AppParam.rs232.stopbit = 0;
    
    AppParam.rs485A.baudrate = 4;        // BAUDRATE_115200 UART1
    AppParam.rs485A.parity = 0;
    AppParam.rs485A.stopbit = 0;

    AppParam.rs232S1.baudrate = 4;       // BAUDRATE_115200 UART2
    AppParam.rs232S1.parity = 0;
    AppParam.rs232S1.stopbit = 0;    
    
    AppParam.rs232S2.baudrate = 4;       // BAUDRATE_115200 UART2
    AppParam.rs232S2.parity = 0;
    AppParam.rs232S2.stopbit = 0;
    
    AppParam.factorySetCount++;
    
    AppParam_Save();
}

static U32 GetConfirmNumber(void)
{
    U32 value;
    
    II2C_ReadByte(CONFIRM_BASE_ADDRESS, 4, (U8*)&value);
    
    return value;
}

static void SetConfirmNumber(U32 value)
{
    ConfirmNumber = value;
    
    II2C_WriteByte(CONFIRM_BASE_ADDRESS, 4, (U8*)&value);
}