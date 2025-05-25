#define _SERIAL_COMM_TASK_C_
    #include "SerialCommTask.h"
#undef  _SERIAL_COMM_TASK_C_

#include "LED.h"
#include "ITIMER.h"

typedef enum {
	IDLE = 0,
        INIT = 1,
        LCD_EN = 2,
	AUTO = 3,
} State;

/**********************************/
/* Clear Display Screen Number    */
/**********************************/
#define SCR_START_ERR       0x12
#define SCR_EMER_ERR        0x0f
#define SCR_INLET_ERR       0x10
#define SCR_BLOOD_ERR       0x11

static void SerialCommTask(void const * argument);

U8 BattPowerLevel(U16 Val);
bool MatchSetParam(void);
void ConvStrToInt(char* str);
float RegCalToBar(U16 val, U16 offset);         // ITV-1030
float DP102CalToBar(U16 val, U16 offset);       // DP-102A
/*********************************************************************/
/*  Flow Sensor Request : Packet Data Set(Flow, Volume, Bubble Size) */
/*********************************************************************/
//U8 txTemp[11]= {0xF1, 0x00, 0x0B, 0x3F, 0x0B, 0x00, 0x89, 0x82, 0x8B, 0x2A, 0x08};
//U8 txTemp[11]= {0xF1, 0x00, 0x0B, 0x3F, 0x0B, 0x00, 0x89, 0x82, 0x81, 0x2A, 0x20};

U8 CommState  = MAIN_BPM;        
U8 PowerState = START_ST;        
U8 ParaState  = PARA_G_AVR_VAL;  

U8  rxTestBuffer[64];
U8  rxFlowBuffer[64];
U8  rxTestBufferS1[64];
U8  rxTestBufferS2[64];

U8 ACOn = 0x00;
U8 PwrBatt1 = 0x00;
U8 PwrBatt2 = 0x00;

void SerialCommTask_Init(void)
{     
    SerialComm_RS485A_WriteBytes  = IUART1_WriteBytes;
    SerialComm_RS232S1_WriteBytes = IUART2_WriteBytes;  // RS232 Spare1 [PC]
    SerialComm_RS232_WriteBytes   = IUART3_WriteBytes;
    SerialComm_RS232S2_WriteBytes = IUART4_WriteBytes;  // RS232 Spare2
    
    osThreadDef(SerialComm, SerialCommTask, osPriorityBelowNormal, 0, 128);
    osThreadCreate (osThread(SerialComm), NULL);  
}

/****************************************************************/
/*   SONOFLOW SENSOR DATA REQUEST [Flow, Volume, Bubble Size]   */
/****************************************************************/
U16 SeqCnt = 0;
void RequestFlowData(void)
{
    U8 txTemp[11]= {0xF1, 0x00, 0x0B, 0x3F, 0x0B, 0x00, 0x89, 0x82, 0x81, 0x2A, 0x20};
    //U8 txTemp[11]= {0xF1, 0x00, 0x0B, 0x3F, 0x0B, 0x00, 0x89, 0x82, 0x8B, 0x2A, 0x08};

    txTemp[6] = SeqCnt;
    txTemp[10] = GetCRCOut(txTemp);
    SerialComm_RS485A_WriteBytes(&txTemp[0], 11);
    SeqCnt++;
    if(SeqCnt > 255)
      SeqCnt = 0;
}

