#ifndef   _SEQ_H_
#define   _SEQ_H_

#include "project.h"

#include "ITIMER.h"
//#include "DAC8564.h"
#include "DAC8568.h"
#include "ADC108S022.h"
#include "MCP23S08.h"
#include "Buzzer.h"

#ifdef _SEQ_C_
	#define SEQ_EXT
#else
	#define SEQ_EXT extern
#endif

typedef enum {
	IDLE = 0,
        CHK_ST_SW,
	AUTO,
        
        MAIN_PUMP1 = 0,
        MAIN_PUMP2,
        SUB_PUMP,
        
        MAIN_REG = 0,
        IN_OUT_REG,
        
} State;


typedef enum {
	CYC_ST   = 1,
        CYC_STEP1,      // hold for 10ms
	CYC_STEP2,      // hold for Ejection time
        CYC_STEP3,      // hold for Set Delay
        CYC_STEP4,      // hold for 10ms
        CYC_STEP5,      // hold for 10ms
	CYC_STEP6,      // hold for 30ms
        CYC_STEP7,      // hold for (STEP0 result[time] - SUM(STEP1~7 all time)[ms]
        CYC_ED,        
        
        
        
        P1_CYC_STEP1 = 1,
        P1_CYC_STEP2,
        P1_CYC_STEP3,
        P1_CYC_STEP4,
        P1_CYC_STEP5,
        P1_CYC_STEP6,
        P1_CYC_STEP7,
        
        P2_CYC_STEP1,
        P2_CYC_STEP2,
        P2_CYC_STEP3,
        P2_CYC_STEP4,
        P2_CYC_STEP5,
        P2_CYC_STEP6,
        P2_CYC_STEP7,
        
        ALL_CYC_END,

        
} CycState;


SEQ_EXT void  ADDAInit(void);
SEQ_EXT void DAUpdate(void);
SEQ_EXT void ADUpdate(void);
SEQ_EXT void SetRegPressure(U8 no, U8 val);
SEQ_EXT void ReadWrite(void);

SEQ_EXT void Pump1Cycle(void);
SEQ_EXT void Pump2Cycle(void);
SEQ_EXT void SysAuto(void);
SEQ_EXT void Seq(void);
SEQ_EXT void BuzzerOnOff(bool bOn);
SEQ_EXT void BattBuzOnOff(bool bOn);



#endif
