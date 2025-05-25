#define __ITIMER_C__
    #include "ITIMER.h"
#undef  __ITIMER_C__

static TIM_HandleTypeDef Tim3Handle;

void TIM3_IRQHandler(void);

void ITIMER3_Init(void)
{
    __HAL_RCC_TIM3_CLK_ENABLE();
    
    /* Set TIMx instance */
    /* TimRefClk = 72MHz */
    Tim3Handle.Instance = TIM3;
    Tim3Handle.Init.Period            = 100 - 1;
    Tim3Handle.Init.Prescaler         = 6000 - 1;
    Tim3Handle.Init.ClockDivision     = 0;
    Tim3Handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    Tim3Handle.Init.RepetitionCounter = 0;
    Tim3Handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&Tim3Handle);

    HAL_NVIC_SetPriority(TIM3_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY - 1, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    
    ITIMER3_Callback = NULL;
}

void ITIMER3_SetIsrPeriod(U8 freq)
{
    switch(freq)
    {
    case FREQ_50HZ : TIM3->PSC = 7200 - 1; break;
    case FREQ_60HZ : TIM3->PSC = 6000 - 1; break;
    }
}

void ITIMER3_IsrStart(void)
{
    Tim3Handle.Instance->CNT = 0;
    HAL_TIM_Base_Start_IT(&Tim3Handle); 
}

void ITIMER3_IsrStop(void)
{
    HAL_TIM_Base_Stop_IT(&Tim3Handle);
}

void TIM3_IRQHandler(void)
{
    if(__HAL_TIM_GET_FLAG(&Tim3Handle, TIM_FLAG_UPDATE) != RESET) {
        if(__HAL_TIM_GET_IT_SOURCE(&Tim3Handle, TIM_IT_UPDATE) !=RESET) {
            __HAL_TIM_CLEAR_IT(&Tim3Handle, TIM_IT_UPDATE);
            
            if(ITIMER3_Callback != 0)
              ITIMER3_Callback();
        }
    }
}
