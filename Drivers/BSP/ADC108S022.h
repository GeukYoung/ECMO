#ifndef   __ADC108S022_H__
#define   __ADC108S022_H__

#include "project.h"

#include "ISPI.h"

#ifdef __ADC108S022_C__
	#define ADC108S022_EXT
#else
	#define ADC108S022_EXT extern
#endif

typedef enum
{
  ADC_CH0,
  ADC_CH1,
  ADC_CH2,
  ADC_CH3,
  ADC_CH4,
  ADC_CH5,
  ADC_CH6,
  ADC_CH7,  
}eADC_CH;


ADC108S022_EXT void     ADC108S022_Init(void);
ADC108S022_EXT U16      ADC108S022_GetData(eADC_CH ch);

#endif