#ifndef   __DAC8564_H__
#define   __DAC8564_H__

#include "project.h"

#ifdef __DAC8564_C__
	#define DAC8564_EXT
#else
	#define DAC8564_EXT extern
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

DAC8564_EXT void DAC8564_Init(void);
DAC8564_EXT void DAC8564_Out(eDAC_CH ch, U16 data);

#endif