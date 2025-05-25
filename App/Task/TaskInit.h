#ifndef   __TASK_INIT_H__
#define   __TASK_INIT_H__

#include "project.h"

#ifdef __TASK_INIT_C__
	#define TASK_INIT_EXT
#else
	#define TASK_INIT_EXT extern
#endif

TASK_INIT_EXT void Task_Init(void);

#endif
