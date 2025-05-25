#define __SERIAL_COMM_C__
    #include "SerialComm.h"
#undef  __SERIAL_COMM_C__

#include "IUART.h"
#include    <math.h>

// 2023.08.28 SJS
void Data_Parsing(U8* buf)
{
  int nCmd = buf[1];
  buf[1] = 0;
  switch(nCmd)
  {
  case DIGTAL_OUT:
    AppData.IO.dword = buf[3] + (buf[4]<<8 & 0xff00) + (buf[5]<<16 & 0x030000);
    break;
    
  case MAIN_PUMP:
    break;
    
  case SUB_PUMP1:
    AppData.DA.spump = buf[4] + (buf[5]<<8 & 0xff00);
    break;
    
    case SUB_PUMP2:
    break;
  }
}


/**************************************/
/*         2023.11.19                 */
/*   Parse packet from SONO FLOW      */
/**************************************/
// -----------------------------------------------------------------------
// GetCRC
// pS[]: Buffer of serial communication
// [0]: Start pattern ( = 0xF0 + Address)
// [1], [2]: Ct of bytes
// [2 .. Ct-2]: Data
// [Ct-1]: CRC
// -----------------------------------------------------------------------
U8 GetCRCOut(char* pS)
{
    U8 a, b, i;
    U16 Ct;
    b = pS[0]; // Start pattern
    Ct = pS[1] * 0x100 + pS[2]; // count of bytes in buffer
    for (i = 1; i <= Ct - 2; i++)
    {
        a = (U8)(b ^ pS[i]);
        if ((a & 0x80) != 0) 
          b = (U8)((a << 1) ^ 0xd4);    // 0x4d : polynomial for crc
        else 
          b = (U8)(a << 1);
    }
    return (U8)(b & 0x3f); // mask to 6 bit
}


S32 IEEE754ConvertDec(U32 Val)
{
    U32 Data = Val;

    U32 Sign = 0;
    U32 Exponential = 0;
    U32 FloatPoint = 0;

    Sign = (Data >> 31) & 1;
    Exponential = (Data >> 23) & 0xFF;
    FloatPoint = (U32)(Data & 0x007FFFFF);

    MyFloat Var;
    Var.Raw.Mantissa = FloatPoint;
    Var.Raw.Exponent = Exponential;
    Var.Raw.Sign = Sign; 
    
    S32 Ret = (S32)(Var.fRet);
    
    return Ret;
}
float IEEE754Convertfloat(U32 Val)
{
    U32 Data = Val;

    U32 Sign = 0;
    U32 Exponential = 0;
    U32 FloatPoint = 0;

    Sign = (Data >> 31) & 1;
    Exponential = (Data >> 23) & 0xFF;
    FloatPoint = (U32)(Data & 0x007FFFFF);

    MyFloat Var;
    Var.Raw.Mantissa = FloatPoint;
    Var.Raw.Exponent = Exponential;
    Var.Raw.Sign = Sign; 
    
    float Ret = (float)(Var.fRet);
    
    return Ret;
}

/****************************************************************************************/
/* Convert packet to floating Data by IEEE753 method without calculation of bubble size */
/*  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 : count=23      */
/* F1 00 17 3F 0B 00 89 82 00 00 96 43 8B 69 E0 40 48 2A 00 00 00 00 3C                 */
/****************************************************************************************/
//U8 FlowArrayIdx = 0;
void FlowBufShift(float* fwbuf, int idx)
{
    int Len = idx - 1;
    for(int i = Len; i > 0; i--)
      fwbuf[i] = fwbuf[i - 1];
}
void FlowParsePacket(char* buf)
{
    FlowData_t FlowTemp;
    U8 CRCVal = GetCRCOut(buf);
    U32 nTemp = 0;
    
    float Sum = 0;
    float FlowLowVal = 0;
    float FlowAverageVal = 0;
    
    if(buf[0] == 0xf1 && buf[22] == CRCVal)
    {
        //FlowTemp.ST.State = buf[5];
        FlowVal.ST.State = buf[5];
        
        //////////////////////////////////////////////////////////////////////////
        // blood flow average 1 sec. [l/min]
        nTemp = (U32)((buf[11] << 24) | (buf[10] << 16) | (buf[9] << 8) | buf[8]);
        FlowLowVal = (float)IEEE754Convertfloat(nTemp);          // UNIT[ml/min]
        if(FlowLowVal < 0)
          FlowLowVal = 0;
        Global.TempLowFlow[0] = FlowLowVal;
        
        for(int i = 0; i < 20; i++)
            Sum += Global.TempLowFlow[i];
        FlowAverageVal = (float)((Sum / 20000.)); // ml/min -> L/min
        
        //FlowTemp.flowlowval = FlowAverageVal;
        FlowVal.flowlowval = FlowAverageVal;
        
        FlowBufShift(Global.TempLowFlow, 20);     // buffer shift : 20 X 50ms = 1sec
        
        //////////////////////////////////////////////////////////////////////////    
        // bubble size
        nTemp = (U32)((buf[21] << 24) | (buf[20] << 16) | (buf[19] << 8) | buf[18]);
        //FlowTemp.bubblesize = (float)((float)nTemp / 10);
        FlowVal.bubblesize = (float)((float)nTemp / 10);
    }
    
    //return FlowTemp;
}

