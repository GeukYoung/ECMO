#define __MCP23S08_C__
    #include "MCP23S08.h"
#undef  __MCP23S08_C__

static void MCP23S08_RegInit(void);

void MCP23S08_Init(void)
{
    ISPI1_Init();
    
    SPI1_RST_H();
    UTIL_DelayMS(5);
    SPI1_RST_L();
    UTIL_DelayMS(5);
    SPI1_RST_H();    
    UTIL_DelayMS(5);
    
    MCP23S08_RegInit();
}

static void MCP23S08_RegInit(void)
{
    U8 i;
    U8 Opcode;
    
    for(i=0; i<3; i++)
    {
        Opcode = MCP23S08 | (i<<1)&0x06;
        
        SPI1_CSL();  

        ISPI1_SendData(Opcode);
        ISPI1_SendData(REG_IOCON);
        ISPI1_SendData(IOCON_HAEN);      
          
        SPI1_CSH();
        
        UTIL_DelayUS(10);
    }
    
    for(i=0; i<3; i++)
    {
        Opcode = MCP23S08 | (i<<1)&0x06;
        
        SPI1_CSL();  

        ISPI1_SendData(Opcode);
        ISPI1_SendData(REG_IO_DIR);
        ISPI1_SendData(DIR_OUT);      
          
        SPI1_CSH();
        
        UTIL_DelayUS(10);
    }
}

// clk : 1.8us
// time : 160us
void MCP23S08_SetOut(U32 out)
{
    U8 i;
    U8 Opcode;
    U8 data;
    
    for(i=0; i<3; i++)
    {
        Opcode = MCP23S08 | ((i<<1)&0x06);
        data = ((out >> (i*8)) & 0xFF);
        
        SPI1_CSL();  

        ISPI1_SendData(Opcode);
        ISPI1_SendData(REG_GPIO);
        ISPI1_SendData(data);      
          
        SPI1_CSH();
        
        UTIL_DelayUS(10);
    }
}
