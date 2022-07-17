/**
  ******************************************************************************
  * @file    EEPROM_Emulation/src/eeprom.c 
  * @author  MCD Application Team
  * @version V3.1.0
  * @date    07/27/2009
  * @brief   This file provides all the EEPROM emulation firmware functions.
  ******************************************************************************
  */ 

/******************************************************************************
 * Модуль:      eeprom.c
 * Автор:       Celeron (c) 2017
 * Назначение:  Драйвер эмуляции EEPROM памяти во встроенном FLASH.
 *              Вариант выделения памяти: малыми виртуальными ячейками по 2байта (uint16_t).
 *              Идея и код взяты из "AN2594: EEPROM emulation in STM32F10x microcontrollers"
 ******************************************************************************/
 
#include "stm32f1xx_hal.h"          // подключаем HAL API
#include "eeprom.h"                 // прототипы локальных функций и типов



//============================================================================
// Определение данных
//============================================================================


// Global variable used to store variable value in read sequence 
static uint16_t DataVar = 0;

//FLASH_Status FlashStatus;


//---------------------------------------------------------------------------

// Определить размер страницы FLASH памяти, доступный для данного микроконтроллера:
//  Обычно, это 1K=0x400 (для слабеньких микроконтроллеров) или 2K=0x800 (для микроконтроллеров средней мощности).

// 1) Заморочиться с условными макроопределениями (с понтом на универсальность): 
//  Define the STM32F10Xxx Flash page size depending on the used STM32 device
//#if defined (STM32F10X_LD) || defined (STM32F10X_MD) || defined (STM32F10X_MD_VL) || defined (STM32F10X_LD_VL)
//  #define PAGE_SIZE  (uint16_t)0x400  /* Page size = 1KByte */
//#elif defined (STM32F10X_HD) || defined (STM32F10X_CL)
//  #define PAGE_SIZE  (uint16_t)0x800  /* Page size = 2KByte */
//#endif	

// 2) Прописать вручную: 
//  Note: Page size = 2KByte (microcontroller STM32F103VE)
//#define PAGE_SIZE  (uint16_t)0x800

// 3) Просто взять из библиотеки HAL (всегда корректно - за этим следят инженеры ST):
#define PAGE_SIZE  ((uint16_t)FLASH_PAGE_SIZE)


//---------------------------------------------------------------------------

// Разметить физическую память под сохраняемые данные: 
//  Внимание: эти макроопределения никак не связаны с настройками Linker! Поэтому последний может запросто распределить рабочий код программы на этих страницах, если программа большая и кода много...
//  Поэтому, чтобы <снизить вероятность> перекрытия памяти - под EEPROM следует выделять страницы в самом конце FLASH памяти. 
//  Однако, чтобы железно забить размеченные страницы под EEPROM - следует в настройках Project\Linker(Target) ограничить диапазон <имеющей в микроконтроллере FLASH памяти>, прописанный в IROM (Internal ROM) константах.
//  Также, следует убедиться, что не используете системные области, зарезервированные под Custom BootLoader... Вот, в STM32F103VE нет таких областей <в конце памяти> - поэтому не паримся.

// Исключение: Отступить от края FLASH, оставив несколько страниц памяти вконце нетронутыми (если они используются для других целей)
#define EEPROM_INDENT_PAGES_FROM_END      0

// EEPROM emulation start address in Flash: last 2 pages of available Flash memory (исключая зарезервированные страницы вконце памяти).
#ifdef FLASH_BANK2_END
  // Замечу: микроконтроллеры STM32 XL-серии (у которых более 512K FLASH) имеют два банка памяти - возьмем последний.
  #define EEPROM_START_ADDRESS    (FLASH_BANK2_END - 2*PAGE_SIZE - EEPROM_INDENT_PAGES_FROM_END*PAGE_SIZE + 1) 
#else
  #define EEPROM_START_ADDRESS    (FLASH_BANK1_END - 2*PAGE_SIZE - EEPROM_INDENT_PAGES_FROM_END*PAGE_SIZE + 1) 
#endif


//-----------------------------------------------------------------------------
// Какие-то макросы для реализации (говнокод из AN2594)

