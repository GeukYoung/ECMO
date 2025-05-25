#ifndef   __ITIMER_H__
#define   __ITIMER_H__

#include "project.h"

#ifdef __ITIMER_C__
	#define ITIMER_EXT
#else
	#define ITIMER_EXT extern
#endif

ITIMER_EXT void ITIMER3_Init(void);
ITIMER_EXT void ITIMER3_IsrStart(void);
ITIMER_EXT void ITIMER3_IsrStop(void);
ITIMER_EXT void ITIMER3_SetIsrPeriod(U8 freq);

ITIMER_EXT void (*ITIMER3_Callback)(void);

#endif


