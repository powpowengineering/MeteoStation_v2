/******************************************************************************
 * Модуль:      flash.c
 * Автор:       Celeron (c) 2017
 * Назначение:  Низкоуровневые функции работы с FLASH памятью
 ******************************************************************************/
 
#include "stm32f1xx_hal.h"          // подключаем HAL API
#include "flash.h"                  // прототипы локальных функций и типов



//============================================================================
// Методы API: обёртки стандартного HAL
//============================================================================


// Стереть несколько страниц FLASH подряд (адрес должен указывать на начало первой страницы)
extern __inline HAL_StatusTypeDef FLASH_ErasePages(uint32_t Page_Address, uint32_t NbPages)
{
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Page_Address));

  /**
  * @brief  Perform a mass erase or erase the specified FLASH memory pages
  * @note   To correctly run this function, the @ref HAL_FLASH_Unlock() function
  *         must be called before.
  *         Call the @ref HAL_FLASH_Lock() to disable the flash memory access 
  *         (recommended to protect the FLASH memory against possible unwanted operation)
  * @param[in]  pEraseInit pointer to an FLASH_EraseInitTypeDef structure that
  *         contains the configuration information for the erasing.
  *
  * @param[out]  PageError pointer to variable  that
  *         contains the configuration information on faulty page in case of error
  *         (0xFFFFFFFF means that all the pages have been correctly erased)
  *
  * @retval HAL_StatusTypeDef HAL Status
  */
  //HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError)

  FLASH_EraseInitTypeDef FLASH_EraseInitStruct;
  FLASH_EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;   // TypeErase: Mass erase or page erase. This parameter can be a value of @ref FLASHEx_Type_Erase
  FLASH_EraseInitStruct.PageAddress = Page_Address;            // PageAdress: Initial FLASH page address to erase when mass erase is disabled. This parameter must be a number between Min_Data = 0x08000000 and Max_Data = FLASH_BANKx_END (x = 1 or 2 depending on devices)
  FLASH_EraseInitStruct.NbPages     = NbPages;                 // NbPages: Number of pagess to be erased. This parameter must be a value between Min_Data = 1 and Max_Data = (max number of pages - value of initial page)

  uint32_t PageError;
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&FLASH_EraseInitStruct, &PageError);
  assert_param(PageError == 0xFFFFFFFF);
  
  return status;
}



//-----------------------------------------------------------------------------
// Записать байт (uint8_t) в FLASH
//  Внимание: минимальный адресуемый размер ячейки FLASH памяти в STM32 микроконтроллерах = 2 байта (полуслово 16 бит).
//  Побочным эффектом: по адресу (Address+1) будет записан второй байт=0x00, 
//  и этот старший байт полуслова уже не может быть использован для сохранения данных. 
//  Потому итерация адреса к следующей ячейке должна быть Address+=2, не смотря на то что Data 8-битная...
extern __inline HAL_StatusTypeDef FLASH_Program_UINT8(uint32_t Address, uint8_t Data)
{
  return FLASH_Program_UINT16(Address, (uint16_t) Data);
}



//-----------------------------------------------------------------------------
// Записать полуслово (uint16_t) в FLASH
extern __inline HAL_StatusTypeDef FLASH_Program_UINT16(uint32_t Address, uint16_t Data)
{
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Address));
 
  /**
    * @brief  Program halfword, word or double word at a specified address
    * @note   The function HAL_FLASH_Unlock() should be called before to unlock the FLASH interface
    *         The function HAL_FLASH_Lock() should be called after to lock the FLASH interface
    *
    * @note   If an erase and a program operations are requested simultaneously,    
    *         the erase operation is performed before the program one.
    *  
    * @note   FLASH should be previously erased before new programmation (only exception to this 
    *         is when 0x0000 is programmed)
    *
    * @param  TypeProgram:  Indicate the way to program at a specified address.
    *                       This parameter can be a value of @ref FLASH_Type_Program
    * @param  Address:      Specifies the address to be programmed.
    * @param  Data:         Specifies the data to be programmed
    * 
    * @retval HAL_StatusTypeDef HAL Status
    */
  //HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)  
  
  //FLASH_Type_Program
  //#define FLASH_TYPEPROGRAM_HALFWORD             0x01U  /*!<Program a half-word (16-bit) at a specified address.*/
  //#define FLASH_TYPEPROGRAM_WORD                 0x02U  /*!<Program a word (32-bit) at a specified address.*/
  //#define FLASH_TYPEPROGRAM_DOUBLEWORD           0x03U  /*!<Program a double word (64-bit) at a specified address*/

  HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Address, (uint64_t) Data);
  return status;
}