// Pages 0 and 1 base and end addresses 
#define PAGE0_BASE_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + 0x0000))
#define PAGE0_END_ADDRESS       ((uint32_t)(EEPROM_START_ADDRESS + (PAGE_SIZE - 1)))

#define PAGE1_BASE_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + PAGE_SIZE))
#define PAGE1_END_ADDRESS       ((uint32_t)(EEPROM_START_ADDRESS + (2 * PAGE_SIZE - 1)))

// Used Flash pages for EEPROM emulation
#define PAGE0                   ((uint16_t)0x0000)
#define PAGE1                   ((uint16_t)0x0001)

// No valid page define
#define NO_VALID_PAGE           ((uint16_t)0x00AB)

// Page status definitions
#define ERASED                  ((uint16_t)0xFFFF)     /* PAGE is empty */
#define RECEIVE_DATA            ((uint16_t)0xEEEE)     /* PAGE is marked to receive data */
#define VALID_PAGE              ((uint16_t)0x0000)     /* PAGE containing valid data */

// Valid pages in read and write defines
#define READ_FROM_VALID_PAGE    ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE     ((uint8_t)0x01)

// Page full define
#define PAGE_FULL               ((uint8_t)0x80)

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

// Exported macro
#define IS_FLASH_ADDRESS(ADDRESS) (((ADDRESS) >= 0x08000000) && ((ADDRESS) < 0x080FFFFF))



//============================================================================
// Реализация прикладных функций
//============================================================================
// COPYRIGHT 2009 STMicroelectronics


//---------------------------------------------------------------------------
// Forward declarations for private functions:

static FLASH_Status  EE_Format                     (void);
static uint16_t      EE_FindValidPage              (uint8_t Operation);
static uint16_t      EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data);
static uint16_t      EE_PageTransfer               (uint16_t VirtAddress, uint16_t Data);

static FLASH_Status  FLASH_ErasePage               (uint32_t Page_Address);
static FLASH_Status  FLASH_ProgramHalfWord         (uint32_t Address, uint16_t Data);



//-----------------------------------------------------------------------------

/**
  * @brief  Restore the pages to a known good state in case of page's status
  *   corruption after a power loss.
  * @param  None.
  * @retval - Flash error code: on write Flash error
  *         - FLASH_COMPLETE: on success
  */
