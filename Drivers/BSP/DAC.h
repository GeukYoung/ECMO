#ifndef   __DAC_H__
#define   __DAC_H__

#include "project.h"

#ifdef __DAC_C__
	#define DAC_EXT
#else
	#define DAC_EXT extern
#endif

typedef enum
{
  DAC_CH0,
  DAC_CH1,
  DAC_CH2,
  DAC_CH3,
}eDAC_CH;

#define REG_DAC_CH0     0x0010
#define REG_DAC_CH1     0x0012
#define REG_DAC_CH2     0x0014
#define REG_DAC_CH3     0x0016

DAC_EXT void DAC_Init(void);
DAC_EXT void DAC_Out(eDAC_CH ch, U16 data);

#endif