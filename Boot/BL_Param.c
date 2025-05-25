#define __BL_PARAM_C__
    #include "BL_Param.h"
#undef  __BL_PARAM_C__
#include "Version.h"
#include "flash_if.h"

static void BootloaderParam_FactorySet(void);

void BootloaderParam_Read(void)
{
    FLASH_If_ReadWord(BOOT_PARAM_ADDR, sizeof(BootParam_t), (U8*)&BootParam);  
}

void BootloaderParam_Save(void)
{
    FLASH_If_ParamAreaErase(BOOT_PARAM_ADDR);
    FLASH_If_WriteWord(BOOT_PARAM_ADDR, sizeof(BootParam_t), (U16*)&BootParam); 
}

void BootloaderParam_FactorySet(void)
{        
    U8 str[20] = "No App File";    
    
    memset((U8*)&BootParam, 0, sizeof(BootParam_t));
    
    BootParam.fwStatus.bootMode = BOOT_MODE;

    memcpy(BootParam.fwStatus.appFileInfo.fileName,str,sizeof(str));
    BootParam.fwStatus.appFileInfo.appSize    = 0;
    BootParam.fwStatus.appFileInfo.time.year  = 19;
    BootParam.fwStatus.appFileInfo.time.month = 10;
    BootParam.fwStatus.appFileInfo.time.day   = 15;
    BootParam.fwStatus.appFileInfo.time.hour  = 17;
    BootParam.fwStatus.appFileInfo.time.min   = 24;
    BootParam.fwStatus.appFileInfo.time.sec   = 59;
     
    BootParam.Confirm = CONFIRM_BOOT_PARAM;
    BootloaderParam_Save();
}

void BootloaderParam_Init()
{    
    BootloaderParam_Read();

    if( BootParam.Confirm != CONFIRM_BOOT_PARAM )
    {
        BootloaderParam_FactorySet();
    } 
}

void BootloaderParam_UpdateFwStatus(FwStatus_t *fwStatus)
{
    BootloaderParam_Read();
    memcpy((void*)&BootParam.fwStatus, fwStatus, sizeof(FwStatus_t));    
    BootloaderParam_Save();
}