void ErrorScreen(U8 nScrNo, bool bOnOff)
{
    U8 cTemp[9] = {0xee, 0xb1, 0x00, 0x00, 0x00, 0xff, 0xfc, 0xff, 0xff};
    if(bOnOff == 1)
      cTemp[4] = nScrNo;
    else
      cTemp[4] = 0x00;
    
    SerialComm_RS232_WriteBytes(&cTemp[0], 9);
    
    // RS232 Spare1,2 Test 2023.12.10
    SerialComm_RS232S1_WriteBytes(&cTemp[0], 9);
    SerialComm_RS232S2_WriteBytes(&cTemp[0], 9);
}
/*************************************************/
/* Data만 추출된 String을 인자로 함수 호출할 것      */
/* ex) "56.0,104.0,100.0,0.8,0.9,1.1";           */
/*************************************************/
void ConvStrToInt(char* str)
{
    U8 Len = strlen(str);

    U8 idx = 0; // 6-Para Seperator
    U8 cnt = 0;
    
//    if(Len >= 28)
//      return;
    
    for(int i = 0; i < Len; i++)
    {
        if(str[i] != ',')
        {
          cnt++;
          if(i == Len - 1)
          {
             // 마지막 데이터는 구분자(,)가 없으므로 아래와 같이 처리함.
              char *pVal = (char*)malloc(sizeof(U8)*i);
              memcpy(pVal, &str[Len - cnt], sizeof(U8)*cnt);
              SetParam.val.alarm.inletpress = (U16)(atof(pVal) * 10);
              free(pVal);          
          }
        }
        else
        {
            char *pVal = (char*)malloc(sizeof(U8) * i);
            memcpy(pVal, &str[i - cnt], sizeof(U8) * cnt);
            switch(idx)
            {
                case 0:
                  SetParam.val.bpm = (U16)(atof(pVal) * 10);    // 소수점데이터를 정수형으로 변환 * 10
                  break;
                case 1:
                  SetParam.val.ejection = (U16)(atof(pVal) * 10);
                  break;
                case 2:
                  SetParam.val.delay = (U16)(atof(pVal) * 10);
                  break;   
                case 3:
                  SetParam.val.inoutpress = (U16)(atof(pVal) * 10);
                  break;
                case 4:
                  SetParam.val.mainpress = (U16)(atof(pVal) * 10);
                  break;
                 case 5:
                  SetParam.val.ventflow = (U16)(atof(pVal) * 10);
                  break;
                case 6:
                  SetParam.val.alarm.bloodflow = (U16)(atof(pVal) * 10);
                  break;             
            }
            free(pVal);
            idx++;
            cnt = 0;
        }
    } 
}
/*********************************************************************/
// Compare eeprom data to received SetParam from ClearDisplay
// if return is false, recieved SetPara save to eeprom.
/*********************************************************************/
bool MatchSetParam(void)
{
    SetParam_t sp;
    SetParam_Load(&sp);
    U8 Len = sizeof(SetParam_t);
    bool b = true;
    for(int i = 0; i < Len; i++)
    {
      if(sp.Param[i] != SetParam.Param[i])
      {
        b = false;
        break;
      }
    }
    return b;
}



#define ADC_VOLT_REF    5.0     //  Vref = 5.0V
#define ADC_RESOL      1024     // 10-bit, 8ch adc(ADC108S22CIMTX)
#define REG_MAX_MPA     0.5     //  0.5MPA : 5V
#define DP102_MAX_MPA   1.0     //  1MPA : 5V
float RegCalToBar(U16 val, U16 offset)       // ITV-1030
{
    float ret = 0;
    S16 temp = val - offset;
    if(temp < 0) temp = 0;
    
    float Vin = (temp * ADC_VOLT_REF) / ADC_RESOL;
    
    ret = (U8)((float)((Vin * REG_MAX_MPA / ADC_VOLT_REF) * 10));     // 반올림.
    
    return ret;
}

float DP102CalToBar(U16 val, U16 offset)       // DP102A 1V ~ 5V[Max. 1Mpa]
{
    float ret = 0;
    S16 temp = val - offset;
    if(temp < 0) temp = 0;
    
    float Vin = (float)(((float)temp * ADC_VOLT_REF) / ADC_RESOL);
    
    ret = (float)((float)(Vin * 10.) / (float)(ADC_VOLT_REF-1));
    
    return ret;
}


