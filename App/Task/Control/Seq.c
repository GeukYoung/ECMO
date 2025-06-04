#define _SEQ_C_
    #include "Seq.h"
#undef  _SEQ_C_

// Sequence debugging variable [non-active solenoid]
//bool bDebugMode = false;

void ADDAInit(void)
{
    Cmd_DAC8568(INTENAL_REF_ON);
    
    AppData.AD.inoutreg = 0;
    AppData.AD.mainreg =0;
    AppData.AD.InletPress = 0;
    AppData.AD.in[0] = 0;
    AppData.AD.in[1] = 0;
    AppData.AD.in[2] = 0;
    AppData.AD.BatteryMonitor[0] = 0;
    AppData.AD.BatteryMonitor[1] = 0;
}

// DA8568[8Ch]
void DAUpdate(void)
{
    DAC8568_Out(DAC_CH0, AppData.DA.mainreg);
    DAC8568_Out(DAC_CH1, AppData.DA.inoutreg);
    
    DAC8568_Out(DAC_CH2, AppData.DA.mpump[0]);
    DAC8568_Out(DAC_CH3, AppData.DA.mpump[1]);
    DAC8568_Out(DAC_CH4, AppData.DA.spump);
    
    DAC8568_Out(DAC_CH5, AppData.DA.spare[0]);
    DAC8568_Out(DAC_CH6, AppData.DA.spare[1]);
    DAC8568_Out(DAC_CH7, AppData.DA.spare[2]);  
}

void ADUpdate(void)
{
    //AppData.AD : 0(0V)~1023(3.3V)
    AppData.AD.mainreg     = ADC108S022_GetData(ADC_CH0);
    AppData.AD.inoutreg    = ADC108S022_GetData(ADC_CH1);
    AppData.AD.InletPress  = ADC108S022_GetData(ADC_CH2);
    
    AppData.AD.in[0]       = ADC108S022_GetData(ADC_CH3);
    AppData.AD.in[1]       = ADC108S022_GetData(ADC_CH4);
    AppData.AD.in[2]       = ADC108S022_GetData(ADC_CH5);
    
    AppData.AD.BatteryMonitor[0] = ADC108S022_GetData(ADC_CH6);
    AppData.AD.BatteryMonitor[1] = ADC108S022_GetData(ADC_CH7);
}

// Set pressure to regulators
#define REGULATOR_MAX_MPA  0.5
#define REGULATOR_MAX_VOLT  10 
#define DAC8568_RESOL       65535   // 16bit
void SetRegPressure(U8 no, U8 val)
{
    float cmd = (float)((float)val / 100);      // SetParam.pressure/10[bar] : bar/10[mpa]
    float Vo = (cmd * REGULATOR_MAX_VOLT) / REGULATOR_MAX_MPA;
    
    if(no == MAIN_REG)
      AppData.DA.mainreg = (U16)(Vo * DAC8568_RESOL / REGULATOR_MAX_VOLT);
    else if(no == IN_OUT_REG)
      AppData.DA.inoutreg = (U16)(Vo * DAC8568_RESOL / REGULATOR_MAX_VOLT);
}

// Set Vacuum Pump[Main x 2, sub pump x 1]
// 0.5V : 1LPM
#define PUMP_MAX_VOLT   5
#define VOLT_PER_LPM    1
#define MAIN_VOLT_PER_LPM    0.45                //  5V / 11[LPM]
#define SUB_VOLT_PER_LPM     0.91                //  5V / 5.5[LPM]
void SetVentFlow(U8 no, U16 val)
{
    float cmd = (float)((float)val / 10);
    float Vo = 0;
    
    if(no == MAIN_PUMP1)
    {
        Vo = (cmd * MAIN_VOLT_PER_LPM);
        AppData.DA.mpump[0]       = (U16)(Vo * DAC8568_RESOL / PUMP_MAX_VOLT);
        //AppData.DA.mpump[0] = 65535;
    }
    else if(no == MAIN_PUMP2)
    {
        Vo = (cmd * MAIN_VOLT_PER_LPM);
        AppData.DA.mpump[1]       = (U16)(Vo * DAC8568_RESOL / PUMP_MAX_VOLT);
        //AppData.DA.mpump[1] = 65535;
    }
    else if(no == SUB_PUMP)
    {
        Vo = (cmd * SUB_VOLT_PER_LPM);
        AppData.DA.spump          = (U16)(Vo * DAC8568_RESOL / PUMP_MAX_VOLT);
    }
}


