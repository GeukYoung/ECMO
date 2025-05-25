#define __BUZZER_C__
    #include "Buzzer.h"
#undef  __BUZZER_C__

static void GPIO_Init(void);
static void BUZZER_OnOff(U8 OnOff);

void BUZZER_Init(void)
{
    GPIO_Init();
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);    
}

static void BUZZER_OnOff(U8 OnOff)
{
    if(OnOff == ON) { HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); }
    else            { HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); } 
}

void BUZZER_Start(void)
{
    static U8 BuzzerState = 0;
    static U32 BuzzerCount;
    U8 BuzzerVal = 0;
    
    BuzzerCount++;

    switch(BuzzerState)
    {
    case 0 : 
        BUZZER_OnOff(ON);
        BuzzerVal = ON;
        if(BuzzerCount > 500)
        {
            BuzzerState = 1;
            BuzzerCount = 0;
        }
        break;
    case 1 : 
        BuzzerVal = OFF;
        if(BuzzerCount > 1500)
        {
            BuzzerState = 0;
            BuzzerCount = 0;
        }
        break;
    }
    
    BUZZER_OnOff(BuzzerVal);
}

void BUZZER_Stop(void)
{
    BUZZER_OnOff(OFF);
}
