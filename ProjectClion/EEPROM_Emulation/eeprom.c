/**
  ******************************************************************************
  * @file    EEPROM/EEPROM_Emulation/src/eeprom.c
  * @author  MCD Application Team
  * @brief   This file provides all the EEPROM emulation firmware functions.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "W25Q_drv.h"
#include "eeprom.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Global variable used to store variable value in read sequence */
uint16_t DataVar = 0;

/* Virtual address defined by the user: 0xFFFF value is prohibited */
uint16_t VirtAddVarTab[NB_OF_VAR];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static HAL_StatusTypeDef EE_Format(void);
static uint16_t EE_FindValidPage(uint8_t Operation);
static uint16_t EE_VerifyPageFullWriteVariable(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_PageTransfer(uint16_t VirtAddress, uint16_t Data);
static uint16_t EE_VerifyPageFullyErased(uint32_t Address);
static HAL_StatusTypeDef FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data);
static HAL_StatusTypeDef FLASH_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError);

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
    HAL_StatusTypeDef  FlashStatus;
    uint32_t SectorError = 0;
    FLASH_EraseInitTypeDef pEraseInit;

    // Init Virtual address table
    VirtAddVarTab[0] = RECORD_MAN_VIR_ADR_LSB_LAST_RECORD;
    VirtAddVarTab[1] = RECORD_MAN_VIR_ADR_MSB_LAST_RECORD;

    VirtAddVarTab[2] = RECORD_MAN_VIR_ADR_LSB_NEXT_RECORD;
    VirtAddVarTab[3] = RECORD_MAN_VIR_ADR_MSB_NEXT_RECORD;

    VirtAddVarTab[4] = RECORD_MAN_VIR_ADR_LSB_QTY_RECORD;
    VirtAddVarTab[5] = RECORD_MAN_VIR_ADR_MSB_QTY_RECORD;

    /* Get Page0 status */
//    PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);
    W25Q_ReadData(PAGE0_BASE_ADDRESS, (uint8_t*)&PageStatus0, 2);
    /* Get Page1 status */
