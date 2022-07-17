/******************************************************************************
 * ������:      eeprom_page.c
 * �����:       Celeron (c) 2017
 * ����������:  ������� �������� EEPROM ������ �� ���������� FLASH.
 *              ������� ��������� ������: �����������, �.�. �������� ��������� �������.
 ******************************************************************************/
 
#include "stm32f1xx_hal.h"          // ���������� HAL API
#include "flash.h"                  // �������������� ������� ������ � FLASH �������
#include "eeprom_page.h"            // ��������� ��������� ������� � �����



//============================================================================
// �������� FLASH ������
//============================================================================

// ��������� ���������� ������ ��� ����������� ������: 
//  ��������: ��� ���������������� ����� �� ������� � ����������� Linker! ������� ��������� ����� �������� ������������ ������� ��� ��������� �� ���� ���������, ���� ��������� ������� � ���� �����...
//  �������, ����� <������� �����������> ���������� ������ - ��� EEPROM ������� �������� �������� � ����� ����� FLASH ������. 
//  ������, ����� ������� ������ ����������� �������� ��� EEPROM - ������� � ���������� Project\Linker(Target) ���������� �������� <������� � ���������������� FLASH ������>, ����������� � IROM (Internal ROM) ����������.
//  �����, ������� ���������, ��� �� ����������� ��������� �������, ����������������� ��� Custom BootLoader... ���, � STM32F103VE ��� ����� �������� <� ����� ������> - ������� �� �������.


// ���������� ������ �������� FLASH ������, ��������� ��� ������� ����������������:
//  ������, ��� 1K=0x400 (��� ���������� �����������������) ��� 2K=0x800 (��� ����������������� ������� ��������).
//  ������ ����� �� ���������� HAL (������ ��������� - �� ���� ������ �������� ST) = (uint16_t) FLASH_PAGE_SIZE


// ������: ���������������� STM32 XL-����� (� ������� ����� 512K FLASH) ����� ��� ����� ������ - ������� ���������.
#ifdef STM32F10X_XL
  #define EEPAGE_BANK_END    FLASH_BANK2_END
#else
  #define EEPAGE_BANK_END    FLASH_BANK1_END
#endif


// ��������� ������ ������� � ������
//  ����������: ������� ���������� ������� � ������ ��������� �����:  idx9.page0 idx9.page1  idx8.page0 idx8.page1  idx7.page0 idx7.page1  ...  idx0.page0 idx0.page1 
#define EEPAGE0_START_ADDRESS(CHUNK_IDX)   ((EEPAGE_BANK_END - EEPAGE_PRESERVED_TRAILING_PAGES * FLASH_PAGE_SIZE - 2*FLASH_PAGE_SIZE + 1) - (2 * FLASH_PAGE_SIZE) * (CHUNK_IDX))
#define EEPAGE0_END_ADDRESS  (CHUNK_IDX)   ((EEPAGE_BANK_END - EEPAGE_PRESERVED_TRAILING_PAGES * FLASH_PAGE_SIZE -   FLASH_PAGE_SIZE    ) - (2 * FLASH_PAGE_SIZE) * (CHUNK_IDX))
#define EEPAGE1_START_ADDRESS(CHUNK_IDX)   ((EEPAGE_BANK_END - EEPAGE_PRESERVED_TRAILING_PAGES * FLASH_PAGE_SIZE -   FLASH_PAGE_SIZE + 1) - (2 * FLASH_PAGE_SIZE) * (CHUNK_IDX))
#define EEPAGE1_END_ADDRESS  (CHUNK_IDX)   ((EEPAGE_BANK_END - EEPAGE_PRESERVED_TRAILING_PAGES * FLASH_PAGE_SIZE                        ) - (2 * FLASH_PAGE_SIZE) * (CHUNK_IDX))



//============================================================================
// ��������������� �������
//============================================================================