//-----------------------------------------------------------------------------
// Записать слово (uint32_t) в FLASH
extern __inline HAL_StatusTypeDef FLASH_Program_UINT32(uint32_t Address, uint32_t Data)
{
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Address));
 
  /**
    * @brief  Program halfword, word or double word at a specified address
    * @note   The function HAL_FLASH_Unlock() should be called before to unlock the FLASH interface
    *         The function HAL_FLASH_Lock() should be called after to lock the FLASH interface
    *
    * @note   If an erase and a program operations are requested simultaneously,    
    *         the erase operation is performed before the program one.
    *  
    * @note   FLASH should be previously erased before new programmation (only exception to this 
    *         is when 0x0000 is programmed)
    *
    * @param  TypeProgram:  Indicate the way to program at a specified address.
    *                       This parameter can be a value of @ref FLASH_Type_Program
    * @param  Address:      Specifies the address to be programmed.
    * @param  Data:         Specifies the data to be programmed
    * 
    * @retval HAL_StatusTypeDef HAL Status
    */
  //HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)  
  
  //FLASH_Type_Program
  //#define FLASH_TYPEPROGRAM_HALFWORD             0x01U  /*!<Program a half-word (16-bit) at a specified address.*/
  //#define FLASH_TYPEPROGRAM_WORD                 0x02U  /*!<Program a word (32-bit) at a specified address.*/
  //#define FLASH_TYPEPROGRAM_DOUBLEWORD           0x03U  /*!<Program a double word (64-bit) at a specified address*/

  HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, (uint64_t) Data);
  return status;
}



//-----------------------------------------------------------------------------
// Записать двойное слово (uint64_t) в FLASH
extern __inline HAL_StatusTypeDef FLASH_Program_UINT64(uint32_t Address, uint64_t Data)
{
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Address));
 
  /**
    * @brief  Program halfword, word or double word at a specified address
    * @note   The function HAL_FLASH_Unlock() should be called before to unlock the FLASH interface
    *         The function HAL_FLASH_Lock() should be called after to lock the FLASH interface
    *
    * @note   If an erase and a program operations are requested simultaneously,    
    *         the erase operation is performed before the program one.
    *  
    * @note   FLASH should be previously erased before new programmation (only exception to this 
    *         is when 0x0000 is programmed)
    *
    * @param  TypeProgram:  Indicate the way to program at a specified address.
    *                       This parameter can be a value of @ref FLASH_Type_Program
    * @param  Address:      Specifies the address to be programmed.
    * @param  Data:         Specifies the data to be programmed
    * 
    * @retval HAL_StatusTypeDef HAL Status
    */
  //HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)  
  
  //FLASH_Type_Program
  //#define FLASH_TYPEPROGRAM_HALFWORD             0x01U  /*!<Program a half-word (16-bit) at a specified address.*/
  //#define FLASH_TYPEPROGRAM_WORD                 0x02U  /*!<Program a word (32-bit) at a specified address.*/
  //#define FLASH_TYPEPROGRAM_DOUBLEWORD           0x03U  /*!<Program a double word (64-bit) at a specified address*/

  HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, (uint64_t) Data);
  return status;
}




//============================================================================
// Прочитать данные из FLASH (вспомогательные Getters для чисел стандартных целых типов)
//============================================================================

extern __inline uint8_t  FLASH_Read_UINT8 (uint32_t Address)
{
  return (*(__IO uint8_t*)Address);
}

extern __inline uint16_t FLASH_Read_UINT16(uint32_t Address)
{
  return (*(__IO uint16_t*)Address);
}