//    PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);
    W25Q_ReadData(PAGE1_BASE_ADDRESS, (uint8_t*)&PageStatus1, 2);


    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    pEraseInit.Banks = EEPROM_FLASH_BANK_LOCATION;
    pEraseInit.Page = PAGE0_ID;
    pEraseInit.NbPages = EEPROM_NUM_FLASH_PAGES_USE_IN_EE_PAGE;

    /* Check for invalid header states and repair if necessary */
    switch (PageStatus0)
    {
        case ERASED:
            if (PageStatus1 == VALID_PAGE) /* Page0 erased, Page1 valid */
            {
                /* Erase Page0 */
                if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
                {
                    FlashStatus = FLASH_Erase(&pEraseInit, &SectorError);
                    /* If erase operation was failed, a Flash error code is returned */
                    if (FlashStatus != HAL_OK)
                    {
                        return FlashStatus;
                    }
                }
            }
            else if (PageStatus1 == RECEIVE_DATA) /* Page0 erased, Page1 receive */
            {
                /* Erase Page0 */
                if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
                {
                    FlashStatus = FLASH_Erase(&pEraseInit, &SectorError);
                    /* If erase operation was failed, a Flash error code is returned */
                    if (FlashStatus != HAL_OK)
                    {
                        return FlashStatus;
                    }
                }
                /* Mark Page1 as valid */
                FlashStatus = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS,VALID_PAGE);
                /* If program operation was failed, a Flash error code is returned */
                if (FlashStatus != HAL_OK)
                {
                    return FlashStatus;
                }
            }
            else /* First EEPROM access (Page0&1 are erased) or invalid state -> format EEPROM */
            {
                /* Erase both Page0 and Page1 and set Page0 as valid page */
                FlashStatus = EE_Format();
                /* If erase/program operation was failed, a Flash error code is returned */
                if (FlashStatus != HAL_OK)
                {
                    return FlashStatus;
                }
            }
            break;

        case RECEIVE_DATA:
            if (PageStatus1 == VALID_PAGE) /* Page0 receive, Page1 valid */
            {
                /* Transfer data from Page1 to Page0 */
                for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++)
                {
                    uint16_t nHalfWord = 0;
                    W25Q_ReadData(PAGE0_BASE_ADDRESS + 6, (uint8_t*)&nHalfWord, 2);

//                    if (( *(__IO uint16_t*)(PAGE0_BASE_ADDRESS + 6)) == VirtAddVarTab[VarIdx])
                    if (nHalfWord == VirtAddVarTab[VarIdx])
                    {
                        x = VarIdx;
                    }
                    if (VarIdx != x)
                    {
                        /* Read the last variables' updates */
                        ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
                        /* In case variable corresponding to the virtual address was found */
                        if (ReadStatus != 0x1)
                        {
                            /* Transfer the variable to the Page0 */
                            EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
                            /* If program operation was failed, a Flash error code is returned */
                            if (EepromStatus != HAL_OK)
                            {
                                return EepromStatus;
                            }
                        }
                    }
                }
                /* Mark Page0 as valid */
                FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS,VALID_PAGE);
                /* If program operation was failed, a Flash error code is returned */
                if (FlashStatus != HAL_OK)
                {
                    return FlashStatus;
                }
                pEraseInit.Page = PAGE1_ID;
                pEraseInit.Banks = EEPROM_FLASH_BANK_LOCATION;
                pEraseInit.NbPages = EEPROM_NUM_FLASH_PAGES_USE_IN_EE_PAGE;
                /* Erase Page1 */
                if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
                {
                    FlashStatus = FLASH_Erase(&pEraseInit, &SectorError);
                    /* If erase operation was failed, a Flash error code is returned */
                    if (FlashStatus != HAL_OK)
                    {
                        return FlashStatus;
                    }
                }
            }
            else if (PageStatus1 == ERASED) /* Page0 receive, Page1 erased */
            {
                pEraseInit.Page = PAGE1_ID;
                pEraseInit.Banks = EEPROM_FLASH_BANK_LOCATION;
                pEraseInit.NbPages = EEPROM_NUM_FLASH_PAGES_USE_IN_EE_PAGE;
                /* Erase Page1 */
                if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
                {
                    FlashStatus = FLASH_Erase(&pEraseInit, &SectorError);
                    /* If erase operation was failed, a Flash error code is returned */
                    if (FlashStatus != HAL_OK)
                    {
                        return FlashStatus;
                    }
                }
                /* Mark Page0 as valid */
                FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS,VALID_PAGE);
                /* If program operation was failed, a Flash error code is returned */
                if (FlashStatus != HAL_OK)
                {
                    return FlashStatus;
                }
            }
            else /* Invalid state -> format eeprom */
            {
                /* Erase both Page0 and Page1 and set Page0 as valid page */
                FlashStatus = EE_Format();
                /* If erase/program operation was failed, a Flash error code is returned */
                if (FlashStatus != HAL_OK)
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
                if (FlashStatus != HAL_OK)
                {
                    return FlashStatus;
                }
            }
            else if (PageStatus1 == ERASED) /* Page0 valid, Page1 erased */
            {
                pEraseInit.Page = PAGE1_ID;
                pEraseInit.Banks = EEPROM_FLASH_BANK_LOCATION;
                pEraseInit.NbPages = EEPROM_NUM_FLASH_PAGES_USE_IN_EE_PAGE;
                /* Erase Page1 */
                if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
                {

                    FlashStatus = FLASH_Erase(&pEraseInit, &SectorError);
                    /* If erase operation was failed, a Flash error code is returned */
                    if (FlashStatus != HAL_OK)
                    {
                        return FlashStatus;
                    }
                }
            }
            else /* Page0 valid, Page1 receive */
            {
                /* Transfer data from Page0 to Page1 */
                for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++)
                {
                    uint16_t nHalfWord = 0;
                    W25Q_ReadData(PAGE0_BASE_ADDRESS + 6, (uint8_t*)&nHalfWord, 2);
//                    if ((*(__IO uint16_t*)(PAGE1_BASE_ADDRESS + 6)) == VirtAddVarTab[VarIdx])
                    if (nHalfWord == VirtAddVarTab[VarIdx])
                    {
                        x = VarIdx;
                    }
                    if (VarIdx != x)
                    {
                        /* Read the last variables' updates */
                        ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
                        /* In case variable corresponding to the virtual address was found */
                        if (ReadStatus != 0x1)
                        {
                            /* Transfer the variable to the Page1 */
                            EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
                            /* If program operation was failed, a Flash error code is returned */
                            if (EepromStatus != HAL_OK)
                            {
                                return EepromStatus;
                            }
                        }
                    }
                }
                /* Mark Page1 as valid */
                FlashStatus = FLASH_ProgramHalfWord(PAGE1_BASE_ADDRESS,VALID_PAGE);
                /* If program operation was failed, a Flash error code is returned */
                if (FlashStatus != HAL_OK)
                {
                    return FlashStatus;
                }
                pEraseInit.Page = PAGE0_ID;
                pEraseInit.Banks = EEPROM_FLASH_BANK_LOCATION;
                pEraseInit.NbPages = EEPROM_NUM_FLASH_PAGES_USE_IN_EE_PAGE;
                /* Erase Page0 */
                if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
                {

                    FlashStatus = FLASH_Erase(&pEraseInit, &SectorError);
                    /* If erase operation was failed, a Flash error code is returned */
                    if (FlashStatus != HAL_OK)
                    {
                        return FlashStatus;
                    }
                }
            }
            break;

        default:  /* Any other state -> format eeprom */
            /* Erase both Page0 and Page1 and set Page0 as valid page */
            FlashStatus = EE_Format();
            /* If erase/program operation was failed, a Flash error code is returned */
            if (FlashStatus != HAL_OK)
            {
                return FlashStatus;
            }
            break;
    }

    return HAL_OK;
}

