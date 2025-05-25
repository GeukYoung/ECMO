#ifndef   __BUZZER_H__
#define   __BUZZER_H__

#include "project.h"

#ifdef __BUZZER_C__
	#define BUZZER_EXT
#else
	#define BUZZER_EXT extern
#endif

BUZZER_EXT void BUZZER_Init(void);
BUZZER_EXT void BUZZER_Start(void);
BUZZER_EXT void BUZZER_Stop(void);

#endif


