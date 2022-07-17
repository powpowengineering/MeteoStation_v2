/******************************************************************************
 * ������:      program.c
 * �����:       Celeron (c) 2017
 * ����������:  ��������������� ������ ��������� ������ ������ (�������� ��� ������ � EEPROM)
 ******************************************************************************/


#include "stm32f1xx_hal.h"          // ���������� HAL API
#include "eeprom_page.h"            // ���������� EEPROM emulation (����������������� ������)
#include "program.h"                // ��������� ��������� ������� � �����



//============================================================================
// ������
//============================================================================


// ������ ������� ��������� (������ ����)
//  ��������: ������� ������ ���� ��� ������� �������� EEPAGE_CHUNK_SIZE ����!
//  ���������: ������ ��� ������ ����������� ����� �������� +1 ���, ����� ��������������� ���������� ��� ������� � ��������� PROGRAM_STEPS_COUNT, ����...
volatile static uint16_t ProgramData[(PROGRAM_STEPS_COUNT+1)*2];


// ���������� �� ������� ��������� (����������������� ��� �������� � �������������� ���������)
volatile static TProgramStatistics Program_Statistics;


// ������ ��� ���������� ������� ������ ������� ���������: 
//  STEP  = ����� ����, ������� � �������!
//  PHASE = ���� 0-�����/1-�����
#define PROGRAM_DATA(STEP, PHASE)  (ProgramData[ (STEP-1)*2 +PHASE ])


//// �������� ������������ ������
////  ���: EEPAGE_CHUNK_SIZE=(FLASH_PAGE_SIZE-2)... ������! ��, ������ ������������ �������� sizeof � ���������� ��������������� :(
//#if  ((FLASH_PAGE_SIZE-2)/4 +1)*2*2 < (FLASH_PAGE_SIZE-2)
//#error "������: ������������ ������ �������� ��� ������ ���������!"
//#endif


// ����������� ��������������� ��������: ��������� ���������� ��������� � �������.
// ��. ������� �� "������������� ��"... http://ru-wiki.org/wiki/%D0%9F%D1%80%D0%B5%D0%BF%D1%80%D0%BE%D1%86%D0%B5%D1%81%D1%81%D0%BE%D1%80_%D0%A1%D0%B8
#define NUMBER_OF_ARRAY_ITEMS( array ) ( sizeof( array ) / sizeof( *(array) ) )




//============================================================================
// ����������
//============================================================================


//-------------------------------------
// �������������� ���������� �� ������� ���������
void Program_RefreshStatistics(void)
{
  // �������� ������������ ������
  //  ��������: ������ ������� ������ ���� ��� ������� EEPAGE_CHUNK_SIZE (FLASH_PAGE_SIZE-2) ����!
  //  ��� EEPAGE_CHUNK_SIZE = FLASH_PAGE_SIZE-2  (������, 2048-2 = 2046 ����, �.�. 1023 ����� �� uint16_t - ��� ������ ��� �������� 511 ������ �����, �� 2 ������)
  assert_param(sizeof(ProgramData) >= EEPAGE_CHUNK_SIZE);
  
  
  // ������������� ����������
  uint32_t sum  = 0;
  uint16_t step = 0;
  
  for(step=1; step <= PROGRAM_STEPS_COUNT 
              && PROGRAM_DATA(step,0) != PROGRAM_END 
              && PROGRAM_DATA(step,1) != PROGRAM_END; step++)
    sum += PROGRAM_DATA(step,0) + PROGRAM_DATA(step,1);
  
  Program_Statistics.TotalSec  = sum;       // ����� ����� (���)
  Program_Statistics.TotalStep = step-1;    // ����� ����� (��)
}



//-------------------------------------
// ������� ���������� �� ������� ���������
extern __inline TProgramStatistics Program_GetStatistics(void)
{
  return Program_Statistics;
}




//============================================================================
// ������ ������
//============================================================================


//-------------------------------------
// ��������� ������ ������� ��������� �� FLASH � ���, � ������������� ����������...
HAL_StatusTypeDef Program_ReloadData(void)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  // �������� ������������ ������
  //  ��������: ������ ������� ������ ���� ��� ������� EEPAGE_CHUNK_SIZE (FLASH_PAGE_SIZE-2) ����!
  //  ��� EEPAGE_CHUNK_SIZE = FLASH_PAGE_SIZE-2  (������, 2048-2 = 2046 ����, �.�. 1023 ����� �� uint16_t - ��� ������ ��� �������� 511 ������ �����, �� 2 ������)
  assert_param(sizeof(ProgramData) >= EEPAGE_CHUNK_SIZE);

  // ��������� ������ ��������� �� FLASH � ���
  //  ����������: ������� �������� �� ZERO-BASED �������: VirtAddress = [0..EEPAGE_CHUNKS_COUNT-1]
  status = EEPAGE_LoadData( Profile_GetProgram()-1, (void*) ProgramData );
  assert_param(status == HAL_OK);

  // � ������ ������ �������� ������� �� FLASH - ������ ��� ������ ������� "�������" �������
  if(status != HAL_OK)
    for(uint16_t i=0; i<NUMBER_OF_ARRAY_ITEMS(ProgramData); i++)
      ProgramData[i] = PROGRAM_END;

  // �������� ����������
  Program_RefreshStatistics();
  
  return status;
}



//-------------------------------------
// ��������� ������ ������� ��������� �� ��� �� FLASH...
HAL_StatusTypeDef Program_SaveData(void)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  // �������� ������������ ������
  //  ��������: ������ ������� ������ ���� ��� ������� EEPAGE_CHUNK_SIZE (FLASH_PAGE_SIZE-2) ����!
  //  ��� EEPAGE_CHUNK_SIZE = FLASH_PAGE_SIZE-2  (������, 2048-2 = 2046 ����, �.�. 1023 ����� �� uint16_t - ��� ������ ��� �������� 511 ������ �����, �� 2 ������)
  assert_param(sizeof(ProgramData) >= EEPAGE_CHUNK_SIZE);
  
  // �������� ����������
  Program_RefreshStatistics();

  status = HAL_FLASH_Unlock();
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;

  // ��������� ������ ��������� �� FLASH �� ���
  //  ����������: ������� �������� �� ZERO-BASED �������: VirtAddress = [0..EEPAGE_CHUNKS_COUNT-1]
  status = EEPAGE_SaveData( Profile_GetProgram()-1, (void*) ProgramData );
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;

  status = HAL_FLASH_Lock();
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  return status;
}




//============================================================================
// �������������� �������� �������
//============================================================================


//-------------------------------------
// �������� ������ ������ ������� ��������� (������ �� ���)
extern __inline uint16_t Program_GetData(uint16_t step, uint8_t phase)
{
  return PROGRAM_DATA(step, phase);
}



//-------------------------------------
// ��������� ������ ������ ������� ��������� (������ � ���)
extern __inline void Program_SetData(uint16_t step, uint8_t phase, uint16_t value)
{
  PROGRAM_DATA(step, phase) = value;
}



//-------------------------------------
// ���������������� ������ ������ ������� ��������� (������ � ���)
extern __inline void Program_IncData(uint16_t step, uint8_t phase, uint16_t value)
{
  PROGRAM_DATA(step, phase) += value;
}



//-------------------------------------
// ���������������� ������ ������ ������� ��������� (������ � ���)
extern __inline void Program_DecData(uint16_t step, uint8_t phase, uint16_t value)
{
  PROGRAM_DATA(step, phase) -= value;
}



