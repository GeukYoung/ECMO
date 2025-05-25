#define __IUART_C__
    #include "IUART.h"
#undef  __IUART_C__

#define RS485A_TX_ENABLE()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET)
#define RS485A_RX_ENABLE()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET)

//#define RS485B_TX_ENABLE()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET)
//#define RS485B_RX_ENABLE()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET)

#define BUFF_SIZE 512

typedef struct Que
{
    U8 buff[BUFF_SIZE];
    U32 head;
    U32 tail;
    U32 size;
}Que_t;

static void UART1_Init(U8 baudrate,U8 parity, U8 stopbit);
static void UART2_Init(U8 baudrate,U8 parity, U8 stopbit);
static void UART3_Init(U8 baudrate,U8 parity, U8 stopbit);
static void UART4_Init(U8 baudrate,U8 parity, U8 stopbit);

static void GPIO_Init(void);

void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void UART4_IRQHandler(void);   // RS232 Add [Sprare] 2023.12.10

static void Que_Clear(Que_t* pQue);
static U8 Que_PutByte(Que_t* pQue, U8 data);
static U8 Que_GetByte(Que_t* pQue, U8 *data);
static U32 Que_GetSize(Que_t* pQue);

static UART_HandleTypeDef hUart1, hUart2, hUart3, hUart4;
static Que_t rxQue, txQue;
static Que_t rxQueA, txQueA;
//static Que_t rxQueB, txQueB;
static Que_t rxQueS1, txQueS1;  // RS232 Spare1
static Que_t rxQueS2, txQueS2;  // RS232 Spare2

//void IUART_Init(RS_t* rs, RS_t* rsA, RS_t* rsB)
//{
//    GPIO_Init();
//    
//    UART1_Init(rsA->baudrate, rsA->parity, rsA->stopbit);       // RS485 <-> SONOFLOW
//    UART2_Init(rsB->baudrate, rsB->parity, rsB->stopbit);       // RS232 <-> PC
//    UART3_Init(rs->baudrate, rs->parity, rs->stopbit);          // RS232 <-> LCD
//    
//    Que_Clear(&txQue);    // RS232 [LCD]
//    Que_Clear(&txQueA);
//    //Que_Clear(&txQueB);
//    Que_Clear(&txQueS1);  // RS232 [Spare1] 
//    Que_Clear(&txQueS2);  // RS232 [Spare2]
//}

void IUART_Init(RS_t* rs, RS_t* rsA, RS_t* rsS1, RS_t* rsS2)
{
    GPIO_Init();
    
    UART1_Init(rsA->baudrate, rsA->parity, rsA->stopbit);       // RS485 <-> SONOFLOW
    UART2_Init(rsS1->baudrate, rsS1->parity, rsS1->stopbit);    // RS232 <-> PC
    UART3_Init(rs->baudrate, rs->parity, rs->stopbit);          // RS232 <-> LCD
    UART4_Init(rsS2->baudrate, rsS2->parity, rsS2->stopbit);    // RS232 <-> PC
    
    Que_Clear(&txQue);    // RS232 [LCD]
    Que_Clear(&txQueA);
    Que_Clear(&txQueS1);  // RS232 [Spare1] 
    Que_Clear(&txQueS2);  // RS232 [Spare2]
}

void IUART1_RecvBufClear(void)  // RS485-SONOFLOW
{
    Que_Clear(&rxQueA);
}

void IUART2_RecvBufClear(void)  // RS232 - SPARE1
{
    Que_Clear(&rxQueS1);
}

void IUART3_RecvBufClear(void)  // RS232-LCD
{
    Que_Clear(&rxQue);
}
void IUART4_RecvBufClear(void)  // RS232 - SPARE2
{
    Que_Clear(&rxQueS2);
}

U8 IUART1_CheckRecvEnd(void)
{
    U8 ret = false;
    
    if(IUART1_RecvFlag)
    {
        if(IUART1_RecvTick == 0)
        {
            IUART1_RecvFlag = false;
            ret = true;
        }
        else
        {
            IUART1_RecvTick--;
        }
    }
    
    return ret;
}

U8 IUART2_CheckRecvEnd(void)
{
    U8 ret = false;
    
    if(IUART2_RecvFlag)
    {
        if(IUART2_RecvTick == 0)
        {
            IUART2_RecvFlag = false;
            ret = true;
        }
        else
        {
            IUART2_RecvTick--;
        }
    }
    
    return ret;
}