void UpdateActualValue(void)
{
    ////////////////////////////////////////
    RealData.blood = FlowVal.flow;              // [l/min]
    
    RealData.inlet       = DP102CalToBar(AppData.AD.InletPress ,ADOffset.InletPress);
    RealData.ventflow    = (float)((float)SetParam.val.ventflow / 10);

    RealData.gphflow     = (U16)(RealData.blood * 1000);        // sonoflow sensor      [ml/min]  update low-data
    RealData.bubbleexist = FlowVal.ST.bit.bubblealarm;
    
    RealData.inoutreg    = RegCalToBar(AppData.AD.inoutreg ,ADOffset.inoutreg);    // CH0
    RealData.mainreg     = RegCalToBar(AppData.AD.mainreg ,ADOffset.mainreg);       // CH1
    
    switch(CommState)
    {
//        case MAIN_G_EN:
//          { 
//              U8 cPacket[12] = {0xee, 0xb1, 0x61, 0x00, 0x00, 0x00, 0x0d, 0x01, 0xff, 0xfc, 0xff, 0xff};
//              SerialComm_RS232_WriteBytes(&cPacket[0], 12);
//              
//              // RS232 Spare1,2 Test 2023.12.10
//              SerialComm_RS232S1_WriteBytes(&cPacket[0], 12);
//              SerialComm_RS232S2_WriteBytes(&cPacket[0], 12);              
//              CommState = MAIN_BPM;
//          }
//          break;
        case MAIN_BPM:
          {
              char cTemp[3];
              sprintf(cTemp, "%d", SetParam.val.bpm/10);
              U8 cPacket[14] = {0xee, 0xb1, 0x10, 0x00, 0x00, 0x00, 0x0f, 0x31, 0x31, 0x37, 0xff, 0xfc, 0xff, 0xff};
              
              cPacket[7] = cTemp[0];
              cPacket[8] = cTemp[1];
              cPacket[9] = cTemp[2];

              SerialComm_RS232_WriteBytes(&cPacket[0], 14);
              
              // RS232 Spare1,2 Test 2023.12.10
              SerialComm_RS232S1_WriteBytes(&cPacket[0], 14);
              SerialComm_RS232S2_WriteBytes(&cPacket[0], 14);              
              CommState = MAIN_BLOOD;
          }
          break;
        case MAIN_BLOOD:
          {
              char cTemp[3];
              sprintf(cTemp, "%f", RealData.blood);
              U8 cPacket[14] = {0xee, 0xb1, 0x10, 0x00, 0x00, 0x00, 0x11, 0x30, 0x31, 0x35, 0xff, 0xfc, 0xff, 0xff};
              
              cPacket[7] = cTemp[0];
              cPacket[8] = cTemp[1];
              cPacket[9] = cTemp[2];

              SerialComm_RS232_WriteBytes(&cPacket[0], 14);   
              
              // RS232 Spare1,2 Test 2023.12.10
              SerialComm_RS232S1_WriteBytes(&cPacket[0], 14);
              SerialComm_RS232S2_WriteBytes(&cPacket[0], 14);              
              CommState = MAIN_INLET;
          }
          break;
        case MAIN_INLET:
          {
              char cTemp[3];
              sprintf(cTemp, "%f", RealData.inlet);
              U8 cPacket[14] = {0xee, 0xb1, 0x10, 0x00, 0x00, 0x00, 0x0e, 0x30, 0x31, 0x35, 0xff, 0xfc, 0xff, 0xff};
              
              cPacket[7] = cTemp[0];
              cPacket[8] = cTemp[1];
              cPacket[9] = cTemp[2];

              SerialComm_RS232_WriteBytes(&cPacket[0], 14);  
              
              // RS232 Spare1,2 Test 2023.12.10
              SerialComm_RS232S1_WriteBytes(&cPacket[0], 14);
              SerialComm_RS232S2_WriteBytes(&cPacket[0], 14);              
              CommState = MAIN_VENT;
          }
          break;
         case MAIN_VENT:
          {
              char cTemp[3];
              sprintf(cTemp, "%f", RealData.ventflow);
              U8 cPacket[14] = {0xee, 0xb1, 0x10, 0x00, 0x00, 0x00, 0x12, 0x30, 0x31, 0x35, 0xff, 0xfc, 0xff, 0xff};
              
              cPacket[7] = cTemp[0];
              cPacket[8] = cTemp[1];
              cPacket[9] = cTemp[2];

              SerialComm_RS232_WriteBytes(&cPacket[0], 14);  
              
              // RS232 Spare1, 2 Test 2023.12.10
              SerialComm_RS232S1_WriteBytes(&cPacket[0], 14);
              SerialComm_RS232S2_WriteBytes(&cPacket[0], 14);              
              CommState = MAIN_GRAPH;
          }
          break;
        case MAIN_GRAPH:
          {
//              U8 cPacket[13] = {0xee, 0xb1, 0x60, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xff, 0xfc, 0xff, 0xff};
//              cPacket[7] = (RealData.gphflow >> 8) & 0xff;
//              cPacket[8] = (RealData.gphflow & 0xff);
//              SerialComm_RS232_WriteBytes(&cPacket[0], 13);            
//              // RS232 Spare1,2 Test 2023.12.10
//              SerialComm_RS232S1_WriteBytes(&cPacket[0], 13);
//              SerialComm_RS232S2_WriteBytes(&cPacket[0], 13);   
            
                MyFloat fVal;
                U8 cPacket[15] = {0xee, 0xb1, 0x60, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfc, 0xff, 0xff};
                
                fVal.fRet = RealData.blood;
                U32 SendData = (U32)(fVal.Raw.Sign << 31 | fVal.Raw.Exponent << 23 | fVal.Raw.Mantissa);
                  
                cPacket[7] = (U8)((SendData >> 24) & 0xff);
                cPacket[8] = (U8)((SendData >> 16) & 0xff); 
                cPacket[9] = (U8)((SendData >> 8) & 0xff);
                cPacket[10] = (U8)((SendData & 0xff));
                SerialComm_RS232_WriteBytes(&cPacket[0], 15); 
                    
                // RS232 Spare1,2 Test 2023.12.10
                SerialComm_RS232S1_WriteBytes(&cPacket[0], 15);
                SerialComm_RS232S2_WriteBytes(&cPacket[0], 15);   

                CommState = MAIN_BUBBLE_CHK;
          }
          break;
      
        case MAIN_BUBBLE_CHK:
          {
                U8 cPacket[8] = {0xee, 0xb5, 0x06, 0x00, 0xff, 0xfc, 0xff, 0xff};
                if(Sys.Err.bit.bubble) // EXIST:1, NONE:0
                    cPacket[3] = 1;
                else
                    cPacket[3] = 0;
                
                SerialComm_RS232_WriteBytes(&cPacket[0], 8);
                // RS232 Spare1,2 Test 2023.12.10
                SerialComm_RS232S1_WriteBytes(&cPacket[0], 8);
                SerialComm_RS232S2_WriteBytes(&cPacket[0], 8);   
                CommState = MAIN_BPM;
          }
          break;
      
        default:
          CommState = MAIN_BPM;
          break;
    }  
}

