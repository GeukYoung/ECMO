#define __APP_DATA_C__
    #include "AppData.h"
#undef  __APP_DATA_C__

#include "Version.h"

//void AppData_Init(void)
//{   
//    AppData.DA.out[0] = 0;
//    AppData.DA.out[1] = 0;
//    AppData.DA.out[2] = 0;
//    AppData.DA.out[3] = 0;
//    
//    AppData.Buzzer = FALSE;
//    F
//    AppData.fwVersion = MCU_VERSION;
//}

void AppData_Init(void)
{   
    AppData.DA.inoutreg = 0;
    AppData.DA.mainreg = 0;
    
    AppData.DA.mpump[0] = 0;
    AppData.DA.mpump[1] = 0;
    AppData.DA.spump = 0;
    
    AppData.DA.spare[0] = 0;
    AppData.DA.spare[1] = 0;
    AppData.DA.spare[2] = 0;
    
    AppData.Buzzer = FALSE;
    
    AppData.fwVersion = MCU_VERSION;
}
