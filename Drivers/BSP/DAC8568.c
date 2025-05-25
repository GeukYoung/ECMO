#define __DAC8568_C__
    #include "DAC8568.h"
#undef  __DAC8568_C__

// 2023.12.08 
// DA 8Ch.[2'nd PCB]

static void GPIO_Init(void);

void DAC8568_Init(void)
{
    GPIO_Init();
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4  |
                          GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
}

static void SCK_LOW(void)       { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET); }
static void SCK_HIGH(void)      { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET); }

static void MOSI_LOW(void)      { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET); }
static void MOSI_HIGH(void)     { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET); }
                    
static void CLR_LOW(void)       { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET); }
static void CLR_HIGH(void)      { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET); }

static void LDAC_LOW(void)      { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET); }
static void LDAC_HIGH(void)     { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET); }

static void SYNC_LOW(void)      { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET); }
static void SYNC_HIGH(void)     { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); }

// clk : 3us
// time : 70us
void DAC8568_Out(eDAC_CH ch, U16 data)
{
    U8  i;
    U32 reg;       
    U32 DacOut;

    volatile U32 Dly = 1;
 
    switch(ch)
    {
    case DAC_CH0 : reg = REG_DAC_CHA; break;
    case DAC_CH1 : reg = REG_DAC_CHB; break;
    case DAC_CH2 : reg = REG_DAC_CHC; break;
    case DAC_CH3 : reg = REG_DAC_CHD; break;
    case DAC_CH4 : reg = REG_DAC_CHE; break;
    case DAC_CH5 : reg = REG_DAC_CHF; break;
    case DAC_CH6 : reg = REG_DAC_CHG; break;
    case DAC_CH7 : reg = REG_DAC_CHH; break;
    }
    
    DacOut = (reg | ((U32)data<<4)&0x000FFFF0);
	
    CLR_HIGH(); 
    //LDAC_HIGH();
    LDAC_LOW();
    

    SCK_LOW();
    SYNC_LOW();
    for(; Dly; Dly--);
    SCK_HIGH();
    for(; Dly; Dly--);
    SCK_LOW();
    for(; Dly; Dly--);
    SYNC_HIGH();
    SCK_HIGH();
    for(; Dly; Dly--);
    SCK_LOW();
    SYNC_LOW();
    
    for(; Dly; Dly--);

    for(i=0; i<32; i++)
    {
        SCK_HIGH();
    
    	//if(i == 0) { CLR_LOW(); }
    
        if(DacOut & 0x80000000){ MOSI_HIGH(); }
        else                   { MOSI_LOW();  }     
    
        for(; Dly; Dly--);
    
        SCK_LOW();
        
        //if(i == 1) { CLR_HIGH(); }

        DacOut = DacOut << 1;
    
        for(; Dly; Dly--);
    }

    SCK_HIGH();
    for(; Dly; Dly--);
    SCK_LOW();
    for(; Dly; Dly--);
    SCK_HIGH();
    LDAC_LOW();
    
    for(; Dly; Dly--);
    SCK_LOW();
    SYNC_HIGH();
    for(; Dly; Dly--);
    SCK_HIGH();
    //LDAC_HIGH();
}

void Cmd_DAC8568(U32 data)
{
    U8  i;
    U32 Cmd = data;
    
    volatile U32 Dly = 1;
 
    CLR_HIGH(); 
    //LDAC_HIGH();
    LDAC_LOW();

    SCK_LOW();
    SYNC_LOW();
    for(; Dly; Dly--);
    SCK_HIGH();
    for(; Dly; Dly--);
    SCK_LOW();
    for(; Dly; Dly--);
    SYNC_HIGH();
    SCK_HIGH();
    for(; Dly; Dly--);
    SCK_LOW();
    SYNC_LOW();
    
    for(; Dly; Dly--);

    for(i=0; i<32; i++)
    {
        SCK_HIGH();
    
    	//if(i == 0) { CLR_LOW(); }
    
        if(Cmd & 0x80000000){ MOSI_HIGH(); }
        else                { MOSI_LOW();  }     
    
        for(; Dly; Dly--);
    
        SCK_LOW();
        
        //if(i == 1) { CLR_HIGH(); }
        
        Cmd = Cmd << 1;
    
        for(; Dly; Dly--);
    }

    SCK_HIGH();
    for(; Dly; Dly--);
    SCK_LOW();
    for(; Dly; Dly--);
    SCK_HIGH();
    LDAC_LOW();
    
    for(; Dly; Dly--);
    SCK_LOW();
    SYNC_HIGH();
    for(; Dly; Dly--);
    SCK_HIGH();
    //LDAC_HIGH();
}