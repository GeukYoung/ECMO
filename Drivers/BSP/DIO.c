#define __DIO_C__
    #include "DIO.h"
#undef  __DIO_C__

#define EMER_PIN                GPIO_PIN_0      // TP5
#define START_PIN               GPIO_PIN_1      // TP6
#define MAIN_PUMP1_TACHO        GPIO_PIN_2      // TP
#define MAIN_PUMP2_TACHO        GPIO_PIN_3      // TP
#define SUB_PUMP_TACHO          GPIO_PIN_4      // TP

#define DP_102A_SENSOR          GPIO_PIN_12      // TP
#define SMPS_ON                 GPIO_PIN_13      // TP

static void GPIO_Init(void);

void DIO_Init(void)
{
    GPIO_Init();
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct.Pin = EMER_PIN | START_PIN | MAIN_PUMP1_TACHO | MAIN_PUMP2_TACHO |
                          MAIN_PUMP1_TACHO | SUB_PUMP_TACHO | DP_102A_SENSOR | SMPS_ON;
    
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

bool Emer_On(void)
{
    if(GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOF, EMER_PIN))
      return true;
    else
      return false;
}

bool Start_On(void)
{

    if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOF, START_PIN))
      return true;
    else
      return false;
}

bool Main1PumpTacho_On(void)
{
    if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOF, MAIN_PUMP1_TACHO))
      return true;
    else
      return false;
}

bool Main2PumpTacho_On(void)
{

    if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOF, MAIN_PUMP2_TACHO))
      return true;
    else
      return false;
}

bool SubPumpTacho_On(void)
{

    if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOF, SUB_PUMP_TACHO))
      return true;
    else
      return false;
}

bool Smps_On(void)
{
    if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOF, SMPS_ON))
      return true;
    else
      return false;
}
