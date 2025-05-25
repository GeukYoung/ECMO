#ifndef   __BL_PARAM_H__
#define   __BL_PARAM_H__

#include "project.h"
#include "flash_if.h"
#include "BootProtocol.h"

#ifdef __BL_PARAM_C__
	#define BL_PARAM_EXT
#else
	#define BL_PARAM_EXT extern
#endif

#pragma pack(push,1)
typedef struct BootParam
{
    FwStatus_t fwStatus;
    U32        Confirm;
}BootParam_t;
#pragma pack(pop)

BL_PARAM_EXT   BootParam_t   BootParam;

BL_PARAM_EXT void BootloaderParam_Init(void);
BL_PARAM_EXT void BootloaderParam_Save(void);
BL_PARAM_EXT void BootloaderParam_Read(void);
BL_PARAM_EXT void BootloaderParam_UpdateFwStatus(FwStatus_t *fwStatus);
#endif
