#ifndef   __DAC8568_H__
#define   __DAC8568_H__

#include "project.h"

#ifdef __DAC8568_C__
	#define DAC8568_EXT
#else
	#define DAC8568_EXT extern
#endif

typedef enum
{
  DAC_CH0,
  DAC_CH1,
  DAC_CH2,
  DAC_CH3,
  DAC_CH4,
  DAC_CH5,
  DAC_CH6,
  DAC_CH7,
}eDAC_CH;


#define REG_DAC_CHA     0x02000000
#define REG_DAC_CHB     0x02100000
#define REG_DAC_CHC     0x02200000
#define REG_DAC_CHD     0x02300000
#define REG_DAC_CHE     0x02400000
#define REG_DAC_CHF     0x02500000
#define REG_DAC_CHG     0x02600000
#define REG_DAC_CHH     0x02700000

#define INTENAL_REF_ON  0x090A0000

DAC8568_EXT void DAC8568_Init(void);
DAC8568_EXT void DAC8568_Out(eDAC_CH ch, U16 data);
DAC8568_EXT void Cmd_DAC8568(U32 data);
#endif