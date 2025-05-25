#ifndef   __BOOT_PROTOCOL_H__
#define   __BOOT_PROTOCOL_H__

#include "project.h"
#include "socket.h"
#ifdef __BOOT_PROTOCOL_C__
	#define BOOT_PROTOCOL_EXT
#else
	#define BOOT_PROTOCOL_EXT extern
#endif

#define HEADER_SIZE	       (12)
#define FRAG_SIZE              (2)
#define PAYLOAD_MAX_SIZE       (UDP_MSS-HEADER_SIZE)
#define PACKET_DATA_SIZE       (PAYLOAD_MAX_SIZE-FRAG_SIZE)

typedef enum EBootMode
{
    BOOT_MODE = 0x1,
    APP_MODE
}EBootMode_t;

typedef enum ErrCode
{
  ERROR_CODE_START = 0x00,

  ERR_OK,
  ERR_WRITE,
  ERR_ERASE,
  ERR_INDEX_NOT_MATCHING,
  ERR_PROGRAM_FAIL,
  
  ERROR_CODE_END
}ErrCode_t;

typedef enum ProtocolCommand
{
  COMMAND_START = 0x00,

  GET_FW_UPDATE_VERSION,
  GET_FW_UPDATE_STATUS,	
  SET_FW_UPDATE_RESET,
  SET_FW_UPDATE_START,        
  SET_FW_UPDATE_DATA,	        
  SET_FW_UPDATE_FINISH,

  SET_FACTORY_SET,
  
  COMMAND_END
}ProtocolCommend_t;

#pragma pack(push,1)
typedef struct Time
{//File Info
  unsigned char year;
  unsigned char month;
  unsigned char day;
  unsigned char hour;
  unsigned char min;
  unsigned char sec;
}Time_t;

typedef struct AppFileInfo
{
  unsigned char fileName[256];
  unsigned int  appSize;
  Time_t        time;  
}AppFileInfo_t;

typedef struct FwStatus
{
    unsigned char bootMode;
    AppFileInfo_t  appFileInfo;
}FwStatus_t;

typedef struct FwVer
{
    unsigned char version[32];
}FwVer_t;

typedef struct Fw_Start
{
  unsigned int fwSize;
}Fw_Start_t;

typedef struct Fw_Data
{
  unsigned short fregIdx;
  unsigned char data[PACKET_DATA_SIZE];
}Fw_Data_t;

typedef union BootPayload
{
  FwVer_t      fwVer;
  FwStatus_t    fwStatus;
  Fw_Start_t    fwStart;
  Fw_Data_t     fwData;
  unsigned char byte[PAYLOAD_MAX_SIZE];
}BootPayload_t;

typedef struct BootHeader
{
  unsigned int  cmd;
  unsigned int size;
  unsigned int  errCode;
}BootHeader_t;

typedef struct BootPacket
{
  BootHeader_t  header;
  BootPayload_t payload;
}BootPacket_t;
#pragma pack(pop)

#endif