/**
  * @brief  Verify if specified page is fully erased.
  * @param  Address: page address
  *   This parameter can be one of the following values:
  *     @arg PAGE0_BASE_ADDRESS: Page0 base address
  *     @arg PAGE1_BASE_ADDRESS: Page1 base address
  * @retval page fully erased status:
  *           - 0: if Page not erased
  *           - 1: if Page erased
  */
uint16_t EE_VerifyPageFullyErased(uint32_t Address)
{
    uint32_t ReadStatus = 1;
    uint16_t AddressValue = 0x5555;

    /* Check each active page address starting from end */
    while (Address <= PAGE0_END_ADDRESS)
    {
        /* Get the current location content to be compared with virtual address */
//        AddressValue = (*(__IO uint16_t*)Address);
        W25Q_ReadData(Address, (uint8_t*)&AddressValue, 2);


        /* Compare the read address with the virtual address */
        if (AddressValue != ERASED)
        {

            /* In case variable value is read, reset ReadStatus flag */
            ReadStatus = 0;

            break;
        }
        /* Next address location */
        Address = Address + 4;
    }

    /* Return ReadStatus value: (0: Page not erased, 1: Sector erased) */
    return ReadStatus;
}



STD_RESULT EE_ReadVariable32(uint32_t nVirtAdr,
                             uint32_t* pData)
{
    STD_RESULT enResult = RESULT_NOT_OK;
    uint16_t nTemp = 0U;

    // Read LSB data
    if (0U == EE_ReadVariable((uint16_t)nVirtAdr,
                              &nTemp))
    {
        *pData = nTemp;
        // Read MSB
        if (0U == EE_ReadVariable((uint16_t)(nVirtAdr >> 16U),
                                  &nTemp))
        {
            *pData |= ((uint32_t)nTemp) << 16U;
            enResult = RESULT_OK;
        }
        else
        {
            enResult = RESULT_NOT_OK;
        }
    }
    else
    {
        enResult = RESULT_NOT_OK;
    }

    return enResult;
}


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
    uint32_t Address = EEPROM_START_ADDRESS, PageStartAddress = EEPROM_START_ADDRESS;

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
//        AddressValue = (*(__IO uint16_t*)Address);
        W25Q_ReadData(Address, (uint8_t*)&AddressValue, 2);

        /* Compare the read address with the virtual address */
        if (AddressValue == VirtAddress)
        {
            /* Get content of Address-2 which is variable value */
//            *Data = (*(__IO uint16_t*)(Address - 2));
            W25Q_ReadData(Address - 2, (uint8_t*)Data, 2);

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

STD_RESULT EE_WriteVariable32(const uint32_t nVirAdr,
                              const uint32_t nData)
{
    STD_RESULT enResult = RESULT_NOT_OK;

    // Write LSB data in LSB address
    if (0U == EE_WriteVariable((uint16_t)nVirAdr,
                               (uint16_t)nData))
    {
        // Write MSB data in MSB address
        if (0U == EE_WriteVariable((uint16_t)(nVirAdr >> 16U),
                                   (uint16_t)(nData >> 16U)))
        {
            enResult = RESULT_OK;
        }
        else
        {
            enResult = RESULT_NOT_OK;
        }
    }
    else
    {
        enResult = RESULT_NOT_OK;
    }
    return enResult;
} // end of EE_WriteVariable32()


/**
  * @brief  Writes/updates variable data in EEPROM.
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

/**
  * @brief  Erases PAGE and PAGE1 and writes VALID_PAGE header to PAGE
  * @param  None
  * @retval Status of the last operation (Flash write or erase) done during
  *         EEPROM formatting
  */
static HAL_StatusTypeDef EE_Format(void)
{
    HAL_StatusTypeDef FlashStatus = HAL_OK;
    uint32_t SectorError = 0;
    FLASH_EraseInitTypeDef pEraseInit;

    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    pEraseInit.Page = PAGE0_ID;
    pEraseInit.Banks = EEPROM_FLASH_BANK_LOCATION;
    pEraseInit.NbPages = EEPROM_NUM_FLASH_PAGES_USE_IN_EE_PAGE;
    pEraseInit.NbPages = 1;

    /* Erase Page0 */
    if(!EE_VerifyPageFullyErased(PAGE0_BASE_ADDRESS))
    {
        FlashStatus = FLASH_Erase(&pEraseInit, &SectorError);
        /* If erase operation was failed, a Flash error code is returned */
        if (FlashStatus != HAL_OK)
        {
            return FlashStatus;
        }
    }
    /* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
    FlashStatus = FLASH_ProgramHalfWord(PAGE0_BASE_ADDRESS,VALID_PAGE);
    /* If program operation was failed, a Flash error code is returned */
    if (FlashStatus != HAL_OK)
    {
        return FlashStatus;
    }

    pEraseInit.Page = PAGE1_ID;
    /* Erase Page1 */
    if(!EE_VerifyPageFullyErased(PAGE1_BASE_ADDRESS))
    {
        FlashStatus = FLASH_Erase(&pEraseInit, &SectorError);
        /* If erase operation was failed, a Flash error code is returned */
        if (FlashStatus != HAL_OK)
        {
            return FlashStatus;
        }
    }

    return HAL_OK;
}

/**
  * @brief  Find valid Page for write or read operation
  * @param  Operation: operation to achieve on the valid page.
  *   This parameter can be one of the following values:
  *     @arg READ_FROM_VALID_PAGE: read operation from valid page
  *     @arg WRITE_IN_VALID_PAGE: write operation from valid page
  * @retval Valid page number (PAGE or PAGE1) or NO_VALID_PAGE in case
  *   of no valid page was found
  */
static uint16_t EE_FindValidPage(uint8_t Operation)
{
    uint16_t PageStatus0 = 6, PageStatus1 = 6;

    /* Get Page0 actual status */
//    PageStatus0 = (*(__IO uint16_t*)PAGE0_BASE_ADDRESS);
    W25Q_ReadData(PAGE0_BASE_ADDRESS, (uint8_t*)&PageStatus0, 2);

    /* Get Page1 actual status */
//    PageStatus1 = (*(__IO uint16_t*)PAGE1_BASE_ADDRESS);
    W25Q_ReadData(PAGE1_BASE_ADDRESS, (uint8_t*)&PageStatus1, 2);

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
    HAL_StatusTypeDef FlashStatus = HAL_OK;
    uint16_t ValidPage = PAGE0;
    uint32_t Address = EEPROM_START_ADDRESS, PageEndAddress = EEPROM_START_ADDRESS+PAGE_SIZE;

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
    PageEndAddress = (uint32_t)((EEPROM_START_ADDRESS - 1) + (uint32_t)((ValidPage + 1) * PAGE_SIZE));

    /* Check each active page address starting from beginning */
    while (Address < PageEndAddress)
    {
        uint32_t nWord = 0;
        W25Q_ReadData(Address, (uint8_t*)&nWord, 4);

        /* Verify if Address and Address+2 contents are 0xFFFFFFFF */
//        if ((*(__IO uint32_t*)Address) == 0xFFFFFFFF)
        if (nWord == 0xFFFFFFFF)
        {
            /* Set variable data */
            FlashStatus = FLASH_ProgramHalfWord(Address,Data);
            /* If program operation was failed, a Flash error code is returned */
            if (FlashStatus != HAL_OK)
            {
                return FlashStatus;
            }
            /* Set variable virtual address */
            FlashStatus = FLASH_ProgramHalfWord(Address + 2,VirtAddress);
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
    HAL_StatusTypeDef FlashStatus = HAL_OK;
    uint32_t NewPageAddress = EEPROM_START_ADDRESS;
    uint16_t OldPageId=0;
    uint16_t ValidPage = PAGE0, VarIdx = 0;
    uint16_t EepromStatus = 0, ReadStatus = 0;
    uint32_t SectorError = 0;
    FLASH_EraseInitTypeDef pEraseInit;

    /* Get active Page for read operation */
    ValidPage = EE_FindValidPage(READ_FROM_VALID_PAGE);

    if (ValidPage == PAGE1)       /* Page1 valid */
    {
        /* New page address where variable will be moved to */
        NewPageAddress = PAGE0_BASE_ADDRESS;

        /* Old page ID where variable will be taken from */
        OldPageId = PAGE1_ID;
    }
    else if (ValidPage == PAGE0)  /* Page0 valid */
    {
        /* New page address  where variable will be moved to */
        NewPageAddress = PAGE1_BASE_ADDRESS;

        /* Old page ID where variable will be taken from */
        OldPageId = PAGE0_ID;
    }
    else
    {
        return NO_VALID_PAGE;       /* No valid Page */
    }

    /* Set the new Page status to RECEIVE_DATA status */
    FlashStatus = FLASH_ProgramHalfWord(NewPageAddress,RECEIVE_DATA);
    /* If program operation was failed, a Flash error code is returned */
    if (FlashStatus != HAL_OK)
    {
        return FlashStatus;
    }

    /* Write the variable passed as parameter in the new active page */
    EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddress, Data);
    /* If program operation was failed, a Flash error code is returned */
    if (EepromStatus != HAL_OK)
    {
        return EepromStatus;
    }

    /* Transfer process: transfer variables from old to the new active page */
    for (VarIdx = 0; VarIdx < NB_OF_VAR; VarIdx++)
    {
        if (VirtAddVarTab[VarIdx] != VirtAddress)  /* Check each variable except the one passed as parameter */
        {
            /* Read the other last variable updates */
            ReadStatus = EE_ReadVariable(VirtAddVarTab[VarIdx], &DataVar);
            /* In case variable corresponding to the virtual address was found */
            if (ReadStatus != 0x1)
            {
                /* Transfer the variable to the new active page */
                EepromStatus = EE_VerifyPageFullWriteVariable(VirtAddVarTab[VarIdx], DataVar);
                /* If program operation was failed, a Flash error code is returned */
                if (EepromStatus != HAL_OK)
                {
                    return EepromStatus;
                }
            }
        }
    }

    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    pEraseInit.Page = OldPageId;
    pEraseInit.Banks = EEPROM_FLASH_BANK_LOCATION;
    pEraseInit.NbPages = EEPROM_NUM_FLASH_PAGES_USE_IN_EE_PAGE;


    /* Erase the old Page: Set old Page status to ERASED status */
    FlashStatus = FLASH_Erase(&pEraseInit, &SectorError);
    /* If erase operation was failed, a Flash error code is returned */
    if (FlashStatus != HAL_OK)
    {
        return FlashStatus;
    }

    /* Set new Page status to VALID_PAGE status */
    FlashStatus = FLASH_ProgramHalfWord(NewPageAddress,VALID_PAGE);
    /* If program operation was failed, a Flash error code is returned */
    if (FlashStatus != HAL_OK)
    {
        return FlashStatus;
    }

    /* Return last operation flash status */
    return FlashStatus;
}

static HAL_StatusTypeDef FLASH_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError)
{
    HAL_StatusTypeDef enResult = HAL_ERROR;
    uint32_t nAdr = 0U;

    if (pEraseInit->Page == 0U)
    {
        nAdr = PAGE0_BASE_ADDRESS;
    }
    else
    {
        nAdr = PAGE1_BASE_ADDRESS;
    }

    W25Q_UnLockGlobal();
    if (RESULT_OK == W25Q_EraseBlock(nAdr, W25Q_BLOCK_MEMORY_4KB))
    {
        enResult = HAL_OK;
    }
    else
    {
        enResult = HAL_ERROR;
    }


    return enResult;
}



static HAL_StatusTypeDef FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data)
{
    HAL_StatusTypeDef enResult = HAL_ERROR;
    uint8_t *pData = (uint8_t*)&Data;


    W25Q_UnLockGlobal();
    if (RESULT_OK == W25Q_WriteData(Address,pData, 2))
    {
        enResult = HAL_OK;
    }
    else
    {
        enResult = HAL_ERROR;
    }

    return enResult;
}
