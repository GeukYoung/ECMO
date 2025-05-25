#ifndef   __DIO_H__
#define   __DIO_H__

#include "project.h"

#ifdef __DIOC__
	#define DIO_EXT
#else
	#define DIO_EXT extern
#endif


DIO_EXT void DIO_Init(void);
DIO_EXT bool Emer_OnDIO_EXT (void);
DIO_EXT bool Start_On(void);
DIO_EXT bool MainPumpTacho_On(void);
DIO_EXT bool SubPump1Tacho_On(void);
DIO_EXT bool SubPump2Tacho_On(void);
DIO_EXT bool Smps_On(void);
#endif