// �� "����� �����" �������� ������ �������� ������:
typedef enum  {
  eepageCLEAN       = ((uint16_t)0xFFFF),   // ����� ����� �������� �������� ������ (�������� �������������)
  eepageVALID       = ((uint16_t)0xEEEE),   // "������ ���������" - ������������� ����� ��������� ���������� �������� ������ (����������������) ������ � ��� ��������
  eepageINVALID     = ((uint16_t)0x0000),   // "������ ��������" - ������������� ������ ��������, ����� ��� ��� �������� ���������� ����� ������ � "������ ��������"...  
} TEEPageWorkflow;

// ���������: �������� � ����������� ������� (eepageINVALID) ���� �� ���������, �� ��������� �������� ������ � ����� �������� (eepageVALID). 
//  �� ������ � ��� ��� ��������� "�����������". ������, ��� ����� ��� ���� ������������ ��� "��������� �����" ��� �������������� ������ � "������ ��������", 
//  ���� ���������� ���� �� ������� ��� ���������������� ��������� - ��� ���� �����-�� ������ ����� ������������� "�� ��������� �����"...


// ������ "���������� ���������" �������� ������ (������ ��������� ����)
#define EEPAGE_HEADER_SIZE   sizeof(TEEPageWorkflow)



//-----------------------------------------------------------------------------
// �������� ������ ������, �������� EEPAGE_CHUNK_SIZE ����, �������� �� 2 ����� (uint16_t),
// ������ ������������ �� ��������� ������� � ������ source (����� ���� ��� FLASH, ��� � ���),
// � ��������������� � �������� (��������������, ��� ��� FLASH ������) ������� � ������ target.

static HAL_StatusTypeDef EEPAGE_CopyDataFromSourceToTarget_UINT16(uint32_t source, uint32_t target)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  for(uint16_t i = 0; i < (EEPAGE_CHUNK_SIZE / sizeof(uint16_t)); i++)
  {
    status = AN2594_ProgramHalfWord( target, FLASH_Read_UINT16(source) );
    assert_param(status == HAL_OK);    
    if(status != HAL_OK) return status;
    
    source += sizeof(uint16_t);
    target += sizeof(uint16_t);
  }
  
  return status;
}



//-----------------------------------------------------------------------------
// ����������� "���� �������" (������������ ������, ������� � ��������� �����, ������� ������)
// ����� ������ ������������ �� ��������� ������� � ������ DataSource (����� ���� ��� FLASH, ��� � ���),
// ��� ������ ��������� ������ ���������� ������ ������ ������ � ������ FLASH.

static HAL_StatusTypeDef EEPAGE_MoveDataToNewPage_UINT16(uint32_t OldInvalidPageBase, uint32_t NewValidPageBase, uint32_t DataSource)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  // ��������� ������ �������� ������ "����������" (�������� ��������, �� ������ ������ - ���� �� �����)
  status = AN2594_ProgramHalfWord( OldInvalidPageBase, eepageINVALID );
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // ������� ��������-��������
  status = AN2594_ErasePage(NewValidPageBase);
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // ��������� ����� �������� �������
  status = EEPAGE_CopyDataFromSourceToTarget_UINT16(DataSource /*OldInvalidPageBase + EEPAGE_HEADER_SIZE*/, NewValidPageBase + EEPAGE_HEADER_SIZE);
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // ��������� ����� �������� ������ "��������"
  status = AN2594_ProgramHalfWord( NewValidPageBase, eepageVALID );
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // ������� ��������-��������
  status = AN2594_ErasePage(OldInvalidPageBase);
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;  
  
  return status;
}



//-----------------------------------------------------------------------------
// ��������� "���� �������" �� ������������ ���������, 
// � �������� �� ������������ ��� �������������

static HAL_StatusTypeDef EEPAGE_FixPageIfNecessary(uint8_t VirtAddress)
{
  HAL_StatusTypeDef status = HAL_OK;

  // �������� ����������
  if(VirtAddress >= EEPAGE_CHUNKS_COUNT)
    return HAL_ERROR;
  
  
  // ������ ������� "���� �������"
  TEEPageWorkflow PageStatus0 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE0_START_ADDRESS(VirtAddress) );
  TEEPageWorkflow PageStatus1 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE1_START_ADDRESS(VirtAddress) );
  
  
  // ������� �������� ��������: ���� �� ������� ������ ���� "��������", � ������ "������".
  if( PageStatus0 == eepageVALID )
  {
    if( PageStatus1 != eepageCLEAN )
    {
      status = AN2594_ErasePage(EEPAGE1_START_ADDRESS(VirtAddress));
      assert_param(status == HAL_OK);
    }
    return status;
  }  
  if( PageStatus1 == eepageVALID )
  {
    if( PageStatus0 != eepageCLEAN )
    {
      status = AN2594_ErasePage(EEPAGE0_START_ADDRESS(VirtAddress));
      assert_param(status == HAL_OK);
    }
    return status;
  }  
  
  
  // ������� ���� ��� ������� ������: ���� ���� �� ������� �������� ��� "����������", � ������ ����� ����� ������ ������ (�������� "��������" - ����� �������� �� ��� ������� �� ������� ����)...
  if( PageStatus0 == eepageINVALID && 
      PageStatus1 != eepageINVALID )
  {
    // �������������� "��������� �����" ������ �� �������� 0->1
    status = EEPAGE_MoveDataToNewPage_UINT16(EEPAGE0_START_ADDRESS(VirtAddress), 
                                             EEPAGE1_START_ADDRESS(VirtAddress), 
                                             EEPAGE0_START_ADDRESS(VirtAddress) + EEPAGE_HEADER_SIZE);
    assert_param(status == HAL_OK);
    return status;
  }
  if( PageStatus0 != eepageINVALID && 
      PageStatus1 == eepageINVALID )
  {
    // �������������� "��������� �����" ������ �� �������� 1->0
    status = EEPAGE_MoveDataToNewPage_UINT16(EEPAGE1_START_ADDRESS(VirtAddress), 
                                             EEPAGE0_START_ADDRESS(VirtAddress), 
                                             EEPAGE1_START_ADDRESS(VirtAddress) + EEPAGE_HEADER_SIZE);
    assert_param(status == HAL_OK);
    return status;
  }  
  
  
//  // ���� ��� �������� ������ (��� ������ ��� ����� ������ �������), �� ���� �� ��� ����� ������������� �������������� ��� "��������" (������� � ������), � ������ ��������� "������".
//  if( PageStatus0 == eepageINVALID && 
//      PageStatus1 == eepageCLEAN )
//  {
//    status = AN2594_ProgramHalfWord( EEPAGE0_START_ADDRESS(VirtAddress), eepageVALID );
//    assert_param(status == HAL_OK);
//    return status;
//  }
  
  
  // �� ���� ������ ������� (����������� ���������� ����������� - ���-�� ���� ����� �� ���) ����������� �� �����, � ���� ��������������� ��� ��������.
  {
    // ������� ��� ��������
    status = AN2594_ErasePage(EEPAGE0_START_ADDRESS(VirtAddress));   //������: � ������, PAGE0 ����������� ������, � ����� �� ��� PAGE1...
    assert_param(status == HAL_OK);
    if(status != HAL_OK) return status;  

    status = AN2594_ErasePage(EEPAGE1_START_ADDRESS(VirtAddress));   //������: � ������, PAGE0 ����������� ������, � ����� �� ��� PAGE1...
    assert_param(status == HAL_OK);
    if(status != HAL_OK) return status;  

    // ��������� ������ �������� ������ "��������"
    status = AN2594_ProgramHalfWord( EEPAGE0_START_ADDRESS(VirtAddress), eepageVALID );
    assert_param(status == HAL_OK);
    if(status != HAL_OK) return status;
  }
  
  
  return status;
}




//============================================================================
// ������ API
//============================================================================

//  ���������� ��������� ���������� ��������: HAL_OK ��� HAL_ERROR, HAL_TIMEOUT.