//FlowData_t FlowParsePacket(char* buf)
//{
//    FlowData_t FlowTemp;
//    U8 CRCVal = GetCRCOut(buf);
//    U32 nTemp = 0;
//    S32 nTempFlow = 0;
//    float FlowAverageVal = 0;
//    if(buf[0] == 0xf1 && buf[22] == CRCVal)
//    {
//        FlowTemp.ST.State = buf[5];
//        nTemp = (U32)((buf[11] << 24) | (buf[10] << 16) | (buf[9] << 8) | buf[8]);
//        FlowTemp.flowlowval = (float)IEEE754Convertfloat(nTemp);          // UNIT[ml/min]
//        if(FlowTemp.flowlowval < 0)
//          FlowTemp.flowlowval = 0;
//        
//        //////////////////////////////////////////////////////////////////////////
//        // blood flow average 1 min. [l/min]
//        nTempFlow = (U32)((float)(FlowTemp.flowlowval * 100));
//        nTempFlow = (U32) nTempFlow;
//        //TempFlow[0] = FlowTemp.flowlowval;
//        TempFlow[0] = (float)((float)nTempFlow / 100);  // fixed floating point 0.00
//        
//        U32 Average = 0;
//        for(int i = 0; i < 60; i++)
//            Average += TempFlow[i];
//        FlowAverageVal = (float)(((float)Average / 60000));
//        FlowTemp.flow = FlowAverageVal;
//        // buffer shift
//        FlowBufShift(TempFlow, 60);     // array = 60
//        
//        //////////////////////////////////////////////////////////////////////////    
//        // flow ul/sec
//        
//        nTemp = (U32)((buf[16] << 24) | (buf[15] << 16) | (buf[14] << 8) | buf[13]);
//        FlowTemp.flow_mlps = (U32)IEEE754ConvertDec(nTemp);                // ul/sec       
//        if(FlowTemp.flow_mlps < 0)
//          FlowTemp.flow_mlps = 0;
//        FlowTemp.flow_mlps = (U32)((float)FlowTemp.flow_mlps / 1000);      // ml/sec
//        
//        // bubble size
//        nTemp = (U32)((buf[21] << 24) | (buf[20] << 16) | (buf[19] << 8) | buf[18]);
//        FlowTemp.bubblesize = (float)((float)nTemp / 10);
//    }
//    
//    return FlowTemp;
//}

/**************************************/
/*         2023.11.19                 */
/*   Parse packet from Clear Diplay   */
/**************************************/
#define STX1    0xee
#define STX2    0xb5
#define ETX     0xff
void LcdParsePacket(char* buf)
{
    U8 Len = strlen(buf);
    
    if(buf[0] == STX1 && buf[1] == STX2)
    {
        for(int i = 0; i < Len; i++)
        {
             if(buf[i] == ETX)
             {
               PckStr.Len = (i - 2);
                break;
             }
        }
        memcpy(PckStr.str, &buf[2], sizeof(U8) * PckStr.Len);
    }
}

void SerialComm_RS485A_MsgProc(U8* buffer)
{
    U32 size;
       
    if(IUART1_CheckRecvEnd() == false)
    {
        return;
    }
    
    size = IUART1_GetRecvSize();
    
    for(int i=0; i<size; i++)
    {
        buffer[i] = IUART1_ReadByte();
    } 
        
    //FlowVal = FlowParsePacket(buffer);
    FlowParsePacket(buffer);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
void SerialComm_RS232S1_MsgProc(U8* buffer)
{
    U32 size;
    
    if(IUART2_CheckRecvEnd() == false)
    {
        return;
    }
    
    size = IUART2_GetRecvSize();
    
    for(int i=0; i<size; i++)
    {
        buffer[i] = IUART2_ReadByte();
    }

    LcdParsePacket(buffer);    
}

void SerialComm_RS232_MsgProc(U8* buffer)
{
    U32 size;
    
    if(IUART3_CheckRecvEnd() == false)
    {
        return;
    }
    
    size = IUART3_GetRecvSize();
    
    for(int i=0; i<size; i++)
    {
        buffer[i] = IUART3_ReadByte();
    }
    
    LcdParsePacket(buffer);
}

void SerialComm_RS232S2_MsgProc(U8* buffer)
{
    U32 size;
    
    if(IUART4_CheckRecvEnd() == false)
    {
        return;
    }
    
    size = IUART4_GetRecvSize();
    
    for(int i=0; i<size; i++)
    {
        buffer[i] = IUART4_ReadByte();
    }  

    LcdParsePacket(buffer);    
}

