#ifndef   __MCP23S08_H__
#define   __MCP23S08_H__

#include "project.h"

#include "ISPI.h"

#ifdef __MCP23S08_C__
	#define MCP23S08_EXT
#else
	#define MCP23S08_EXT extern
#endif

#define MCP23S08        0x40

#define MCP23S08_READ   0x01
#define MCP23S08_WRITE  0x00

#define REG_IO_DIR      0x00
#define REG_IOCON       0x05
#define REG_GPIO        0x09
#define REG_OLAT        0x0A

#define IOCON_HAEN      0x08           

#define DIR_OUT         0x00


MCP23S08_EXT void MCP23S08_Init(void);
MCP23S08_EXT void MCP23S08_SetOut(U32 out);

#endif