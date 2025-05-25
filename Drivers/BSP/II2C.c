#define __II2C_C__
    #include "II2C.h"
#undef  __II2C_C__

static void II2C1_GPIO_Init(void);
static void II2C_WP_H(void);
static void II2C_WP_L(void);

static I2C_HandleTypeDef I2CHandle;

void II2C_Init(void)
{   
    II2C1_GPIO_Init();    
    
    __HAL_RCC_I2C1_CLK_ENABLE();  

    I2CHandle.Instance                  = I2C1;
    I2CHandle.Init.ClockSpeed           = 10000;
    I2CHandle.Init.DutyCycle            = I2C_DUTYCYCLE_2;
    I2CHandle.Init.OwnAddress1          = 0;    //AT24Cxx_ADDR;
    I2CHandle.Init.AddressingMode       = I2C_ADDRESSINGMODE_7BIT;
    I2CHandle.Init.DualAddressMode      = I2C_DUALADDRESS_DISABLE;
    I2CHandle.Init.OwnAddress2          = 0;
    I2CHandle.Init.GeneralCallMode      = I2C_GENERALCALL_DISABLE;
    I2CHandle.Init.NoStretchMode        = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&I2CHandle);  
    
    __HAL_I2C_ENABLE(&I2CHandle);
    
    II2C_WP_L();
}

static void II2C1_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin     = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode    = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull    = GPIO_PULLUP;      //GPIO_NOPULL;
    GPIO_InitStruct.Speed   = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
    GPIO_InitStruct.Pin 	= GPIO_PIN_5;
    GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull 	= GPIO_PULLUP;
    GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void II2C_WP_H(void)    { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);   }
static void II2C_WP_L(void)    { HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); }

void II2C_WriteByte(U16 addr, U16 size, U8* pData)
{
    while((size-16)>0) 
    {
        while(HAL_I2C_Mem_Write(&I2CHandle, (U16)AT24Cxx_ADDR, (U16)addr, I2C_MEMADD_SIZE_8BIT, pData, (U16)16,1000)!= HAL_OK);
        size -= 16;
        pData += 16;
        addr += 16;
    }
    while(HAL_I2C_Mem_Write(&I2CHandle, (U16)AT24Cxx_ADDR, (U16)addr, I2C_MEMADD_SIZE_8BIT, pData, (U16)size,1000)!= HAL_OK);
}

void II2C_ReadByte(U16 addr, U16 size, U8* pData)
{
    U16 TimeOut;
  
    while((size-16)>0)
    {
        TimeOut = 0;
        while(HAL_I2C_Mem_Read(&I2CHandle, (U16)AT24Cxx_ADDR, (U16)addr, I2C_MEMADD_SIZE_8BIT, pData, (U16)16, 1000)!= HAL_OK && TimeOut < 10)
        {
            TimeOut++;
        }
        size -= 16;
        pData += 16;
        addr += 16;
    }
    TimeOut = 0;
    while(HAL_I2C_Mem_Read(&I2CHandle, (U16)AT24Cxx_ADDR, (U16)addr, I2C_MEMADD_SIZE_8BIT, pData, (U16)size, 1000)!= HAL_OK && TimeOut < 10)
    {
        TimeOut++;
    }
}
