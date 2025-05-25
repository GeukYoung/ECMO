#define __MSG_PROC_C__
    #include "MsgProc.h"
#undef  __MSG_PROC_C__

#include "Version.h"
#include "BL_Param.h"

static U32            fregIndex;
static U32            totIndex;
static BOOL           IsSystemReset = false;

static U32 Fw_Update_Version (BootPacket_t* msg);
static U32 Fw_Update_Status  (BootPacket_t* msg);
static U32 Fw_Update_Reset   (BootPacket_t* msg);

void MsgProc(SOCKET sock,SockAddr_t *dest_addr,BootPacket_t* msg)
{
U32 txSize;

    switch (msg->header.cmd)
    {
    case GET_FW_UPDATE_VERSION    : txSize = Fw_Update_Version(msg);
    break;
    case GET_FW_UPDATE_STATUS    : txSize = Fw_Update_Status(msg);
    break;
    case SET_FW_UPDATE_RESET     : txSize = Fw_Update_Reset(msg);
    break;
    }  
  
    if(txSize > 0)
    {
        Socket_Sendto(sock,(U8*)msg,txSize,dest_addr);
        if(IsSystemReset)
        {
            HAL_Delay(100);
            NVIC_SystemReset();
        }
    }
}

static U32 Fw_Update_Version(BootPacket_t* msg)
{
    memcpy(&msg->payload.fwVer.version,MCU_VERSION_STR,strlen(MCU_VERSION_STR)+1);

    msg->header.size = sizeof(BootHeader_t) + sizeof(FwVer_t);
    msg->header.errCode = ERR_OK;
    return msg->header.size;
}

static U32 Fw_Update_Reset   (BootPacket_t* msg)
{
    IsSystemReset = true;
    msg->header.size = sizeof(BootHeader_t);
    msg->header.errCode = ERR_OK;
    return msg->header.size;  
}

static U32 Fw_Update_Status(BootPacket_t* msg)
{
    BootParam.fwStatus.bootMode = APP_MODE;
    memcpy(&msg->payload.fwStatus,&BootParam.fwStatus,sizeof(FwStatus_t));

    msg->header.size = sizeof(BootHeader_t) + sizeof(FwStatus_t);
    msg->header.errCode = ERR_OK;
    return msg->header.size;
}