uint16_t EE_Init(void)
{
  uint16_t PageStatus0 = 6, PageStatus1 = 6;
  uint16_t VarIdx = 0;
  uint16_t EepromStatus = 0, ReadStatus = 0;
  int16_t x = -1;
  uint16_t  FlashStatus;

  /* Get Page0 status */
  PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);
  /* Get Page1 status */
  PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

  /* Check for invalid header states and repair if necessary */
  switch (PageStatus0)
  {
    case ERASED:
      if (PageStatus1 == VALID_PAGE) /* Page0 erased, Page1 valid */
      {
        // BUG: Внимание! В данной говнокодной реализации EE_Init(), при каждом включении питания, впустую повторно стирается "теневая страница памяти", вырабатывая циклы перезаписи FLASH памяти (которых и так не много).

        /* Erase Page0 */
        FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS);
        /* If erase operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      else if (PageStatus1 == RECEIVE_DATA) /* Page0 erased, Page1 receive */
      {
        /* Erase Page0 */
        FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS);
        /* If erase operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
        /* Mark Page1 as valid */
        FlashStatus = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS, VALID_PAGE);
        /* If program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      else /* First EEPROM access (Page0&1 are erased) or invalid state -> format EEPROM */
      {
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        FlashStatus = EE_Format();
        /* If erase/program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      break;

    case RECEIVE_DATA:
      if (PageStatus1 == VALID_PAGE) /* Page0 receive, Page1 valid */
      {
        /* Transfer data from Page1 to Page0 */
        for (VarIdx = EEPROM_VIRTUAL_ADDRESS_MIN; VarIdx <= EEPROM_VIRTUAL_ADDRESS_MAX; VarIdx++)
        {
          if (( *(__IO uint16_t*)(PAGE0_BASE_ADDRESS + 6)) == VarIdx)
          {
            x = VarIdx;
          }
          if (VarIdx != x)
          {
            /* Read the last variables' updates */
            ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
            /* In case variable corresponding to the virtual address was found */
            if (ReadStatus != 0x1)
            {
              /* Transfer the variable to the Page0 */
              EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
              /* If program operation was failed, a Flash error code is returned */
              if (EepromStatus != FLASH_COMPLETE)
              {
                return EepromStatus;
              }
            }
          }
        }
        /* Mark Page0 as valid */
        FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE);
        /* If program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
        /* Erase Page1 */
        FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS);
        /* If erase operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      else if (PageStatus1 == ERASED) /* Page0 receive, Page1 erased */
      {
        /* Erase Page1 */
        FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS);
        /* If erase operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
        /* Mark Page0 as valid */
        FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE);
        /* If program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      else /* Invalid state -> format eeprom */
      {
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        FlashStatus = EE_Format();
        /* If erase/program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      break;

    case VALID_PAGE:
      if (PageStatus1 == VALID_PAGE) /* Invalid state -> format eeprom */
      {
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        FlashStatus = EE_Format();
        /* If erase/program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      else if (PageStatus1 == ERASED) /* Page0 valid, Page1 erased */
      {
        // BUG: Внимание! В данной говнокодной реализации EE_Init(), при каждом включении питания, впустую повторно стирается "теневая страница памяти", вырабатывая циклы перезаписи FLASH памяти (которых и так не много).
        
        /* Erase Page1 */
        FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS);
        /* If erase operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      else /* Page0 valid, Page1 receive */
      {
        /* Transfer data from Page0 to Page1 */
        for (VarIdx = EEPROM_VIRTUAL_ADDRESS_MIN; VarIdx <= EEPROM_VIRTUAL_ADDRESS_MAX; VarIdx++)
        {
          if ((*(__IO uint16_t*)(PAGE1_BASE_ADDRESS + 6)) == VarIdx)
          {
            x = VarIdx;
          }
          if (VarIdx != x)
          {
            /* Read the last variables' updates */
            ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
            /* In case variable corresponding to the virtual address was found */
            if (ReadStatus != 0x1)
            {
              /* Transfer the variable to the Page1 */
              EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
              /* If program operation was failed, a Flash error code is returned */
              if (EepromStatus != FLASH_COMPLETE)
              {
                return EepromStatus;
              }
            }
          }
        }
        /* Mark Page1 as valid */
        FlashStatus = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS, VALID_PAGE);
        /* If program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
        /* Erase Page0 */
        FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS);
        /* If erase operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE)
        {
          return FlashStatus;
        }
      }
      break;

    default:  /* Any other state -> format eeprom */
      /* Erase both Page0 and Page1 and set Page0 as valid page */
      FlashStatus = EE_Format();
      /* If erase/program operation was failed, a Flash error code is returned */
      if (FlashStatus != FLASH_COMPLETE)
      {
        return FlashStatus;
      }
      break;
  }

  return FLASH_COMPLETE;
}



//---------------------------------------------------------------------------

/**
  * @brief  Returns the last stored variable data, if found, which correspond to
  *   the passed virtual address
  * @param  VirtAddress: Variable virtual address
  * @param  Data: Global variable contains the read variable value
  * @retval Success or error status:
  *           - 0: if variable was found
  *           - 1: if the variable was not found
  *           - NO_VALID_PAGE: if no valid page was found.
  */
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data)
{
  uint16_t ValidPage = PAGE0;
  uint16_t AddressValue = 0x5555, ReadStatus = 1;
  uint32_t Address = 0x08010000, PageStartAddress = 0x08010000;

  /* Get active Page for read operation */
  ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

  /* Check if there is no valid page */
  if (ValidPage == NO_VALID_PAGE)
  {
    return  NO_VALID_PAGE;
  }

  /* Get the valid Page start Address */
  PageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE));

  /* Get the valid Page end Address */
  Address = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + ValidPage) * PAGE_SIZE));

  /* Check each active page address starting from end */
  while (Address > (PageStartAddress + 2))
  {
    /* Get the current location content to be compared with virtual address */
    AddressValue = (*(__IO uint16_t*)Address);

    /* Compare the read address with the virtual address */
    if (AddressValue == VirtAddress)
    {
      /* Get content of Address-2 which is variable value */
      *Data = (*(__IO uint16_t*)(Address - 2));

      /* In case variable value is read, reset ReadStatus flag */
      ReadStatus = 0;

      break;
    }
    else
    {
      /* Next address location */
      Address = Address - 4;
    }
  }

  /* Return ReadStatus value: (0: variable exist, 1: variable doesn't exist) */
  return ReadStatus;
}