/****************************************************************/
// 1st board : Charger indicate same as below.
// R1 = 180k[ohm], R2 = 20k[ohm], Vref = 3.3V
// ADin Volt = Vin x 20k[ohm]/(180k + 20k)[ohm]
// L0.   0% = under 19V             |           ADVal < 580
// L1.  25% = between 19V to 22V    |    580 <= ADVal < 680
// L2.  50% = between 22V to 25V    |    680 <= ADVal < 780
// L3.  75% = between 25V to 28V    |    780 <= ADVal < 880
// L4. 100% = 0ver 28V (Max.29.4V)  |    870 <= ADVal
/****************************************************************/
/****************************************************************/
// 2nd Board : Charger indicate same as below.
// R1 = 180k[ohm], R2 = 20k[ohm] Vref = 5.0V
// ADin Volt = Vin x 20k[ohm]/(180k + 20k)[ohm]
// L0.   0% = under 20V             |           ADVal < 410
// L2.  33% = between 20V to 23V    |    410 <= ADVal < 470
// L3.  66% = between 23V to 26V    |    470 <= ADVal < 530
// L4. 100% = 0ver 26V (Max.29.4V)  |    530 <= ADVal
/****************************************************************/
U8 BattPowerLevel(U16 Val)
{
    U8 PowerLevel = 0;
    
    if(Val < 410)
      PowerLevel = 0;
    else if(Val >= 410 && Val < 470)
      PowerLevel = 1;
    else if(Val >= 470 && Val < 530)
      PowerLevel = 2;  
    else if(Val >= 530)
      PowerLevel = 3; 
    else
      PowerLevel = 99;  // fault
    
    return PowerLevel;
}

/******************************************************************/
// Battery1 Status :            "Batt. No. Status"  
//                  25%    "EE B5 " + "03 00" + " FF FC FF FF";  
//                  50%    "EE B5 " + "03 01" + " FF FC FF FF";    
//                  75%    "EE B5 " + "03 02" + " FF FC FF FF";    
//                 100%    "EE B5 " + "03 03" + " FF FC FF FF";    
// Battery2 Status :            "Batt. No. Status"  
//                  25%    "EE B5 " + "04 00" + " FF FC FF FF";  
//                  50%    "EE B5 " + "04 01" + " FF FC FF FF";    
//                  75%    "EE B5 " + "04 02" + " FF FC FF FF";    
//                 100%    "EE B5 " + "04 03" + " FF FC FF FF";  
/******************************************************************/
void UpdatePowerStatus(void)
{
  
    U8 cPacket[8] = {0xee, 0xb5, 0x03, 0x01, 0xff, 0xfc, 0xff, 0xff};
    switch(PowerState)
    {
        case START_ST:
          {
            PowerState = AC_ST;
          }
          break;
          
        case AC_ST:
          {
              PwrState.ac = Smps_On();

                      // AC Line                 Remained Power
              cPacket[2] = 0x02;          cPacket[3] = PwrState.ac; // ON:1, OFF:0
              SerialComm_RS232_WriteBytes(&cPacket[0], 8);
              
              // RS232 Spare1,2 Test 2023.12.10
              SerialComm_RS232S1_WriteBytes(&cPacket[0], 8);
              SerialComm_RS232S2_WriteBytes(&cPacket[0], 8);              
              PowerState = BATT1_ST;
          }
          break;
        
        case BATT1_ST:
          {
              PwrState.batt1 = BattPowerLevel(AppData.AD.BatteryMonitor[0]);
              // Battery1                 Remained Power
              cPacket[2] = 0x03;          cPacket[3] = PwrState.batt1;
              SerialComm_RS232_WriteBytes(&cPacket[0], 8); 
              
              // RS232 Spare1,2 Test 2023.12.10
              SerialComm_RS232S1_WriteBytes(&cPacket[0], 8);
              SerialComm_RS232S2_WriteBytes(&cPacket[0], 8);             
              PowerState = BATT2_ST;
          }
          break;
          
        case BATT2_ST:
          {
              PwrState.batt2 = BattPowerLevel(AppData.AD.BatteryMonitor[1]);
               // Battery2                 Remained Power
              cPacket[2] = 0x04;          cPacket[3] = PwrState.batt2;
              SerialComm_RS232_WriteBytes(&cPacket[0], 8);  
              
              // RS232 Spare1,2 Test 2023.12.10
              SerialComm_RS232S1_WriteBytes(&cPacket[0], 8);
              SerialComm_RS232S2_WriteBytes(&cPacket[0], 8);             
              PowerState = END_ST;
          }
          break;
        
        case END_ST:
          {
            PowerState = START_ST;
          }
          break;
          
        default:
          PowerState = START_ST;
          break;
    }
}

