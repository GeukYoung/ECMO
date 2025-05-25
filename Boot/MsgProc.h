#ifndef   __MSG_PROC_H__
#define   __MSG_PROC_H__

#include "project.h"
#include "BootProtocol.h"
#include "socket.h"

#ifdef __MSG_PROC_C__
	#define MSG_PROC_EXT
#else
	#define MSG_PROC_EXT extern
#endif

MSG_PROC_EXT void MsgProc(SOCKET sock,SockAddr_t *dest_addr,BootPacket_t* msg);

#endif
