#define __LED_C__
    #include "LED.h"
#undef  __LED_C__

#define LED0_PIN        GPIO_PIN_6
#define LED1_PIN        GPIO_PIN_7

static void GPIO_Init(void);

void LED_Init(void)
{
    GPIO_Init();
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);

    LED_OnOff(STATUS0,0);
    LED_OnOff(STATUS1,0);
}

void LED_OnOff(eLedNum num,U8 OnOff)
{
    if(OnOff == true)
    {
        switch(num)
        {
        case STATUS0 : HAL_GPIO_WritePin(GPIOF, LED0_PIN, GPIO_PIN_RESET); break;
        case STATUS1 : HAL_GPIO_WritePin(GPIOF, LED1_PIN, GPIO_PIN_RESET); break;
        }
    }
    else
    {
        switch(num)
        {
        case STATUS0 : HAL_GPIO_WritePin(GPIOF, LED0_PIN, GPIO_PIN_SET); break;
        case STATUS1 : HAL_GPIO_WritePin(GPIOF, LED1_PIN, GPIO_PIN_SET); break;
        }
    }
}

void LED_Toggle(eLedNum num)
{
    switch(num)
    {
    case STATUS0 : HAL_GPIO_TogglePin(GPIOF, LED0_PIN); break;
    case STATUS1 : HAL_GPIO_TogglePin(GPIOF, LED1_PIN); break;
    }
}