//---------------------------------------------------------------------------

/**
  * @brief  Writes/upadtes variable data in EEPROM.
  * @param  VirtAddress: Variable virtual address
  * @param  Data: 16 bit data to be written
  * @retval Success or error status:
  *           - FLASH_COMPLETE: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data)
{
  uint16_t Status = 0;

  /* Write the variable virtual address and value in the EEPROM */
  Status = EE_VerifyPageFullWriteVariable(VirtAddress, Data);

  /* In case the EEPROM active page is full */
  if (Status == PAGE_FULL)
  {
    /* Perform Page transfer */
    Status = EE_PageTransfer(VirtAddress, Data);
  }

  /* Return last operation status */
  return Status;
}



//---------------------------------------------------------------------------

/**
  * @brief  Erases PAGE0 and PAGE1 and writes VALID_PAGE header to PAGE0
  * @param  None
  * @retval Status of the last operation (Flash write or erase) done during
  *         EEPROM formating
  */
static FLASH_Status EE_Format(void)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;

  /* Erase Page0 */
  FlashStatus = FLASH_ErasePage(PAGE0_BASE_ADDRESS);

  /* If erase operation was failed, a Flash error code is returned */
  if (FlashStatus != FLASH_COMPLETE)
  {
    return FlashStatus;
  }
  /* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
  FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS, VALID_PAGE);

  /* If program operation was failed, a Flash error code is returned */
  if (FlashStatus != FLASH_COMPLETE)
  {
    return FlashStatus;
  }
  /* Erase Page1 */
  FlashStatus = FLASH_ErasePage(PAGE1_BASE_ADDRESS);

  /* Return Page1 erase operation status */
  return FlashStatus;
}



//---------------------------------------------------------------------------

/**
  * @brief  Find valid Page for write or read operation
  * @param  Operation: operation to achieve on the valid page.
  *   This parameter can be one of the following values:
  *     @arg READ_FROM_VALID_PAGE: read operation from valid page
  *     @arg WRITE_IN_VALID_PAGE: write operation from valid page
  * @retval Valid page number (PAGE0 or PAGE1) or NO_VALID_PAGE in case
  *   of no valid page was found
  */
static uint16_t EE_FindValidPage(uint8_t Operation)
{
  uint16_t PageStatus0 = 6, PageStatus1 = 6;

  /* Get Page0 actual status */
  PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);

  /* Get Page1 actual status */
  PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);

  /* Write or read operation */
  switch (Operation)
  {
    case WRITE_IN_VALID_PAGE:   /* ---- Write operation ---- */
      if (PageStatus1 == VALID_PAGE)
      {
        /* Page0 receiving data */
        if (PageStatus0 == RECEIVE_DATA)
        {
          return PAGE0;         /* Page0 valid */
        }
        else
        {
          return PAGE1;         /* Page1 valid */
        }
      }
      else if (PageStatus0 == VALID_PAGE)
      {
        /* Page1 receiving data */
        if (PageStatus1 == RECEIVE_DATA)
        {
          return PAGE1;         /* Page1 valid */
        }
        else
        {
          return PAGE0;         /* Page0 valid */
        }
      }
      else
      {
        return NO_VALID_PAGE;   /* No valid Page */
      }

    case READ_FROM_VALID_PAGE:  /* ---- Read operation ---- */
      if (PageStatus0 == VALID_PAGE)
      {
        return PAGE0;           /* Page0 valid */
      }
      else if (PageStatus1 == VALID_PAGE)
      {
        return PAGE1;           /* Page1 valid */
      }
      else
      {
        return NO_VALID_PAGE ;  /* No valid Page */
      }

    default:
      return PAGE0;             /* Page0 valid */
  }
}



