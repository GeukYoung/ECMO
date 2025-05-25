#define __ADC108S022_C__
    #include "ADC108S022.h"
#undef  __ADC108S022_C__

void ADC108S022_Init(void)
{
    ISPI2_Init();
}

// clk : 3.6us
// time : 128us
U16 ADC108S022_GetData(eADC_CH ch)
{
    uIntConverter_t RdData;
    U8 WrData, dummy = 0;
    
    WrData = ch<<3;
    SPI2_CSL();
    
    RdData._byte[1] = ISPI2_SendData(WrData);
    RdData._byte[0] = ISPI2_SendData(dummy);
    
    RdData._byte[1] = ISPI2_SendData(WrData);
    RdData._byte[0] = ISPI2_SendData(dummy);
    
    SPI2_CSH();
    
    UTIL_DelayUS(10);

    return (RdData._short[0]>>2);
}