extern __inline uint32_t FLASH_Read_UINT32(uint32_t Address)
{
  return (*(__IO uint32_t*)Address);
}

extern __inline uint64_t FLASH_Read_UINT64(uint32_t Address)
{
  return (*(__IO uint64_t*)Address);
}



//============================================================================
// Методы из "AN2594: EEPROM emulation in STM32F10x microcontrollers"
//============================================================================
// COPYRIGHT 2009 STMicroelectronics


/**
  ******************************************************************************
  * @file    EEPROM_Emulation/src/eeprom.c 
  * @author  MCD Application Team
  * @version V3.1.0
  * @date    07/27/2009
  * @brief   This file provides all the EEPROM emulation firmware functions.
  ******************************************************************************
  */ 

  
//-----------------------------------------------------------------------------

// Flash Control Register bits
#define CR_PG_Set                ((uint32_t)0x00000001)
#define CR_PG_Reset              ((uint32_t)0x00001FFE) 
#define CR_PER_Set               ((uint32_t)0x00000002)
#define CR_PER_Reset             ((uint32_t)0x00001FFD)
#define CR_MER_Set               ((uint32_t)0x00000004)
#define CR_MER_Reset             ((uint32_t)0x00001FFB)
#define CR_OPTPG_Set             ((uint32_t)0x00000010)
#define CR_OPTPG_Reset           ((uint32_t)0x00001FEF)
#define CR_OPTER_Set             ((uint32_t)0x00000020)
#define CR_OPTER_Reset           ((uint32_t)0x00001FDF)
#define CR_STRT_Set              ((uint32_t)0x00000040)
#define CR_LOCK_Set              ((uint32_t)0x00000080)


// Delay definition
#define EraseTimeout          ((uint32_t)0x000B0000)
#define ProgramTimeout        ((uint32_t)0x00002000)


//-----------------------------------------------------------------------------

/**
  * @brief  Erases a specified FLASH page.
  * @note   This function can be used for all STM32F10x devices.
  * @param  Page_Address: The page address to be erased.
  * @retval HAL_OK или HAL_ERROR, HAL_TIMEOUT.
  */
HAL_StatusTypeDef AN2594_ErasePage(uint32_t Page_Address)
{
  HAL_StatusTypeDef status = HAL_OK;
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Page_Address));

#ifdef STM32F10X_XL
  if(Page_Address < FLASH_BANK1_END_ADDRESS)  
  {
    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastBank1Operation(EraseTimeout);
    if(status == HAL_OK)
    { 
      /* if the previous operation is completed, proceed to erase the page */
      FLASH->CR|= CR_PER_Set;
      FLASH->AR = Page_Address; 
      FLASH->CR|= CR_STRT_Set;
    
      /* Wait for last operation to be completed */
      status = FLASH_WaitForLastBank1Operation(EraseTimeout);

      /* Disable the PER Bit */
      FLASH->CR &= CR_PER_Reset;
    }
  }
  else
  {
    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastBank2Operation(EraseTimeout);
    if(status == HAL_OK)
    { 
      /* if the previous operation is completed, proceed to erase the page */
      FLASH->CR2|= CR_PER_Set;
      FLASH->AR2 = Page_Address; 
      FLASH->CR2|= CR_STRT_Set;
    
      /* Wait for last operation to be completed */
      status = FLASH_WaitForLastBank2Operation(EraseTimeout);
      
      /* Disable the PER Bit */
      FLASH->CR2 &= CR_PER_Reset;
    }
  }
#else
  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(EraseTimeout);
  if(status == HAL_OK)
  { 
    /* if the previous operation is completed, proceed to erase the page */
    FLASH->CR|= CR_PER_Set;
    FLASH->AR = Page_Address;
    FLASH->CR|= CR_STRT_Set;
    
    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(EraseTimeout);
    /* Disable the PER Bit */
    FLASH->CR &= CR_PER_Reset;
  }
#endif /* STM32F10X_XL */

  /* Return the Erase Status */
  return status;
}


//-----------------------------------------------------------------------------