U8 IUART3_CheckRecvEnd(void)
{
    U8 ret = false;
    
    if(IUART3_RecvFlag)
    {
        if(IUART3_RecvTick == 0)
        {
            IUART3_RecvFlag = false;
            ret = true;
        }
        else
        {
            IUART3_RecvTick--;
        }
    }
    
    return ret;
}

U8 IUART4_CheckRecvEnd(void)
{
    U8 ret = false;
    
    if(IUART4_RecvFlag)
    {
        if(IUART4_RecvTick == 0)
        {
            IUART4_RecvFlag = false;
            ret = true;
        }
        else
        {
            IUART4_RecvTick--;
        }
    }
    
    return ret;
}

U32 IUART1_GetRecvSize(void)  // RS485 - SONOFLOW
{
    return Que_GetSize(&rxQueA);
}

U32 IUART2_GetRecvSize(void)  // RS232 - SPARE1
{
    return Que_GetSize(&rxQueS1);
}

U32 IUART3_GetRecvSize(void)  // RS232 - LCD
{
    return Que_GetSize(&rxQue);
}

U32 IUART4_GetRecvSize(void)  // RS232 - SPARE2
{
    return Que_GetSize(&rxQueS2);
}

void IUART1_WriteBytes(uint8_t* pData, uint16_t size)
{
    for(int i=0; i<size; i++)
    {
        Que_PutByte(&txQueA, pData[i]);
    }

    RS485A_TX_ENABLE();
    __HAL_UART_DISABLE_IT(&hUart1, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&hUart1, UART_IT_TXE); 
}

void IUART2_WriteBytes(uint8_t* pData, uint16_t size)
{
    for(int i=0; i<size; i++)
    {
        Que_PutByte(&txQueS1, pData[i]);
    }

    __HAL_UART_DISABLE_IT(&hUart2, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&hUart2, UART_IT_TXE); 
}

void IUART3_WriteBytes(uint8_t* pData, uint16_t size)
{
    for(int i=0; i<size; i++)
    {
        Que_PutByte(&txQue, pData[i]);
    }

    __HAL_UART_DISABLE_IT(&hUart3, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&hUart3, UART_IT_TXE); 
}

void IUART4_WriteBytes(uint8_t* pData, uint16_t size)
{
    for(int i=0; i<size; i++)
    {
        Que_PutByte(&txQueS2, pData[i]);
    }

    __HAL_UART_DISABLE_IT(&hUart4, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&hUart4, UART_IT_TXE); 
}

U8 IUART1_ReadByte(void)
{
    U8 data;
    
    Que_GetByte(&rxQueA, &data);
    
    return data;
}

U8 IUART2_ReadByte(void)
{
    U8 data;
    
    Que_GetByte(&rxQueS1, &data);
    
    return data;
}

U8 IUART3_ReadByte(void)
{
    U8 data;
    
    Que_GetByte(&rxQue, &data);
    
    return data;
}

U8 IUART4_ReadByte(void)
{
    U8 data;
    
    Que_GetByte(&rxQueS2, &data);
    
    return data;
}

static void UART1_Init(U8 baudrate,U8 parity, U8 stopbit)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    
    U32 _baudrate;
    U32 _parity;
    U32 _stopbit;
    
    switch(baudrate)
    {
    case BAUDRATE_9600   : _baudrate = 9600;   break; 
    case BAUDRATE_19200  : _baudrate = 19200;  break;
    case BAUDRATE_38400  : _baudrate = 38400;  break;
    case BAUDRATE_57600  : _baudrate = 57600;  break;
    case BAUDRATE_115200 : _baudrate = 115200; break;
    case BAUDRATE_230400 : _baudrate = 230400; break;
    default              : _baudrate = 115200;
    }
    
    switch(parity)
    {
    case PARITY_NONE  : _parity = UART_PARITY_NONE; break; 
    case PARITY_ODD   : _parity = UART_PARITY_EVEN; break;
    case PARITY_EVEN  : _parity = UART_PARITY_ODD;  break;
    default           : _parity = UART_PARITY_NONE;
    }
     
    switch(stopbit)
    {
    case STOPBIT_1   : _stopbit = UART_STOPBITS_1; break; 
    case STOPBIT_2   : _stopbit = UART_STOPBITS_2; break;
    default          : _stopbit = UART_STOPBITS_1;
    }

    hUart1.Instance        = USART1;

    hUart1.Init.BaudRate   = _baudrate;
    hUart1.Init.WordLength = UART_WORDLENGTH_8B;
    hUart1.Init.StopBits   = _stopbit;
    hUart1.Init.Parity     = _parity;
    hUart1.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    hUart1.Init.Mode       = UART_MODE_TX_RX;

    HAL_UART_Init(&hUart1);
    
    HAL_NVIC_SetPriority(USART1_IRQn, 0xF, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    RS485A_RX_ENABLE();
    __HAL_UART_ENABLE_IT(&hUart1, UART_IT_RXNE);
}