//---------------------------------------------------------------------------

/**
  * @brief  Verify if active page is full and Writes variable in EEPROM.
  * @param  VirtAddress: 16 bit virtual address of the variable
  * @param  Data: 16 bit data to be written as variable value
  * @retval Success or error status:
  *           - FLASH_COMPLETE: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;
  uint16_t ValidPage = PAGE0;
  uint32_t Address = 0x08010000, PageEndAddress = 0x080107FF;

  /* Get valid Page for write operation */
  ValidPage = EE_FindValidPage(WRITE_IN_VALID_PAGE);

  /* Check if there is no valid page */
  if (ValidPage == NO_VALID_PAGE)
  {
    return  NO_VALID_PAGE;
  }

  /* Get the valid Page start Address */
  Address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * PAGE_SIZE));

  /* Get the valid Page end Address */
  PageEndAddress = (uint32_t)((EEPROM_START_ADDRESS - 2) + (uint32_t)((1 + ValidPage) * PAGE_SIZE));

  /* Check each active page address starting from begining */
  while (Address < PageEndAddress)
  {
    /* Verify if Address and Address+2 contents are 0xFFFFFFFF */
    if ((*(__IO uint32_t*)Address) == 0xFFFFFFFF)
    {
      /* Set variable data */
      FlashStatus = FLASH_ProgramHalfWord(Address, Data);
      /* If program operation was failed, a Flash error code is returned */
      if (FlashStatus != FLASH_COMPLETE)
      {
        return FlashStatus;
      }
      /* Set variable virtual address */
      FlashStatus = FLASH_ProgramHalfWord(Address + 2, VirtAddress);
      /* Return program operation status */
      return FlashStatus;
    }
    else
    {
      /* Next address location */
      Address = Address + 4;
    }
  }

  /* Return PAGE_FULL in case the valid page is full */
  return PAGE_FULL;
}



//---------------------------------------------------------------------------

/**
  * @brief  Transfers last updated variables data from the full Page to
  *   an empty one.
  * @param  VirtAddress: 16 bit virtual address of the variable
  * @param  Data: 16 bit data to be written as variable value
  * @retval Success or error status:
  *           - FLASH_COMPLETE: on success
  *           - PAGE_FULL: if valid page is full
  *           - NO_VALID_PAGE: if no valid page was found
  *           - Flash error code: on write Flash error
  */
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data)
{
  FLASH_Status FlashStatus = FLASH_COMPLETE;
  uint32_t NewPageAddress = 0x080103FF, OldPageAddress = 0x08010000;
  uint16_t ValidPage = PAGE0, VarIdx = 0;
  uint16_t EepromStatus = 0, ReadStatus = 0;

  /* Get active Page for read operation */
  ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

  if (ValidPage == PAGE1)       /* Page1 valid */
  {
    /* New page address where variable will be moved to */
    NewPageAddress = PAGE0_BASE_ADDRESS;

    /* Old page address where variable will be taken from */
    OldPageAddress = PAGE1_BASE_ADDRESS;
  }
  else if (ValidPage == PAGE0)  /* Page0 valid */
  {
    /* New page address where variable will be moved to */
    NewPageAddress = PAGE1_BASE_ADDRESS;

    /* Old page address where variable will be taken from */
    OldPageAddress = PAGE0_BASE_ADDRESS;
  }
  else
  {
    return NO_VALID_PAGE;       /* No valid Page */
  }

  // TODO: вот здесь, стоит предварительно стереть страницу-приемник...
/*
  // Erase New Page
  FlashStatus = FLASH_ErasePage(NewPageAddress);
  // If erase operation was failed, a Flash error code is returned 
  if (FlashStatus != FLASH_COMPLETE)
  {
    return FlashStatus;
  }
*/
  
  /* Set the new Page status to RECEIVE_DATA status */
  FlashStatus = FLASH_ProgramHalfWord(NewPageAddress, RECEIVE_DATA);
  /* If program operation was failed, a Flash error code is returned */
  if (FlashStatus != FLASH_COMPLETE)
  {
    return FlashStatus;
  }

  /* Write the variable passed as parameter in the new active page */
  EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddress, Data);
  /* If program operation was failed, a Flash error code is returned */
  if (EepromStatus != FLASH_COMPLETE)
  {
    return EepromStatus;
  }

  /* Transfer process: transfer variables from old to the new active page */
  for (VarIdx = EEPROM_VIRTUAL_ADDRESS_MIN; VarIdx <= EEPROM_VIRTUAL_ADDRESS_MAX; VarIdx++)
  {
    if (VarIdx != VirtAddress)  /* Check each variable except the one passed as parameter */
    {
      /* Read the other last variable updates */
      ReadStatus = EE_ReadVariable(VarIdx, &DataVar);
      /* In case variable corresponding to the virtual address was found */
      if (ReadStatus != 0x1)
      {
        /* Transfer the variable to the new active page */
        EepromStatus = EE_VerifyPageFullWriteVariable(VarIdx, DataVar);
        /* If program operation was failed, a Flash error code is returned */
        if (EepromStatus != FLASH_COMPLETE)
        {
          return EepromStatus;
        }
      }
    }
  }

  /* Erase the old Page: Set old Page status to ERASED status */
  FlashStatus = FLASH_ErasePage(OldPageAddress);
  /* If erase operation was failed, a Flash error code is returned */
  if (FlashStatus != FLASH_COMPLETE)
  {
    return FlashStatus;
  }

  /* Set new Page status to VALID_PAGE status */
  FlashStatus = FLASH_ProgramHalfWord(NewPageAddress, VALID_PAGE);
  /* If program operation was failed, a Flash error code is returned */
  if (FlashStatus != FLASH_COMPLETE)
  {
    return FlashStatus;
  }

  /* Return last operation flash status */
  return FlashStatus;
}



