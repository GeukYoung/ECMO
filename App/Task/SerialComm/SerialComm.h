#ifndef   __SERIAL_COMM_H__
#define   __SERIAL_COMM_H__

#include "project.h"

#ifdef __SERIAL_COMM_C__
	#define SERIAL_COMM_EXT
#else
	#define SERIAL_COMM_EXT extern
#endif

typedef enum {
	DIGTAL_OUT = 1,
        
	MAIN_PUMP  = 13 ,
	SUB_PUMP1  = 14,
	SUB_PUMP2  = 15,
} CMD;

// Extract SetParameter String Only
typedef struct PacketDataStr_t
{
    char        str[100];
    char        Len;
}PacketStr_t;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Convert IEEE754 floating point
typedef union 
{
    float fRet;
    struct
    {
        U32 Mantissa : 23;
        U32 Exponent : 8;
        U32 Sign : 1;
    } Raw;
} MyFloat;

SERIAL_COMM_EXT void SerialComm_Init(void);
SERIAL_COMM_EXT void SerialComm_RS485A_MsgProc(U8* buffer);
//SERIAL_COMM_EXT void SerialComm_RS485B_MsgProc(U8* buffer);
SERIAL_COMM_EXT void SerialComm_RS232S1_MsgProc(U8* buffer);
SERIAL_COMM_EXT void SerialComm_RS232_MsgProc(U8* buffer);
SERIAL_COMM_EXT void SerialComm_RS232S2_MsgProc(U8* buffer);

//2023.08.28 SJS
SERIAL_COMM_EXT void Data_Parsing(U8* buf);
SERIAL_COMM_EXT void LcdParsePacket(char* buf);
SERIAL_COMM_EXT U8 GetCRCOut(char* pS);
SERIAL_COMM_EXT void FlowBufShift(float* fwbuf, int idx);

SERIAL_COMM_EXT PacketStr_t  PckStr;
SERIAL_COMM_EXT MyFloat ieee754Val;

#endif