/**
  * @brief  Programs a half word at a specified address.
  * @note   This function can be used for all STM32F10x devices.
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval HAL_OK или HAL_ERROR, HAL_TIMEOUT.
  */
HAL_StatusTypeDef AN2594_ProgramHalfWord(uint32_t Address, uint16_t Data)
{
  HAL_StatusTypeDef status = HAL_OK;
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Address));

#ifdef STM32F10X_XL
  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(ProgramTimeout);
  
  if(Address < FLASH_BANK1_END_ADDRESS)
  {
    if(status == HAL_OK)
    {
      /* if the previous operation is completed, proceed to program the new data */
      FLASH->CR |= CR_PG_Set;
  
      *(__IO uint16_t*)Address = Data;
      /* Wait for last operation to be completed */
      status = FLASH_WaitForLastBank1Operation(ProgramTimeout);

      /* Disable the PG Bit */
      FLASH->CR &= CR_PG_Reset;
    }
  }
  else
  {
    if(status == HAL_OK)
    {
      /* if the previous operation is completed, proceed to program the new data */
      FLASH->CR2 |= CR_PG_Set;
  
      *(__IO uint16_t*)Address = Data;
      /* Wait for last operation to be completed */
      status = FLASH_WaitForLastBank2Operation(ProgramTimeout);

      /* Disable the PG Bit */
      FLASH->CR2 &= CR_PG_Reset;
    }
  }
#else
  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(ProgramTimeout);
  
  if(status == HAL_OK)
  {
    /* if the previous operation is completed, proceed to program the new data */
    FLASH->CR |= CR_PG_Set;
  
    *(__IO uint16_t*)Address = Data;
    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(ProgramTimeout);
    
    /* Disable the PG Bit */
    FLASH->CR &= CR_PG_Reset;
  } 
#endif  /* STM32F10X_XL */
  
  /* Return the Program Status */
  return status;
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics ***********/




//============================================================================
// Дополнительно
//============================================================================

// Внимание: оказалось, что нельзя так просто стирать по одному биту в любой ячейке и в любом порядке (возникает ошибка во FLASH_WaitForLastOperation)! И не все последовательности перезаписи работают! 
// Фактически, единственная более-менее стабильно работающая последовательность перезаписи: 0xFFFF -> 0xEEEE -> 0x0000 (та что использована в AN2594 - наверное, инженеры ST что-то знают про низкоуровневые особенности своей FLASH-памяти?)
// Поэтому, эксперименты с FLASH памятью: поиск рабочей последовательности деградации статус-кодов...

HAL_StatusTypeDef FLASH_CheckRewriteSequence(void)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t address = 0x0807E000;

  assert_param(HAL_OK == (status = AN2594_ErasePage      (address)));
  assert_param(HAL_OK == (status = AN2594_ProgramHalfWord(address, 0xFFFF)));
  //assert_param(HAL_OK == (status = AN2594_ProgramHalfWord(address, 0xFFFE)));
  //assert_param(HAL_OK == (status = AN2594_ProgramHalfWord(address, 0xFFEE)));
  //assert_param(HAL_OK == (status = AN2594_ProgramHalfWord(address, 0xFEEE)));
  assert_param(HAL_OK == (status = AN2594_ProgramHalfWord(address, 0xEEEE)));
  //assert_param(HAL_OK == (status = AN2594_ProgramHalfWord(address, 0xEEE0)));
  //assert_param(HAL_OK == (status = AN2594_ProgramHalfWord(address, 0xEE00)));
  //assert_param(HAL_OK == (status = AN2594_ProgramHalfWord(address, 0xE000)));
  assert_param(HAL_OK == (status = AN2594_ProgramHalfWord(address, 0x0000)));
  
  // Проверялся концепт статус кодов:
  //  eepageCLEAN       = ((uint16_t)0xFFFF),
  //  eepageREADY       = ((uint16_t)0xEEEE), 
  //  eepagePROGRAMMING = ((uint16_t)0xCCCC), 
  //  eepageVALID       = ((uint16_t)0x8888), 
  //  eepageINVALID     = ((uint16_t)0x0000),
  
  return status;
}


