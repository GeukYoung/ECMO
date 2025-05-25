#ifndef   _CONTROL_TASK_H_
#define   _CONTROL_TASK_H_

#include "project.h"

#ifdef _CONTROL_TASK_C_
	#define CONTROL_TASK_EXT
#else
	#define CONTROL_TASK_EXT extern
#endif

CONTROL_TASK_EXT void ControlTask_Init(void);

#endif