U32 BattbuzOnTime = 0;
U8 BattBuzOnCount = 0;
U16 LowBattBuzPeriod = 0;
void ReadWrite(void)
{
    bool bLowBatt = Sys.Err.bit.lowbatt;
    bool bOn = Sys.Err.bit.starton || Sys.Err.bit.bloodflow || Sys.Err.bit.bubble || 
               Sys.Err.bit.inletpress || Sys.Err.bit.outpress || Sys.Err.bit.emeron ||
               Sys.Err.bit.disconnectflow;      // 2024.07.07

    if(bLowBatt)        // 5cnt Buz-on every 1min
    {
        if(BattBuzOnCount < 5)
        {
          BattBuzOnOff(true); 
        }
        else
        {
          LowBattBuzPeriod++;
          if(LowBattBuzPeriod >= 30000) 
          {
             BattBuzOnCount = 0;
             LowBattBuzPeriod = 0;
          }
          BattBuzOnOff(false);          
        }
    }
    else
    {
      if(nControlTaskState == 3)        // AUTO
        BuzzerOnOff(bOn);          
    } 
    
    DAUpdate();
    ADUpdate();  
    // IO Update
    MCP23S08_SetOut(AppData.IO.dword);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pump1 Cycle Step[1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7] -> Pump2 Cycle Step[1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P1_Step1(void)
{
    // IN
  AppData.IO.bit.P1InA     = true;
  AppData.IO.bit.P1InP     = true;
  // OUT
  AppData.IO.bit.P1OutA    = false;
  AppData.IO.bit.P1OutP    = true;
  // MAIN
  AppData.IO.bit.P1SacA1   = false;
  AppData.IO.bit.P1SacA2   = false;
  AppData.IO.bit.P1SacP1   = false;
  AppData.IO.bit.P1SacP2   = false;
  AppData.IO.bit.P1SacP3   = false;
}

void P1_Step2(void)
{
  // IN
  AppData.IO.bit.P1InA     = true;
  AppData.IO.bit.P1InP     = true;
  // OUT
  AppData.IO.bit.P1OutA    = false;
  AppData.IO.bit.P1OutP    = true;
  // MAIN
  AppData.IO.bit.P1SacA1   = true;
  AppData.IO.bit.P1SacA2   = true;
  AppData.IO.bit.P1SacP1   = true;
  AppData.IO.bit.P1SacP2   = true;
  AppData.IO.bit.P1SacP3   = true;   
}

void P1_Step3(void)
{
  // IN
  AppData.IO.bit.P1InA     = true;
  AppData.IO.bit.P1InP     = true;
  // OUT
  AppData.IO.bit.P1OutA    = false;
  AppData.IO.bit.P1OutP    = true;
  // MAIN
  AppData.IO.bit.P1SacA1   = false;
  AppData.IO.bit.P1SacA2   = false;
  AppData.IO.bit.P1SacP1   = true;
  AppData.IO.bit.P1SacP2   = true;
  AppData.IO.bit.P1SacP3   = true;   
}

void P1_Step4(void)
{
  // IN
  AppData.IO.bit.P1InA     = false;
  AppData.IO.bit.P1InP     = true;
  // OUT
  AppData.IO.bit.P1OutA    = false;
  AppData.IO.bit.P1OutP    = false;
  // MAIN
  AppData.IO.bit.P1SacA1   = false;
  AppData.IO.bit.P1SacA2   = false;
  AppData.IO.bit.P1SacP1   = true;
  AppData.IO.bit.P1SacP2   = true;
  AppData.IO.bit.P1SacP3   = true;   
}

void P1_Step5(void)
{
  // IN
  AppData.IO.bit.P1InA     = false;
  AppData.IO.bit.P1InP     = true;
  // OUT
  AppData.IO.bit.P1OutA    = true;
  AppData.IO.bit.P1OutP    = false;
  // MAIN
  AppData.IO.bit.P1SacA1   = false;
  AppData.IO.bit.P1SacA2   = false;
  AppData.IO.bit.P1SacP1   = true;
  AppData.IO.bit.P1SacP2   = true;
  AppData.IO.bit.P1SacP3   = true;   
}

void P1_Step6(void)
{
  // IN
  AppData.IO.bit.P1InA     = false;
  AppData.IO.bit.P1InP     = true;
  // OUT
  AppData.IO.bit.P1OutA    = true;
  AppData.IO.bit.P1OutP    = false;
  // MAIN
  AppData.IO.bit.P1SacA1   = false;
  AppData.IO.bit.P1SacA2   = false;
  AppData.IO.bit.P1SacP1   = false;
  AppData.IO.bit.P1SacP2   = false;
  AppData.IO.bit.P1SacP3   = false;  
}

void P1_Step7(void)
{
  // IN
  AppData.IO.bit.P1InA     = false;
  AppData.IO.bit.P1InP     = false;
  // OUT
  AppData.IO.bit.P1OutA    = false;
  AppData.IO.bit.P1OutP    = false;
  // MAIN
  AppData.IO.bit.P1SacA1   = false;
  AppData.IO.bit.P1SacA2   = false;
  AppData.IO.bit.P1SacP1   = false;
  AppData.IO.bit.P1SacP2   = false;
  AppData.IO.bit.P1SacP3   = false;  
}

void P2_Step1(void)
{
  // IN
  AppData.IO.bit.P2InA     = true;
  AppData.IO.bit.P2InP     = true;
  // OUT
  AppData.IO.bit.P2OutA    = false;
  AppData.IO.bit.P2OutP    = true;
  // MAIN
  AppData.IO.bit.P2SacA1   = false;
  AppData.IO.bit.P2SacA2   = false;
  AppData.IO.bit.P2SacP1   = false;
  AppData.IO.bit.P2SacP2   = false;
  AppData.IO.bit.P2SacP3   = false;
}

void P2_Step2(void)
{
  // IN
  AppData.IO.bit.P2InA     = true;
  AppData.IO.bit.P2InP     = true;
  // OUT
  AppData.IO.bit.P2OutA    = false;
  AppData.IO.bit.P2OutP    = true;
  // MAIN
  AppData.IO.bit.P2SacA1   = true;
  AppData.IO.bit.P2SacA2   = true;
  AppData.IO.bit.P2SacP1   = true;
  AppData.IO.bit.P2SacP2   = true;
  AppData.IO.bit.P2SacP3   = true;   
}

void P2_Step3(void)
{
  // IN
  AppData.IO.bit.P2InA     = true;
  AppData.IO.bit.P2InP     = true;
  // OUT
  AppData.IO.bit.P2OutA    = false;
  AppData.IO.bit.P2OutP    = true;
  // MAIN
  AppData.IO.bit.P2SacA1   = false;
  AppData.IO.bit.P2SacA2   = false;
  AppData.IO.bit.P2SacP1   = true;
  AppData.IO.bit.P2SacP2   = true;
  AppData.IO.bit.P2SacP3   = true;   
}

void P2_Step4(void)
{
  // IN
  AppData.IO.bit.P2InA     = false;
  AppData.IO.bit.P2InP     = true;
  // OUT
  AppData.IO.bit.P2OutA    = false;
  AppData.IO.bit.P2OutP    = false;
  // MAIN
  AppData.IO.bit.P2SacA1   = false;
  AppData.IO.bit.P2SacA2   = false;
  AppData.IO.bit.P2SacP1   = true;
  AppData.IO.bit.P2SacP2   = true;
  AppData.IO.bit.P2SacP3   = true;   
}

void P2_Step5(void)
{
  // IN
  AppData.IO.bit.P2InA     = false;
  AppData.IO.bit.P2InP     = true;
  // OUT
  AppData.IO.bit.P2OutA    = true;
  AppData.IO.bit.P2OutP    = false;
  // MAIN
  AppData.IO.bit.P2SacA1   = false;
  AppData.IO.bit.P2SacA2   = false;
  AppData.IO.bit.P2SacP1   = true;
  AppData.IO.bit.P2SacP2   = true;
  AppData.IO.bit.P2SacP3   = true;   
}

void P2_Step6(void)
{
  // IN
  AppData.IO.bit.P2InA     = false;
  AppData.IO.bit.P2InP     = true;
  // OUT
  AppData.IO.bit.P2OutA    = true;
  AppData.IO.bit.P2OutP    = false;
  // MAIN
  AppData.IO.bit.P2SacA1   = false;
  AppData.IO.bit.P2SacA2   = false;
  AppData.IO.bit.P2SacP1   = false;
  AppData.IO.bit.P2SacP2   = false;
  AppData.IO.bit.P2SacP3   = false;  
}

void P2_Step7(void)
{
  // IN
  AppData.IO.bit.P2InA     = false;
  AppData.IO.bit.P2InP     = false;
  // OUT
  AppData.IO.bit.P2OutA    = false;
  AppData.IO.bit.P2OutP    = false;
  // MAIN
  AppData.IO.bit.P2SacA1   = false;
  AppData.IO.bit.P2SacA2   = false;
  AppData.IO.bit.P2SacP1   = false;
  AppData.IO.bit.P2SacP2   = false;
  AppData.IO.bit.P2SacP3   = false;  
}

#define SCAN_TIME       2       // 2ms
U32 PumpCycleTime = 0;
U32 PumpTick = 0;
U16 PumpPhase = 0;

void PumpCycle(void)
{
    Sys.RealBPSTime++;  
    PumpPhase = Sys.CalcBPS / 2;

    if(Sys.g.bfirststart)
      return;
    
    if(Global.bFirstFullTempBuf == false)
    {
        Global.TempFlowBufCnt++;  
        if(Global.TempFlowBufCnt > 8)
        {
            Global.bFirstFullTempBuf = true;
            Global.TempFlowBufCnt = 0;
            
            for(int i = 0; i < 40; i++)
                Global.TempFlow[i] = FlowVal.flowlowval;
        }
    }

    if(Sys.RealBPSTime == 1) // Pump1 Cycle Start
    {
      Sys.Pump1CycState = P1_CYC_STEP1;  
    }

    if(Sys.RealBPSTime == PumpPhase) // Pump2 Cycle Start
    {
      Sys.Pump2CycState = P2_CYC_STEP1;
    }

    switch(Sys.Pump1CycState)
    {
      case P1_CYC_STEP1:
          {
            P1_Step1();

            if(Sys.Delay_P1++ >= 5)      // scan time = 2ms * 5cnt = 10ms
            {
                PumpCycleTime = HAL_GetTick() - PumpTick;
                PumpTick = HAL_GetTick();              
                Sys.Delay_P1 = 0;
                Sys.Pump1CycState = P1_CYC_STEP2;
            }
          }
        break;

      case P1_CYC_STEP2:
          {
              P1_Step2();
              
              U16 EjectTime = (U16)(SetParam.val.ejection / (SCAN_TIME * 10));   // eeprom에 x10 정수형으로 저장되어있음.,
              if(Sys.Delay_P1++ >= EjectTime)
              {
                  Sys.Delay_P1 = 0;
                  Sys.Pump1CycState = P1_CYC_STEP3;
              }
          }
        break;

      case P1_CYC_STEP3:
          {
              P1_Step3();

              U16 DelayTime = (U16)(SetParam.val.delay / (SCAN_TIME * 10));       // eeprom에 x10 정수형으로 저장되어있음.,
              if(Sys.Delay_P1++ >= DelayTime)
              {
                  Sys.Delay_P1 = 0;
                  Sys.Pump1CycState = P1_CYC_STEP4;
              }
          }
        break;

      case P1_CYC_STEP4:
          {
              P1_Step4();

              if(Sys.Delay_P1++ >= 5)     // hold for 10ms
              {
                  Sys.Delay_P1 = 0;
                  Sys.Pump1CycState = P1_CYC_STEP5;
              }
          }
        break;

      case P1_CYC_STEP5:
          {
              P1_Step5();

              if(Sys.Delay_P1++ >= 5)     // hold for 10ms
              {
                  Sys.Delay_P1 = 0;
                  Sys.Pump1CycState = P1_CYC_STEP6;
              }
          }
        break;

      case P1_CYC_STEP6:
          {
              P1_Step6();

              if(Sys.Delay_P1++ >= 15)     // hold for 30ms
              {
                  Sys.Delay_P1 = 0;
                  Sys.Pump1CycState = P1_CYC_STEP7;
              }
          }
        break;

      case P1_CYC_STEP7:
          {
              P1_Step7();

              Sys.Delay_P1 = 0;
              Sys.Pump1CycState = 0;
          }
        break;
    }

    switch(Sys.Pump2CycState)
    {
      case P2_CYC_STEP1:
          {
            P2_Step1();
            if(Sys.Delay_P2++ >= 5)      // scan time = 2ms * 5cnt = 10ms
            {
                PumpCycleTime = HAL_GetTick() - PumpTick;
                PumpTick = HAL_GetTick();              
                Sys.Delay_P2 = 0;
                Sys.Pump2CycState = P2_CYC_STEP2;
            }
          }
        break;

      case P2_CYC_STEP2:
          {
              P2_Step2();
              
              U16 EjectTime = (U16)(SetParam.val.ejection / (SCAN_TIME * 10));   // eeprom에 x10 정수형으로 저장되어있음.,
              if(Sys.Delay_P2++ >= EjectTime)
              {
                  Sys.Delay_P2 = 0;
                  Sys.Pump2CycState = P2_CYC_STEP3;
              }
          }
        break;

      case P2_CYC_STEP3:
          {
              P2_Step3();

              U16 DelayTime = (U16)(SetParam.val.delay / (SCAN_TIME * 10));       // eeprom에 x10 정수형으로 저장되어있음.,
              if(Sys.Delay_P2++ >= DelayTime)
              {
                  Sys.Delay_P2 = 0;
                  Sys.Pump2CycState = P2_CYC_STEP4;
              }
          }
        break;

      case P2_CYC_STEP4:
          {
              P2_Step4();

              if(Sys.Delay_P2++ >= 5)     // hold for 10ms
              {
                  Sys.Delay_P2 = 0;
                  Sys.Pump2CycState = P2_CYC_STEP5;
              }
          }
        break;

      case P2_CYC_STEP5:
          {
              P2_Step5();

              if(Sys.Delay_P2++ >= 5)     // hold for 10ms
              {
                  Sys.Delay_P2 = 0;
                  Sys.Pump2CycState = P2_CYC_STEP6;
              }
          }
        break;

      case P2_CYC_STEP6:
          {
              P2_Step6();

              if(Sys.Delay_P2++ >= 15)     // hold for 30ms
              {
                  Sys.Delay_P2 = 0;
                  Sys.Pump2CycState = P2_CYC_STEP7;
              }
          }
        break;

      case P2_CYC_STEP7:
          {
              P2_Step7();

              Sys.Delay_P2 = 0;
              Sys.Pump2CycState = 0;
          }
        break;
    }

    if(Sys.RealBPSTime >= Sys.CalcBPS) // 1 cycle 종료
    {
      float fVal = (float)((60000. / (float)(Sys.RealBPSTime * SCAN_TIME)) + 0.2);
      RealData.bpm = (U16)(fVal * 2);
      Sys.RealBPSTime = 0;
    }
}

U16 FirstStartDelay = 0;// Pump Run after 1.5s and then Cycle Run after 0.5s
void SysAuto(void)
{
    if(Sys.g.bfirststart && Sys.Pump1CycState == P1_CYC_STEP1)
    {
        FirstStartDelay++;
        if(FirstStartDelay > 750)
          AppData.IO.bit.TOMASPUMP = true;
        
        if(FirstStartDelay > 1000)
        {
          FirstStartDelay = 0;
          Sys.g.bfirststart = false; 
        }
    }

    if(!Sys.g.stop) // pushed start switch
    {
        //bool bAutoCondition = !Sys.Err.bit.inletpress && !Sys.Err.bit.bloodflow;      // alarm 발생시에도 구동.
        bool bAutoCondition = !Sys.Err.bit.bubble;      // bubblestopalarm = true & bubble detection, system stop.
        if(bAutoCondition)
        {
            U16 SetBPM = (U16)(SetParam.val.bpm / 10);
            Sys.CalcBPS = (U16)(60000 / SetBPM);            // SetParam에 x 10하여 정수형으로 eeprom에 저장되엇음.
            if(!Sys.g.cyclerun)
            {
                SetVentFlow(MAIN_PUMP1, 110);       // max. 11 LPM 
                SetVentFlow(MAIN_PUMP2, 110);       // max. 11 LPM
                SetVentFlow(SUB_PUMP, SetParam.val.ventflow);   
            }
        }
        else
        {
            if(!Sys.g.cyclerun)
            {
                SetVentFlow(MAIN_PUMP1, 0);
                SetVentFlow(MAIN_PUMP2, 0);
                SetVentFlow(SUB_PUMP, 0);
                
                AppData.IO.bit.TOMASPUMP = false;
                Sys.g.bfirststart = true;
            }
        }
    }
    else
    {
        if(!Sys.g.cyclerun)
        {
            Sys.g.bfirststart = true;
            FirstStartDelay = 0;
            // System Stop -> average buffer blood flow clear 2024.03.15 
            memset(Global.TempFlow, 0, sizeof(float)*60);
            SetVentFlow(MAIN_PUMP1, 0);       // Not Installed
            SetVentFlow(MAIN_PUMP2, 0);       // Not Installed
            SetVentFlow(SUB_PUMP, 0);
            AppData.IO.bit.TOMASPUMP = false;
        }
    }
    bool bRegStop = ((RealData.inlet < 0.5) && !Sys.g.cyclerun);
    //bool bRegStop = ((RealData.inlet < 0.7) && !Sys.g.cyclerun);
    if(bRegStop)
    {
      SetRegPressure(IN_OUT_REG, 0);         // Main Pump Regulator 
      SetRegPressure(MAIN_REG, 0);          // In & Out Pump Regulator    
    }
    else
    {
      SetRegPressure(IN_OUT_REG, SetParam.val.mainpress);         // Main Pump Regulator 
      SetRegPressure(MAIN_REG, SetParam.val.inoutpress);          // In & Out Pump Regulator    
    }
}

// Start Button On [2024.06.19]
void Start(void)
{
  switch(Sys.StartCycState)
  {
    case SW_C_START:
      if(!Start_On())
        break;
      Sys.StartTimerCnt = 0;
      if(Sys.g.emer || Sys.Err.bit.starton)
        break;
      if(Sys.g.autorun)
        break;
      Sys.g.stop = false;
      Sys.StopCycState = SW_C_END;
      break;
    
    case SW_C_END:
      Sys.StartTimerCnt++;
      if(Sys.StartTimerCnt > 50)
        Sys.StopCycState = SW_C_START;
      break;
  }
}

// Stop Button On [2024.06.19]
void Stop(void)
{
  switch(Sys.StopCycState)
  {
    case SW_C_START:
      if(!Stop_On())
        break;
      Sys.StopTimerCnt = 0;
      Sys.g.stop = true;
      Sys.StopCycState = SW_C_END;
      break;
    
    case SW_C_END:
      Sys.StopTimerCnt++;
      if(Sys.StopTimerCnt > 50)
        Sys.StopCycState = SW_C_START;
      break;
  }
}

void Seq(void)
{
    Start();
    Stop();
    
    // monitoring to push button state 
    Sys.g.startswon = Start_On();
    Sys.g.stopswon  = Stop_On();
    
    Sys.Err.bit.emeron = Sys.g.emer = Emer_On();
    //Sys.g.stop = !Sys.g.startswon || Sys.g.emer || Sys.Err.bit.starton;
    
    // Emergency, Bubble Alarm시 정지 : 2024.07.07
    if(Sys.g.emer|| Sys.Err.bit.bubble)
      Sys.g.stop = true;
    
    
    if(Sys.Err.bit.starton)
    {
      if(!Sys.g.startswon)
        Sys.Err.bit.starton = false;
    }
    
    if(Global.FlowSenCommCnt > 100)
      Sys.Err.bit.disconnectflow = true;
    else
      Sys.Err.bit.disconnectflow = false;
    
    Sys.g.cyclerun =  (Sys.Pump1CycState >= P1_CYC_STEP1) && (Sys.Pump1CycState <= ALL_CYC_END);


    // Start, Stop button 2024.06.18
    if(Sys.g.startswon)
    {
      if(Sys.g.autorun || Sys.g.cyclerun)
        Sys.g.startswon = false;
    }
    
    
    PumpCycle();        // Pump1,2 Cycle
    
    if (false == Sys.g.stop)
    {
        if (false == Sys.g.autorun) // STOP->RUN 
        {
            Global.TempFlowBufCnt = 0; 
            Global.FlowSenCommCnt = 0;  // Watchdog of FlowSensor Communication
            Global.bFirstFullTempBuf = false;
            Sys.g.autorun = true;
        }
    }
    else
    {
        if (true == Sys.g.autorun) // RUN->STOP 
            Sys.g.autorun = false;
        Global.FlowSenCommCnt = 0;  // Watchdog of FlowSensor Communication
    }
    
    SysAuto();
}

// Check System error
void CheckError(void)
{
    U16 RealFlow        = FlowVal.flow * 10;
    U16 RealInletPress  = RealData.inlet * 10;  
    
    if(!Sys.g.primingmode)
    {
        if(Sys.g.cyclerun || !Sys.g.stop)
        {
            // check in&out pressure
            if(RealData.inlet < 0.5)    // Inlet didn't connect. 
                Sys.g.chkinlerstart = true;
            else
            {
              if(RealInletPress > SetParam.val.alarm.inletpress)
                 Sys.g.chkinlerstart = true;
            }

            if(Sys.g.chkinlerstart && SetParam.val.alarm.inletpress > RealInletPress)
              Sys.Err.bit.inletpress = true;
            else
              Sys.Err.bit.inletpress = false;  
            
            // check blood flow error
            if(RealFlow > SetParam.val.alarm.bloodflow)
              Sys.g.chkflowstart = true;
            
            if(Sys.g.chkflowstart && SetParam.val.alarm.bloodflow > RealFlow)
              Sys.Err.bit.bloodflow = true;
            else
              Sys.Err.bit.bloodflow = false;
        }
        else
        {
            if(!Sys.g.startswon)
            {
                Sys.Err.bit.inletpress = false;
                Sys.Err.bit.outpress   = false;
                Sys.Err.bit.bloodflow  = false;
            }
            Sys.g.chkinlerstart = false;
            Sys.g.chkflowstart = false;
        }
    }
    else
    {
        Sys.Err.bit.inletpress = false;
        Sys.Err.bit.outpress   = false;
        Sys.Err.bit.bloodflow  = false;
        
        Sys.g.chkinlerstart = false;
        Sys.g.chkflowstart  = false;
    }
    
    if(Sys.g.cyclerun || !Sys.g.stop)
    {
        // check bubble alarm
        // bubble size   > over 20 * max 9.0mm%
        // bubble Alarm해제는 bubbleAlarmStop Option을 끌때만 가능.
        float BubbleErrorSize = (float)(9 * 0.2);
        if(Sys.g.bubblealarmstop && FlowVal.bubblesize > BubbleErrorSize)
          Sys.Err.bit.bubble = true;
          
    }
    
      // check low battery
      if(PwrState.ac == 0)      // ac off
      {
          if(PwrState.batt1 > 0)      // battery1 plug in
          {
            if(PwrState.batt1 < 1)
            //if(PwrState.batt1 < 4)    // test only
              Sys.Err.bit.lowbatt = 1;
            else
              Sys.Err.bit.lowbatt = 0;
          }
          else if(PwrState.batt2 > 0) // battery2 plug in
          {
            if(PwrState.batt2 < 1)
            //if(PwrState.batt2 < 4)    // test only
              Sys.Err.bit.lowbatt = 1;
            else
              Sys.Err.bit.lowbatt = 0;
          }
      }
      else
      {
        Sys.Err.bit.lowbatt = 0;
        LowBattBuzPeriod = 0;
        BattBuzOnCount = 0;
      }    
}

// Normal Alarm Buzzer On/Off
U32 buzzerOnTime = 0;
void BuzzerOnOff(bool bOn)
{
    if(Sys.g.BuzzerOff)
      bOn = false;
    
    if(bOn)
      buzzerOnTime++;
    else        // no alarm 
    {
        buzzerOnTime = 0;
        AppData.IO.bit.BUZZER = false;
        
        if(Sys.g.stop)  // stop : white
        {
          AppData.IO.bit.RLED = true;
          AppData.IO.bit.GLED = true;
          AppData.IO.bit.BLED = true;
        }
        else            // run : green
        {
          AppData.IO.bit.RLED = false;
          AppData.IO.bit.GLED = true;
          AppData.IO.bit.BLED = false;
        }
        return;
    }
    
    // buzzer process  
    if(buzzerOnTime < 1000)
    {
        AppData.IO.bit.BUZZER = true;
        // LAMP PROCESS
        if(Sys.Err.bit.starton) // Yellow
        {
          AppData.IO.bit.RLED = true;
          AppData.IO.bit.GLED = true;
          AppData.IO.bit.BLED = false;
        }
        else if(Sys.Err.bit.emeron || Sys.Err.bit.inletpress ||
                Sys.Err.bit.bloodflow || Sys.Err.bit.bubble || Sys.Err.bit.disconnectflow)
        {
          AppData.IO.bit.RLED = true;
          AppData.IO.bit.GLED = false; 
          AppData.IO.bit.BLED = false;
        }
    }
    else if(buzzerOnTime >= 1000 && buzzerOnTime < 2000)
    {
        AppData.IO.bit.BUZZER = false;
        // LAMP PROCESS
        if(Sys.Err.bit.starton) // Yellow
        {
          AppData.IO.bit.RLED = true;
          AppData.IO.bit.GLED = true;
          AppData.IO.bit.BLED = false;
        }
        else if(Sys.Err.bit.emeron || Sys.Err.bit.inletpress ||
                Sys.Err.bit.bloodflow || Sys.Err.bit.bubble)
        {
          AppData.IO.bit.RLED = false;
          AppData.IO.bit.GLED = false;
          AppData.IO.bit.BLED = false;          
        }
    }
    else
        buzzerOnTime  = 0;  
}

// Batt Buzzer On/Off

void BattBuzOnOff(bool bOn)
{
    if(bOn)
      BattbuzOnTime++;
    else
    {
        buzzerOnTime = 0;
        AppData.IO.bit.BUZZER = false;
        return;
    }

    if(BattbuzOnTime < 500)
        AppData.IO.bit.BUZZER = true;
    else if(BattbuzOnTime >= 500 && BattbuzOnTime < 1000)
        AppData.IO.bit.BUZZER = false;
    else
    {
        BattbuzOnTime = 0;  
        if(bOn)
        {
          BattBuzOnCount++;
          if(BattBuzOnCount > 200)
            BattBuzOnCount = 0;
        }
        else
          BattBuzOnCount = 0;
    }
}
