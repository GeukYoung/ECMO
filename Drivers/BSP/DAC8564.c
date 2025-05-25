#define __DAC8564_C__
    #include "DAC8564.h"
#undef  __DAC8564_C__

static void GPIO_Init(void);

void DAC8564_Init(void)
{
    GPIO_Init();
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3  |
                          GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
}

static void SCK_LOW(void)       { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET); }
static void SCK_HIGH(void)      { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET); }

static void MOSI_LOW(void)      { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET); }
static void MOSI_HIGH(void)     { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET); }
                    
static void A0_LOW(void)        { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET); }
static void A0_HIGH(void)       { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET); }

static void A1_LOW(void)        { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET); }
static void A1_HIGH(void)       { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET); }

static void LDAC_LOW(void)      { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET); }
static void LDAC_HIGH(void)     { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET); }

static void EN_LOW(void)        { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET); }
static void EN_HIGH(void)       { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET); }

static void SYNC_LOW(void)      { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET); }
static void SYNC_HIGH(void)     { HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); }

// clk : 3us
// time : 70us
void DAC8564_Out(eDAC_CH ch, U16 data)
{
    U8  i;
    U16 reg;
    U32 DacOut;    

    volatile U32 Dly = 1;

    switch(ch)
    {
    case DAC_CH0 : reg = REG_DAC_CH0; break;
    case DAC_CH1 : reg = REG_DAC_CH1; break;
    case DAC_CH2 : reg = REG_DAC_CH2; break;
    case DAC_CH3 : reg = REG_DAC_CH3; break;
    }

    DacOut = (U32)((((U32)reg<<16)&0xFF0000) | (U32)(data&0xFFFF));

    SCK_LOW();
    EN_HIGH();
    SYNC_LOW();
    for(; Dly; Dly--);
    SCK_HIGH();
    for(; Dly; Dly--);
    SCK_LOW();
    for(; Dly; Dly--);
    SYNC_HIGH();
    SCK_HIGH();
    EN_LOW();  
    for(; Dly; Dly--);
    SCK_LOW();
    SYNC_LOW();

    for(; Dly; Dly--);

    for(i=0; i<24; i++)
    {
        SCK_HIGH();
    
        if(DacOut & 0x800000){ MOSI_HIGH(); }
        else                 { MOSI_LOW();  }     
    
        for(; Dly; Dly--);
    
        SCK_LOW();
    
        DacOut = DacOut << 1;
    
        for(; Dly; Dly--);
    }

    SCK_HIGH();
    for(; Dly; Dly--);
    SCK_LOW();
    for(; Dly; Dly--);
    SCK_HIGH();
    LDAC_HIGH();

    for(; Dly; Dly--);
    SCK_LOW();
    EN_HIGH();
    SYNC_HIGH();
    for(; Dly; Dly--);
    SCK_HIGH();
    LDAC_LOW();
}