static void UART2_Init(U8 baudrate,U8 parity, U8 stopbit)
{
    __HAL_RCC_USART2_CLK_ENABLE();
    
    U32 _baudrate;
    U32 _parity;
    U32 _stopbit;
    
    switch(baudrate)
    {
    case BAUDRATE_9600   : _baudrate = 9600;   break; 
    case BAUDRATE_19200  : _baudrate = 19200;  break;
    case BAUDRATE_38400  : _baudrate = 38400;  break;
    case BAUDRATE_57600  : _baudrate = 57600;  break;
    case BAUDRATE_115200 : _baudrate = 115200; break;
    case BAUDRATE_230400 : _baudrate = 230400; break;
    default              : _baudrate = 115200;
    }
    
    switch(parity)
    {
    case PARITY_NONE  : _parity = UART_PARITY_NONE; break; 
    case PARITY_ODD   : _parity = UART_PARITY_EVEN; break;
    case PARITY_EVEN  : _parity = UART_PARITY_ODD;  break;
    default           : _parity = UART_PARITY_NONE;
    }
     
    switch(stopbit)
    {
    case STOPBIT_1   : _stopbit = UART_STOPBITS_1; break; 
    case STOPBIT_2   : _stopbit = UART_STOPBITS_2; break;
    default          : _stopbit = UART_STOPBITS_1;
    }

    hUart2.Instance        = USART2;

    hUart2.Init.BaudRate   = _baudrate;
    hUart2.Init.WordLength = UART_WORDLENGTH_8B;
    hUart2.Init.StopBits   = _stopbit;
    hUart2.Init.Parity     = _parity;
    hUart2.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    hUart2.Init.Mode       = UART_MODE_TX_RX;

    HAL_UART_Init(&hUart2);
    
    HAL_NVIC_SetPriority(USART2_IRQn, 0xF, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    __HAL_UART_ENABLE_IT(&hUart2, UART_IT_RXNE);
}

static void UART3_Init(U8 baudrate,U8 parity, U8 stopbit)
{
    __HAL_RCC_USART3_CLK_ENABLE();
    
    U32 _baudrate;
    U32 _parity;
    U32 _stopbit;
    
    switch(baudrate)
    {
    case BAUDRATE_9600   : _baudrate = 9600;   break; 
    case BAUDRATE_19200  : _baudrate = 19200;  break;
    case BAUDRATE_38400  : _baudrate = 38400;  break;
    case BAUDRATE_57600  : _baudrate = 57600;  break;
    case BAUDRATE_115200 : _baudrate = 115200; break;
    case BAUDRATE_230400 : _baudrate = 230400; break;
    default              : _baudrate = 115200;
    }
    
    switch(parity)
    {
    case PARITY_NONE  : _parity = UART_PARITY_NONE; break; 
    case PARITY_ODD   : _parity = UART_PARITY_EVEN; break;
    case PARITY_EVEN  : _parity = UART_PARITY_ODD;  break;
    default           : _parity = UART_PARITY_NONE;
    }
     
    switch(stopbit)
    {
    case STOPBIT_1   : _stopbit = UART_STOPBITS_1; break; 
    case STOPBIT_2   : _stopbit = UART_STOPBITS_2; break;
    default          : _stopbit = UART_STOPBITS_1;
    }

    hUart3.Instance        = USART3;

    hUart3.Init.BaudRate   = _baudrate;
    hUart3.Init.WordLength = UART_WORDLENGTH_8B;
    hUart3.Init.StopBits   = _stopbit;
    hUart3.Init.Parity     = _parity;
    hUart3.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    hUart3.Init.Mode       = UART_MODE_TX_RX;

    HAL_UART_Init(&hUart3);
    
    HAL_NVIC_SetPriority(USART3_IRQn, 0xF, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    __HAL_UART_ENABLE_IT(&hUart3, UART_IT_RXNE);
}

static void UART4_Init(U8 baudrate,U8 parity, U8 stopbit)
{
    __HAL_RCC_UART4_CLK_ENABLE();
    
    U32 _baudrate;
    U32 _parity;
    U32 _stopbit;
    
    switch(baudrate)
    {
    case BAUDRATE_9600   : _baudrate = 9600;   break; 
    case BAUDRATE_19200  : _baudrate = 19200;  break;
    case BAUDRATE_38400  : _baudrate = 38400;  break;
    case BAUDRATE_57600  : _baudrate = 57600;  break;
    case BAUDRATE_115200 : _baudrate = 115200; break;
    case BAUDRATE_230400 : _baudrate = 230400; break;
    default              : _baudrate = 115200;
    }
    
    switch(parity)
    {
    case PARITY_NONE  : _parity = UART_PARITY_NONE; break; 
    case PARITY_ODD   : _parity = UART_PARITY_EVEN; break;
    case PARITY_EVEN  : _parity = UART_PARITY_ODD;  break;
    default           : _parity = UART_PARITY_NONE;
    }
     
    switch(stopbit)
    {
    case STOPBIT_1   : _stopbit = UART_STOPBITS_1; break; 
    case STOPBIT_2   : _stopbit = UART_STOPBITS_2; break;
    default          : _stopbit = UART_STOPBITS_1;
    }

    hUart4.Instance        = UART4;

    hUart4.Init.BaudRate   = _baudrate;
    hUart4.Init.WordLength = UART_WORDLENGTH_8B;
    hUart4.Init.StopBits   = _stopbit;
    hUart4.Init.Parity     = _parity;
    hUart4.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    hUart4.Init.Mode       = UART_MODE_TX_RX;

    HAL_UART_Init(&hUart4);
    
    HAL_NVIC_SetPriority(UART4_IRQn, 0xF, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);

    __HAL_UART_ENABLE_IT(&hUart4, UART_IT_RXNE);
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitStruct.Pin         = GPIO_PIN_2 | GPIO_PIN_9;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull        = GPIO_NOPULL;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin         = GPIO_PIN_3 | GPIO_PIN_10;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_INPUT;
    GPIO_InitStruct.Pull        = GPIO_NOPULL;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin         = GPIO_PIN_1 | GPIO_PIN_8;
    GPIO_InitStruct.Mode        = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull        = GPIO_PULLUP;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin         = GPIO_PIN_10;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull        = GPIO_NOPULL;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin         = GPIO_PIN_11;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_INPUT;
    GPIO_InitStruct.Pull        = GPIO_NOPULL;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // RS232 Add [Sprare] 2023.12.10
    GPIO_InitStruct.Pin         = GPIO_PIN_10;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull        = GPIO_NOPULL;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin         = GPIO_PIN_11;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_INPUT;
    GPIO_InitStruct.Pull        = GPIO_NOPULL;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void USART1_IRQHandler(void)
{
    UART_HandleTypeDef* huart = &hUart1;
    U8 data;

    if(USART1->SR & UART_FLAG_ORE)
    {
        data = huart->Instance->DR;
    }

    if((huart->Instance->SR & UART_FLAG_RXNE) != RESET)
    {
        IUART1_RecvFlag = true;
        IUART1_RecvTick = TERMINATE_TICK_TIME;

        huart->Instance->SR &= ~UART_FLAG_RXNE;
        Que_PutByte(&rxQueA, huart->Instance->DR);
    }
    
    if((huart->Instance->SR & UART_FLAG_TXE) != RESET)
    {
        if(Que_GetByte(&txQueA, &data))
        {
            huart->Instance->DR = data;
        }
        else
        {
            __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
            __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
        }
    }
    
    if((huart->Instance->SR & UART_FLAG_TC) != RESET)
    {
        RS485A_RX_ENABLE();
        huart->Instance->SR &= ~UART_FLAG_TC;
        __HAL_UART_DISABLE_IT(huart, UART_IT_TC);
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
    }
}

void USART2_IRQHandler(void)
{
    UART_HandleTypeDef* huart = &hUart2;
    U8 data;

    if(USART2->SR & UART_FLAG_ORE)
    {
        data = huart->Instance->DR;
    }

    if((huart->Instance->SR & UART_FLAG_RXNE) != RESET)
    {
        IUART2_RecvFlag = true;
        IUART2_RecvTick = TERMINATE_TICK_TIME;

        huart->Instance->SR &= ~UART_FLAG_RXNE;
        Que_PutByte(&rxQueS1, huart->Instance->DR);
    }
    
    if((huart->Instance->SR & UART_FLAG_TXE) != RESET)
    {
        if(Que_GetByte(&txQueS1, &data))
        {
            huart->Instance->DR = data;
        }
        else
        {
            __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
            __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
        }
    }
    
    if((huart->Instance->SR & UART_FLAG_TC) != RESET)
    {
        huart->Instance->SR &= ~UART_FLAG_TC;
        __HAL_UART_DISABLE_IT(huart, UART_IT_TC);
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
    }
}

void USART3_IRQHandler(void)
{
    UART_HandleTypeDef* huart = &hUart3;
    U8 data;

    if(USART3->SR & UART_FLAG_ORE)
    {
        data = huart->Instance->DR;
    }

    if((huart->Instance->SR & UART_FLAG_RXNE) != RESET)
    {
        IUART3_RecvFlag = true;
        IUART3_RecvTick = TERMINATE_TICK_TIME;

        huart->Instance->SR &= ~UART_FLAG_RXNE;
        Que_PutByte(&rxQue, huart->Instance->DR);
    }
    
    if((huart->Instance->SR & UART_FLAG_TXE) != RESET)
    {
        if(Que_GetByte(&txQue,&data))
        {
            huart->Instance->DR = data;
        }
        else
        {
            __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
            __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
        }
    }
    
    if((huart->Instance->SR & UART_FLAG_TC) != RESET)
    {
        huart->Instance->SR &= ~UART_FLAG_TC;
        __HAL_UART_DISABLE_IT(huart, UART_IT_TC);
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
    }
}

void UART4_IRQHandler(void)
{
    UART_HandleTypeDef* huart = &hUart4;
    U8 data;

    if(UART4->SR & UART_FLAG_ORE)
    {
        data = huart->Instance->DR;
    }

    if((huart->Instance->SR & UART_FLAG_RXNE) != RESET)
    {
        IUART4_RecvFlag = true;
        IUART4_RecvTick = TERMINATE_TICK_TIME;

        huart->Instance->SR &= ~UART_FLAG_RXNE;
        Que_PutByte(&rxQueS2, huart->Instance->DR);
    }
    
    if((huart->Instance->SR & UART_FLAG_TXE) != RESET)
    {
        if(Que_GetByte(&txQueS2, &data))
        {
            huart->Instance->DR = data;
        }
        else
        {
            __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
            __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
        }
    }
    
    if((huart->Instance->SR & UART_FLAG_TC) != RESET)
    {
        huart->Instance->SR &= ~UART_FLAG_TC;
        __HAL_UART_DISABLE_IT(huart, UART_IT_TC);
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
    }
}


static void Que_Clear(Que_t* pQue)
{
    pQue->head = pQue->tail = pQue->size = 0;
    memset(pQue->buff,0,BUFF_SIZE);
}

static U8 Que_PutByte(Que_t* pQue, U8 data)
{
    if(Que_GetSize(pQue) == (BUFF_SIZE-1)) 
      return FALSE;
    
    pQue->buff[pQue->head++] =data;
    pQue->head %= BUFF_SIZE;
    
    pQue->size = Que_GetSize(pQue);
    
    return TRUE;
}

static U8 Que_GetByte(Que_t* pQue, U8 *data)
{
    if(Que_GetSize(pQue) == 0) 
      return FALSE;
    
    *data = pQue->buff[pQue->tail++];
    pQue->tail %= BUFF_SIZE;
    
    pQue->size = Que_GetSize(pQue);
    
    return TRUE;
}

static U32 Que_GetSize(Que_t* pQue)
{
    return (pQue->head - pQue->tail  + BUFF_SIZE) % BUFF_SIZE;
}