bool bShowStartScr = false;
bool bShowEmerScr  = false;
bool bShowInletScr = false;
bool bShowBloodScr = false;
void CheckErrorScreen(void)     // scan time [every 200ms]
{
      // Start On Error Screen
      if(Sys.Err.bit.starton)
      {
          ErrorScreen(SCR_START_ERR, true);
          bShowStartScr = true;
          return;
      }
      else
      {
          if(bShowStartScr == true)
          {
            bShowStartScr = false;
            ErrorScreen(SCR_START_ERR, false);
          }
      }
      // Emergency Error Screen
      if(Sys.Err.bit.emeron)
      {
          bShowEmerScr = true;
          ErrorScreen(SCR_EMER_ERR, true);
          return;
      }
      else
      {
          if(bShowEmerScr == true)
          {
              bShowEmerScr = false;
              ErrorScreen(SCR_EMER_ERR, false);
          }
      }
      
      if(Sys.g.cyclerun || !Sys.g.stop)
      {
          // Inlet Pressure Error Screen
           if(Sys.Err.bit.inletpress)
          {
              bShowInletScr = true;
              ErrorScreen(SCR_INLET_ERR, true);
              return;
          }
          else
          {
             if(bShowInletScr == true)
            {
                bShowInletScr = false;
                ErrorScreen(SCR_INLET_ERR, false);
            }             
          }
          // Blood Flow Error Screen
            if(Sys.Err.bit.bloodflow)
          {
              bShowBloodScr = true;
              ErrorScreen(SCR_BLOOD_ERR, true);
              return;
          }
          else
          {
             if(bShowBloodScr == true)
            {
                bShowBloodScr = false;
                ErrorScreen(SCR_BLOOD_ERR, false);
            }           
          }
      }
      else
      {
          if(bShowInletScr == true)
          {
              bShowInletScr = false;
              ErrorScreen(SCR_INLET_ERR, false);
          }  
          if(bShowBloodScr == true)
          {
              bShowBloodScr = false;
              ErrorScreen(SCR_BLOOD_ERR, false);
          }
      }
}
U16 LogPeriodCnt = 0;
void UpdateLog(U16 Period)      // this loop excute every 1sec
{
    LogPeriodCnt++;
    if(LogPeriodCnt >= Period)
    {
        // "EE B5 05 00 xx xx FF FC FF FF"        high  Low  bubble error
        U8 cPacket[12] = {0xee, 0xb5, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfc, 0xff, 0xff};
        S16 TempFlowVal = (S16)(RealData.blood  * 1000);   // ml/min

        cPacket[4] = (U8)(TempFlowVal >> 8 & 0xff);      // high Byte
        cPacket[5] = (U8)(TempFlowVal & 0xff);           // Low Byte
        cPacket[6] = Sys.Err.bit.bubble; 
           
        // display error name 
        if(Sys.Err.bit.starton)
          cPacket[7] = ER_START;
        else if(Sys.Err.bit.emeron)
          cPacket[7] = ER_EMERGENCY;
        else if(Sys.Err.bit.inletpress)
          cPacket[7] = ER_INLET_PRESS;
        else if(Sys.Err.bit.bloodflow)
          cPacket[7] = ER_BLOOD_FLOW;
        else if(Sys.Err.bit.bubble)
          cPacket[7] = ER_BUBBLE_DETECT;
        else if(Sys.Err.bit.lowbatt)
          cPacket[7] = ER_LOW_BATTERY;
        else
          cPacket[7] = ER_NONE;
          
        SerialComm_RS232_WriteBytes(&cPacket[0], 12); 
        
        // RS232 Spare1,2 Test 2023.12.10
        SerialComm_RS232S1_WriteBytes(&cPacket[0], 12);
        SerialComm_RS232S2_WriteBytes(&cPacket[0], 12);  
        LogPeriodCnt = 0;
    }
}

