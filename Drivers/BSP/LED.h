#ifndef   __LED_H__
#define   __LED_H__

#include "project.h"

#ifdef __LED_C__
	#define LED_EXT
#else
	#define LED_EXT extern
#endif

typedef enum
{
  STATUS0,
  STATUS1,
}eLedNum;

LED_EXT void LED_Init(void);
LED_EXT void LED_OnOff(eLedNum num,U8 OnOff);
LED_EXT void LED_Toggle(eLedNum num);

#endif
