#define __FLASH_IF_C
    #include "flash_if.h"
#undef  __FLASH_IF_C

static void FLASH_If_Init(void);

static void FLASH_If_Init(void)
{
    /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  /* Clear all FLASH flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
  /* Unlock the Program memory */
  HAL_FLASH_Lock();
}

U32 FLASH_If_ParamAreaErase(U32 start)
{
  uint32_t PageError = 0;
  FLASH_EraseInitTypeDef pEraseInit;
  HAL_StatusTypeDef status = HAL_OK;

  /* Unlock the Flash to enable the flash control register access *************/ 
  HAL_FLASH_Unlock();


  pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  pEraseInit.PageAddress = start;
  pEraseInit.Banks = FLASH_BANK_1;
  pEraseInit.NbPages = 1;
  status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  if (status != HAL_OK)
  {
    /* Error occurred while page erase */
    return FLASHIF_ERASE_FAIL;
  }

  return FLASHIF_OK;
}
/**
  * @brief  This function does an erase of all user flash area
  * @param  StartSector: start of user flash area
  * @retval 0: user flash area successfully erased
  *         1: error occurred
  */
U32 FLASH_If_AppAreaErase(U32 start)
{
  uint32_t NbrOfPages = 0;
  uint32_t PageError = 0;
  FLASH_EraseInitTypeDef pEraseInit;
  HAL_StatusTypeDef status = HAL_OK;

  /* Unlock the Flash to enable the flash control register access *************/ 
  HAL_FLASH_Unlock();

  /* Get the sector where start the user flash area */
  NbrOfPages = (END_FLASH_ADDR - start)/FLASH_PAGE_SIZE;

  pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  pEraseInit.PageAddress = start;
  pEraseInit.Banks = FLASH_BANK_1;
  pEraseInit.NbPages = NbrOfPages;
  status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  if (status != HAL_OK)
  {
    /* Error occurred while page erase */
    return FLASHIF_ERASE_FAIL;
  }

  return FLASHIF_OK;
}

/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  FlashAddress: start address for writing data buffer
  * @param  Data: pointer on data buffer
  * @param  DataLength: length of data buffer (unit is 32-bit word)   
  * @retval 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */

U32 FLASH_If_WriteWord(U32 FlashAddress, U32 size, U16 *Data)
{   
U32 i = 0;
U16 fData;
U32 newSize;

  newSize = size/2 + size%2;
  
  HAL_FLASH_Unlock();
  
  for (i = 0; i < newSize; i++)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashAddress, Data[i]) == HAL_OK)      
    {
      fData = *(U16*)FlashAddress;
       /* Check the written value */
      if (fData != Data[i])
      {
        /* Flash content doesn't match SRAM content */
        return(FLASHIF_WRITING_AND_READ_NOT_MATCH);
      }
      /* Increment FLASH destination address */
      FlashAddress += 2;
    }
    else
    {
      /* Error occurred while writing data in Flash memory */
      return (FLASHIF_WRITING_ERROR);
    }
  }
  
  HAL_FLASH_Lock();
  
  return (FLASHIF_OK);
}


void FLASH_If_ReadWord(U32 FlashAddress, U32 size, U8 *Data)
{
    U32 i;

    if(FLASH_WaitForLastOperation((U32)50000) == HAL_OK)
    {
        for(i=0; i<size; i++)
        {
             Data[i] =  *(U8*)(FlashAddress+i);
        }
    }
}    