void UpdateParameterGraph(void)
{
    switch(ParaState)
    {
       
      case PARA_G_AVR_VAL:
        {
          U8 cPacket[15] = {0xee, 0xb1, 0x60, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfc, 0xff, 0xff};
        
          ieee754Val.fRet = RealData.blood;
          U32 SendData = (U32)(ieee754Val.Raw.Sign << 31 | ieee754Val.Raw.Exponent << 23 | ieee754Val.Raw.Mantissa);
            
          cPacket[7] = (U8)((SendData >> 24) & 0xff);
          cPacket[8] = (U8)((SendData >> 16) & 0xff); 
          cPacket[9] = (U8)((SendData >> 8) & 0xff);
          cPacket[10] = (U8)((SendData & 0xff));
          SerialComm_RS232_WriteBytes(&cPacket[0], 15); 
              
          // RS232 Spare1,2 Test 2023.12.10
          SerialComm_RS232S1_WriteBytes(&cPacket[0], 15);
          SerialComm_RS232S2_WriteBytes(&cPacket[0], 15);   
          ParaState = PARA_G_AVR_TEXT;
        }
        break;
        
      case PARA_G_AVR_TEXT:
        {
          char cTemp[3];
          sprintf(cTemp, "%f", RealData.blood);
          U8 cPacket[14] = {0xee, 0xb1, 0x10, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0xff, 0xfc, 0xff, 0xff};
          //U8 cPacket[14] = {0xee, 0xb1, 0x10, 0x00, 0x00, 0x00, 0x12, 0x30, 0x31, 0x35, 0xff, 0xfc, 0xff, 0xff};
          cPacket[7] = cTemp[0];
          cPacket[8] = cTemp[1];
          cPacket[9] = cTemp[2];

          SerialComm_RS232_WriteBytes(&cPacket[0], 14);  
          
          // RS232 Spare1, 2 Test 2023.12.10
          SerialComm_RS232S1_WriteBytes(&cPacket[0], 14);
          SerialComm_RS232S2_WriteBytes(&cPacket[0], 14);            
                    
          ParaState = PARA_G_LOW_D_VAL;
        }
        break;
        
      case PARA_G_LOW_D_VAL:
        {
          U8 cPacket[15] = {0xee, 0xb1, 0x60, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfc, 0xff, 0xff};
          
          ieee754Val.fRet = (float)(FlowVal.flowlowval); // 단위는 L/S이지만 l/min lowdata로 데이터전송(강기훈연구원)
          U32 SendData = (U32)(ieee754Val.Raw.Sign << 31 | ieee754Val.Raw.Exponent << 23 | ieee754Val.Raw.Mantissa);
            
          cPacket[7] = (U8)((SendData >> 24) & 0xff);
          cPacket[8] = (U8)((SendData >> 16) & 0xff); 
          cPacket[9] = (U8)((SendData >> 8) & 0xff);
          cPacket[10] = (U8)((SendData & 0xff));
 
          SerialComm_RS232_WriteBytes(&cPacket[0], 15); 
              
          // RS232 Spare1,2 Test 2023.12.10
          SerialComm_RS232S1_WriteBytes(&cPacket[0], 15);
          SerialComm_RS232S2_WriteBytes(&cPacket[0], 15);   
          ParaState = PARA_G_LOW_D_TEXT;
        }
        break;
        
      case PARA_G_LOW_D_TEXT:
        {
          char cTemp[3];
          float fTemp = (float)(FlowVal.flowlowval);
          sprintf(cTemp, "%f", fTemp);
          U8 cPacket[14] = {0xee, 0xb1, 0x10, 0x00, 0x04, 0x00, 0x0d, 0x00, 0x00, 0x00, 0xff, 0xfc, 0xff, 0xff};
          
          cPacket[7] = cTemp[0];
          cPacket[8] = cTemp[1];
          cPacket[9] = cTemp[2];

          SerialComm_RS232_WriteBytes(&cPacket[0], 14);  
          
          // RS232 Spare1, 2 Test 2023.12.10
          SerialComm_RS232S1_WriteBytes(&cPacket[0], 14);
          SerialComm_RS232S2_WriteBytes(&cPacket[0], 14);            
                    
          ParaState = PARA_G_AVR_VAL;       
        }
        break;
    }
}

