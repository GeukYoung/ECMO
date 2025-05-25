#ifndef   __IUART_H__
#define   __IUART_H__

#include "project.h"

#ifdef __IUART_C__
	#define IUART_EXT
#else
	#define IUART_EXT extern
#endif

#define TERMINATE_TICK_TIME     (2) //2ms

IUART_EXT U8    IUART1_RecvFlag;
IUART_EXT U8    IUART1_RecvTick;

IUART_EXT U8    IUART2_RecvFlag;
IUART_EXT U8    IUART2_RecvTick;

IUART_EXT U8    IUART3_RecvFlag;
IUART_EXT U8    IUART3_RecvTick;

// RS232 Add [Spare] 2023.12.10
IUART_EXT U8    IUART4_RecvFlag;
IUART_EXT U8    IUART4_RecvTick;

//IUART_EXT void  IUART_Init(RS_t* rs, RS_t* rsA, RS_t* rsB);
IUART_EXT void IUART_Init(RS_t* rs, RS_t* rsA, RS_t* rsS1, RS_t* rsS2);

IUART_EXT U8    IUART1_CheckRecvEnd(void);
IUART_EXT U8    IUART2_CheckRecvEnd(void);
IUART_EXT U8    IUART3_CheckRecvEnd(void);
IUART_EXT U8    IUART4_CheckRecvEnd(void);       // RS232 Add [Sprare] 2023.12.10

IUART_EXT U32   IUART1_GetRecvSize(void);
IUART_EXT U32   IUART2_GetRecvSize(void);
IUART_EXT U32   IUART3_GetRecvSize(void);
IUART_EXT U32   IUART4_GetRecvSize(void);       // RS232 Add [Sprare] 2023.12.10

IUART_EXT void  IUART1_WriteBytes(uint8_t* pData, uint16_t size);       // RS485A : Flow_Sensor1
IUART_EXT void  IUART2_WriteBytes(uint8_t* pData, uint16_t size);       // RS485B -> RS232[Spare1], PC
IUART_EXT void  IUART3_WriteBytes(uint8_t* pData, uint16_t size);       // RS232 : LCD
IUART_EXT void  IUART4_WriteBytes(uint8_t* pData, uint16_t size);       // RS232 Add [Sprare2] 2023.12.10

IUART_EXT U8    IUART1_ReadByte(void);
IUART_EXT U8    IUART2_ReadByte(void);
IUART_EXT U8    IUART3_ReadByte(void);
IUART_EXT U8    IUART4_ReadByte(void);          // RS232 Add [Sprare] 2023.12.10
#endif


