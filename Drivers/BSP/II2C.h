#ifndef   __II2C_H__
#define   __II2C_H__

#include "project.h"

#ifdef __II2C_C__
	#define II2C_EXT
#else
	#define II2C_EXT extern
#endif

#define AT24Cxx_ADDR            0xA0  

II2C_EXT void II2C_Init(void);

II2C_EXT void II2C_WriteByte(U16 addr, U16 size, U8* pData);
II2C_EXT void II2C_ReadByte(U16 addr, U16 size, U8* pData);

#endif