void GraphEnable(int nDelay)
{
    switch(nDelay)
    {
      case 100:
        {
            U8 cPacket[12] = {0xee, 0xb1, 0x61, 0x00, 0x00, 0x00, 0x0d, 0x01, 0xff, 0xfc, 0xff, 0xff};
            SerialComm_RS232_WriteBytes(&cPacket[0], 12);
            
            // RS232 Spare1,2 Test 2023.12.10
            SerialComm_RS232S1_WriteBytes(&cPacket[0], 12);
            SerialComm_RS232S2_WriteBytes(&cPacket[0], 12);     
        }
        break;  

    case 200:
      {
          U8 cPacket[12] = {0xee, 0xb1, 0x61, 0x00, 0x04, 0x00, 0x05, 0x01, 0xff, 0xfc, 0xff, 0xff};
          SerialComm_RS232_WriteBytes(&cPacket[0], 12);
          
          // RS232 Spare1,2 Test 2023.12.10
          SerialComm_RS232S1_WriteBytes(&cPacket[0], 12);
          SerialComm_RS232S2_WriteBytes(&cPacket[0], 12);         
      }
      break;


    case 300:
      {
           U8 cPacket[12] = {0xee, 0xb1, 0x61, 0x00, 0x04, 0x00, 0x06, 0x01, 0xff, 0xfc, 0xff, 0xff};
          SerialComm_RS232_WriteBytes(&cPacket[0], 12);
          
          // RS232 Spare1,2 Test 2023.12.10
          SerialComm_RS232S1_WriteBytes(&cPacket[0], 12);
          SerialComm_RS232S2_WriteBytes(&cPacket[0], 12);         
      }
      break;
    }
  
}
// Flow Data Average 60cnt = 1min
float Sum = 0;
float FlowAverageVal = 0;
void FlowAvrData(void)
{
    //////////////////////////////////////////////////////////////////////////
    // blood flow average 1 min. [l/min]
    Global.TempFlow[0] = FlowVal.flowlowval;
    
    for(int i = 0; i < 60; i++)
        Sum += Global.TempFlow[i];
    FlowAverageVal = (float)(((float)Sum / 60));
    FlowVal.flow = FlowAverageVal;
    FlowAverageVal = 0;
    Sum = 0;
    // buffer shift
    FlowBufShift(Global.TempFlow, 60);     // array = 60
    
    //////////////////////////////////////////////////////////////////////////    


}
static void SerialCommTask(void const * argument)
{   
    U32 UartTick = 0;
    U32 SystemTick = 0;
    U32 OneSecTick = 0;
    U32 ParaScrTick = 0;
    U32 CmdFlowTick = 0;
    
    U8 nCnt50ms = 0;
    U8 nState = IDLE;
    U32 nDelay = 0;
    PckStr.Len = 0;
    
    Sys.UpdateLogPeriod = 1;    // update log every 1sec[default]
    
    UartTick   = HAL_GetTick();
    SystemTick = HAL_GetTick();
    OneSecTick = HAL_GetTick();
    ParaScrTick= HAL_GetTick();
    CmdFlowTick= HAL_GetTick();
    
    while(1)
    {
        switch(nState)
        {
           case IDLE:
           {
             nDelay++;
             if(nDelay > 5000)
             {
                nDelay = 0;
                nState = INIT;
             }              
           }
           break;
           
           case INIT:
           {
                nDelay++;
               if(nDelay > 3000)
               {
                   // Init Data
                  nDelay = 0;
                  RealData.bpm = 0;
                  RealData.blood = 0;
                  RealData.inlet = 0;
                  RealData.ventflow = 0;
                  RealData.gphflow = 0;
                  nState = LCD_EN;
               }
               
           }
           break;
             
        case LCD_EN:
          {
            if(nControlTaskState == AUTO)
            {
              nDelay++;
              if(nDelay > 500)
              {
                nState = AUTO;
              }
              GraphEnable(nDelay);
            }
          }
          break;
          
           case AUTO:
           {
              if(HAL_GetTick() - UartTick > 20)
              {
                  UartTick = HAL_GetTick();
                  // SONOFLOW Sensor
                  SerialComm_RS485A_MsgProc(&rxFlowBuffer[0]); 
                  
                  SerialComm_RS232_MsgProc(&rxTestBuffer[0]);
                  SerialComm_RS232S1_MsgProc(&rxTestBufferS1[0]);       // RS232 Spare1 
                  SerialComm_RS232S2_MsgProc(&rxTestBufferS2[0]);       // RS232 Spare2 
                  if(PckStr.Len > 0)    // from lcd
                  {
                      if((PckStr.str[0] == '3' || PckStr.str[0] == '4' || PckStr.str[0] == '5'|| PckStr.str[0] == '6') && PckStr.str[1] == ',')
                      {
                          // Mute(Buzzer) On/Off
                          if(PckStr.str[0] == '3' && PckStr.str[2] == '1')
                            Sys.g.BuzzerOff = true;
                          else if(PckStr.str[0] == '3' && PckStr.str[2] == '0')
                            Sys.g.BuzzerOff = false;
                          
                          // Priming Mode On/Off
                          if(PckStr.str[0] == '4' && PckStr.str[2] == '1')
                            Sys.g.primingmode = true;
                          else if(PckStr.str[0] == '4' && PckStr.str[2] == '0')
                            Sys.g.primingmode = false;
                          
                          // bubble alarm stop On/Off
                          if(PckStr.str[0] == '6' && PckStr.str[2] == '1')
                            Sys.g.bubblealarmstop = true;
                          else if(PckStr.str[0] == '6' && PckStr.str[2] == '0')
                          {
                            Sys.g.bubblealarmstop = false;
                            Sys.Err.bit.bubble = false;
                          }
                          
                          // Log Period 
                          if(PckStr.str[0] == '5' && PckStr.str[2] == '0')
                            Sys.UpdateLogPeriod = 1;
                          else if(PckStr.str[0] == '5' && PckStr.str[2] == '1' && PckStr.str[3] == '.') 
                            Sys.UpdateLogPeriod = 10;
                          else if(PckStr.str[0] == '5' && PckStr.str[2] == '2' && PckStr.str[3] == '.') 
                            Sys.UpdateLogPeriod = 60;
                          else if(PckStr.str[0] == '5' && PckStr.str[2] == '3' && PckStr.str[3] == '.') 
                            Sys.UpdateLogPeriod = 300;
                          else if(PckStr.str[0] == '5' && PckStr.str[2] == '4' && PckStr.str[3] == '.') 
                            Sys.UpdateLogPeriod = 600;
                          else if(PckStr.str[0] == '5' && PckStr.str[2] == '5' && PckStr.str[3] == '.') 
                            Sys.UpdateLogPeriod = 1800;
                          else if(PckStr.str[0] == '5' && PckStr.str[2] == '6' && PckStr.str[3] == '.') 
                            Sys.UpdateLogPeriod = 3600;
                      }
                      else
                      {
                          if(PckStr.Len >= 28)
                          {
                            ConvStrToInt(PckStr.str);

                            // Alarm limit is saved to eeprom 
                            if(!MatchSetParam()) 
                              SetParam_Save();
                          }
                      }
                      
                      PckStr.Len = 0;
                      memset(PckStr.str, 0, sizeof(U8) * 100);
                  }
                      
              }
              
              
              if(HAL_GetTick() - SystemTick > 167)
              {
                  SystemTick = HAL_GetTick();

                  UpdateActualValue();
                  CheckErrorScreen();
                  UpdatePowerStatus(); // Send Power Status[Batter1,3, AC]
              }   
              
              // Request SONO_FLOW sensor Value[Command Send]
              if(HAL_GetTick() - CmdFlowTick > 50)
              {
                  CmdFlowTick = HAL_GetTick();
                  nCnt50ms++;
                  if(nCnt50ms > 20)     // every 1sec
                  {
                    FlowAvrData();      // avrage for 1min
                    nCnt50ms = 0;
                  }
                  RequestFlowData();
              }
              
              // flow senor is measured every 1sec, update data for everage 1min.
              if(HAL_GetTick() - OneSecTick > 1000)
              {
                  OneSecTick = HAL_GetTick();
                  
                  
                  if(Sys.g.startswon)   // Start button On 동안 Log 전송    2024.02.05
                    UpdateLog(Sys.UpdateLogPeriod);
              }
              
              if(HAL_GetTick() - ParaScrTick > 250)
              {
                  ParaScrTick = HAL_GetTick();
                  UpdateParameterGraph();
              }
           }
           break;           
        }     
        vTaskDelay(1);
    }
}