//============================================================================
// Низкоуровневые функции (работа с FLASH)
//============================================================================
// COPYRIGHT 2009 STMicroelectronics

/**
  * @brief  Erases a specified FLASH page.
  * @note   This function can be used for all STM32F10x devices.
  * @param  Page_Address: The page address to be erased.
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static FLASH_Status FLASH_ErasePage(uint32_t Page_Address)
{
  FLASH_Status status = FLASH_COMPLETE;
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Page_Address));

#ifdef STM32F10X_XL
  if(Page_Address < FLASH_BANK1_END_ADDRESS)  
  {
    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastBank1Operation(EraseTimeout);
    if(status == FLASH_COMPLETE)
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
    if(status == FLASH_COMPLETE)
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
  status = (FLASH_Status) FLASH_WaitForLastOperation(EraseTimeout);
  if(status == FLASH_COMPLETE)
  { 
    /* if the previous operation is completed, proceed to erase the page */
    FLASH->CR|= CR_PER_Set;
    FLASH->AR = Page_Address;
    FLASH->CR|= CR_STRT_Set;
    
    /* Wait for last operation to be completed */
    status = (FLASH_Status) FLASH_WaitForLastOperation(EraseTimeout);
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
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT. 
  */
static FLASH_Status FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;
  /* Check the parameters */
  assert_param(IS_FLASH_ADDRESS(Address));

#ifdef STM32F10X_XL
  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(ProgramTimeout);
  
  if(Address < FLASH_BANK1_END_ADDRESS)
  {
    if(status == FLASH_COMPLETE)
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
    if(status == FLASH_COMPLETE)
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
  status = (FLASH_Status) FLASH_WaitForLastOperation(ProgramTimeout);
  
  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to program the new data */
    FLASH->CR |= CR_PG_Set;
  
    *(__IO uint16_t*)Address = Data;
    /* Wait for last operation to be completed */
    status = (FLASH_Status) FLASH_WaitForLastOperation(ProgramTimeout);
    
    /* Disable the PG Bit */
    FLASH->CR &= CR_PG_Reset;
  } 
#endif  /* STM32F10X_XL */
  
  /* Return the Program Status */
  return status;
}


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