//-----------------------------------------------------------------------------
// ������������� ������
HAL_StatusTypeDef EEPAGE_Init(void)
{
  HAL_StatusTypeDef status = HAL_OK;

  for(uint8_t VirtAddress = 0; VirtAddress < EEPAGE_CHUNKS_COUNT; VirtAddress++)
  {
    HAL_StatusTypeDef page_status = EEPAGE_FixPageIfNecessary(VirtAddress);
    assert_param(page_status == HAL_OK);

    if(page_status != HAL_OK)
      status = HAL_ERROR;
  }

  return status;
}



//-----------------------------------------------------------------------------
// �������� ������� ������ � ���
HAL_StatusTypeDef EEPAGE_LoadData(uint8_t VirtAddress, void* Data)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  // �������� ����������
  if(VirtAddress >= EEPAGE_CHUNKS_COUNT || Data == 0)
    return HAL_ERROR;   
  
  // ������ ��� �������� �� ��������� - �������� � ����������� ������
  status = EEPAGE_FixPageIfNecessary(VirtAddress);
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // ������ ������� "���� �������"
  TEEPageWorkflow PageStatus0 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE0_START_ADDRESS(VirtAddress) );
  TEEPageWorkflow PageStatus1 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE1_START_ADDRESS(VirtAddress) );
  
  // ����� �������� ��������
  uint32_t source = NULL;
  if( PageStatus0 == eepageVALID )
    source = EEPAGE0_START_ADDRESS(VirtAddress) + EEPAGE_HEADER_SIZE;
  else if( PageStatus1 == eepageVALID )
    source = EEPAGE1_START_ADDRESS(VirtAddress) + EEPAGE_HEADER_SIZE;
  else
    return HAL_ERROR;

  // ��������� ������
  __IO uint16_t* target = (uint16_t*) Data;
  for(uint16_t i = 0; i < (EEPAGE_CHUNK_SIZE / sizeof(uint16_t)); i++)
  {
    *target = FLASH_Read_UINT16(source);
    source += sizeof(uint16_t);
    target += 1;    //����������: �.�. target ��� �� ������������� ����������, �������� ����� ������ (��� source). �� target �������� ���������� ���� (uint16_t*), �� �� �������� "���������� ����������" - ��������� ������� ������ ����������������, ����� �� ������������ �� ��������� ����� +sizeof(uint16_t);
  }
  
  return status;
}



//-----------------------------------------------------------------------------
// ���������� ������� ������ �� FLASH
HAL_StatusTypeDef EEPAGE_SaveData(uint8_t VirtAddress, void* Data)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  // �������� ����������
  if(VirtAddress >= EEPAGE_CHUNKS_COUNT || Data == 0)
    return HAL_ERROR;   
  
  // ������ ��� �������� �� ��������� - �������� � ����������� ������
  status = EEPAGE_FixPageIfNecessary(VirtAddress);
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // ������ ������� "���� �������"
  TEEPageWorkflow PageStatus0 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE0_START_ADDRESS(VirtAddress) );
  TEEPageWorkflow PageStatus1 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE1_START_ADDRESS(VirtAddress) );
  
  // ����� �������� ��������
  uint32_t OldInvalidPageBase = NULL;
  uint32_t NewValidPageBase   = NULL;
  if( PageStatus0 == eepageVALID )
  {
    OldInvalidPageBase = EEPAGE0_START_ADDRESS(VirtAddress);
    NewValidPageBase   = EEPAGE1_START_ADDRESS(VirtAddress);
  }
  else if( PageStatus1 == eepageVALID )
  {
    OldInvalidPageBase = EEPAGE1_START_ADDRESS(VirtAddress);
    NewValidPageBase   = EEPAGE0_START_ADDRESS(VirtAddress);
  }
  else
    return HAL_ERROR;

  // ��������� ������
  status = EEPAGE_MoveDataToNewPage_UINT16(OldInvalidPageBase, NewValidPageBase, (uint32_t) Data);
  assert_param(status == HAL_OK);
  return status;
}


