/******************************************************************************
 * ������:      profile.c
 * �����:       Celeron (c) 2015
 * ����������:  ������ ������������� �������� eeprom.c
 *              ������ �������� ������ "Getters" � "Setters" for Virtual variables.
 *              ����� ������ ������� ������� ������������ �� ����������� ����, ��� ��������������� �������������� ������ �� eeprom.c...
 *              Disclaimer: ���� ������ ���� �� ��������� (��������) ������� � ���������� ��� ������ �� ����������� - ������� ���� ������, ��� ���� �� ���������� ��� �������� ������������� ����.
 ******************************************************************************/

#include "stm32f1xx_hal.h"            // ���������� HAL API
#include "eeprom.h"                   // ���������� ������� EEPROM emulation (����������������� ������)
#include "profile.h"                  // ��������� ��������� ������� � �����


// ����������: ������ �������� ������ � EEPROM ������� ���������� HAL_FLASH_Unlock/HAL_FLASH_Lock...
// ����������: ���� ������ �� ������������ ��������������� (FreeRTOS � ��������) - ������� ��������� ������ API ������ �� ������ ������!


//============================================================================
// ����������� ������
//============================================================================

//��������� �������������� ����������� ������ �������� ����, � ����������������, ��� ���������� �������:

#define PROFILE_VIRTUAL_ADDRESS__LENGTH        (CurrentMode     -PROFILE_MODE_MIN)  *VARIABLES_IN_PROFILE  +1
#define PROFILE_VIRTUAL_ADDRESS__COUNT         (CurrentMode     -PROFILE_MODE_MIN)  *VARIABLES_IN_PROFILE  +2
#define PROFILE_VIRTUAL_ADDRESS__MODE          (PROFILE_MODE_MAX-PROFILE_MODE_MIN+1)*VARIABLES_IN_PROFILE  +1
#define PROFILE_VIRTUAL_ADDRESS__KNIVES        (PROFILE_MODE_MAX-PROFILE_MODE_MIN+1)*VARIABLES_IN_PROFILE  +2
#define PROFILE_VIRTUAL_ADDRESS__CALIBRATION   (PROFILE_MODE_MAX-PROFILE_MODE_MIN+1)*VARIABLES_IN_PROFILE  +3
#define PROFILE_VIRTUAL_ADDRESS__FREQUENCY     (PROFILE_MODE_MAX-PROFILE_MODE_MIN+1)*VARIABLES_IN_PROFILE  +4
#define PROFILE_VIRTUAL_ADDRESS__FLIPANGLE     (PROFILE_MODE_MAX-PROFILE_MODE_MIN+1)*VARIABLES_IN_PROFILE  +5

// �����: �� ������ ��������������� �������� �������� [EEPROM_VIRTUAL_ADDRESS_MIN..EEPROM_VIRTUAL_ADDRESS_MAX] � ������ �������� ������ <eeprom.h> !


// ������ ������: ������ ������ ��������� ������ �� 30 ������� (��������) �� 2 ������ (length + count). 
// � �������� �� �������, ��������, ��� 5 ��������� �����-���������� (mode, knives, calibration, frequency, flipangle).
// ��� �������� ������ ����� ��� uint16_t (2 ����� - ��� ����������� ���������� FLASH ������). � ���, ��� �������, ����� ���� �� ��������� � 1-������� ����������.
// �� ���� ����� ��������� ������ ������� ����������� (���� uint32_t), �� � Setter-������� ������� ������, ��������� �� ��������� ������������ �������� �� ��������� �� 2-����� � ��������� �� � ��������� �������, � � Getter-������� �������� �� �������.



//============================================================================
// ��������� ������
//============================================================================


static uint16_t ProfileStatus = 0;    // ��������� ������������ ������-����� ������ � ������ eeprom.c  (� �������� � �������� EE_ReadVariable / EE_WriteVariable)
static uint16_t CurrentMode   = 1;



//-------------------------------------
// ������������� ������
void Profile_Init(void)
{
  // EEPROM: Restore the pages to a known good state in case of page's status corruption after a power loss 
  // (��������� ��������� ��������, ��� ������ ����������������)
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  EE_Init();
  assert_param(HAL_OK == HAL_FLASH_Lock());
  
  // ��������� �� EEPROM �������� ���������� ��������� ������
  Profile_LoadMode();
}



//-------------------------------------
// ������� �������� ������� ��������� ������������ �������� 
extern __inline uint16_t Profile_GetStatus(void)
{
  return ProfileStatus;
}


/*
//-------------------------------------
// ���� ������ �������� "EEPROM Emulation"
void Profile_Test(void)
{
  uint16_t Data;
  uint16_t Address = 1;
  uint16_t Status;
  
  for(int i=0; i<=255; i++)
  {
    Address++;
    
    Status = HAL_FLASH_Unlock();
    assert_param(Status == HAL_OK);
    
    Status = EE_ReadVariable(Address, &Data);
    assert_param(Status == 0);

    Data = 10;
    Status = EE_WriteVariable(Address, Data);
    assert_param(Status == FLASH_COMPLETE);
    
    Data = 15;
    Status = EE_ReadVariable(Address, &Data);
    assert_param(Status == 0);
    
    Data = 15;
    Status = EE_WriteVariable(Address, Data);
    assert_param(Status == FLASH_COMPLETE);

    Status = HAL_FLASH_Lock();
    assert_param(Status == HAL_OK);
  }
}
*/



//============================================================================
// ������������ �������
//============================================================================


//-------------------------------------
// ������� �������� �������� ������
extern __inline uint16_t Profile_GetMode(void)
{
  return CurrentMode;
}



//-------------------------------------
// ������������� �� ����� �����...
void Profile_GotoMode(uint16_t mode)
{
  if(mode > PROFILE_MODE_MAX)
    CurrentMode = PROFILE_MODE_MAX;
  else if(mode < PROFILE_MODE_MIN)
    CurrentMode = PROFILE_MODE_MIN;
  else
    CurrentMode = mode;
}



//-------------------------------------
// ������������� �� ���������� �����...
void Profile_PrevMode(void)
{
  if(CurrentMode > PROFILE_MODE_MIN)
    CurrentMode--;
  else
    CurrentMode = PROFILE_MODE_MAX;     // ����������� �������
}



//-------------------------------------
// ������������� �� ��������� �����...
void Profile_NextMode(void)
{
  if(CurrentMode < PROFILE_MODE_MAX)
    CurrentMode++;
  else
    CurrentMode = PROFILE_MODE_MIN;     // ����������� �������
}



//-------------------------------------
// ��������� �� EEPROM �������� �������� ������
//  (����������: ���� � EEPROM ��� �� �������� �������� ���� ����������, �� ����� ��������� �������.)
void Profile_LoadMode(void)
{
  //assert_param(HAL_OK == HAL_FLASH_Unlock());

  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__MODE, &CurrentMode);    // ����������: ����������� ����� = ������ ���������, �� ������ �������, ������������ �������� ��������.
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());

  // �������� ����������� ������
  if(CurrentMode > PROFILE_MODE_MAX ||
     CurrentMode < PROFILE_MODE_MIN)
    CurrentMode = 1;                    // �������� �� ���������
}



//-------------------------------------
// ��������� � EEPROM �������� �������� ������
void Profile_SaveMode(void)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());

  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__MODE, CurrentMode);    // ����������: ����������� ����� = ������ ���������, �� ������ �������, ������������ �������� ��������.
  assert_param(ProfileStatus == FLASH_COMPLETE);

  assert_param(HAL_OK == HAL_FLASH_Lock());
}




//============================================================================
// ���������� �������
//============================================================================


//-------------------------------------
// ��������� �� EEPROM �������� �����, ��� �������� ������
uint16_t Profile_LoadLength(void)
{
  uint16_t data = 100;      // �������� �� ��������� (������������, ���� ������ �� ��������� �� EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__LENGTH, &data);    // ����������: ����������� ����� = ����� * <����� ���������� � �������>
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());

  return data;
}



//-------------------------------------
// ��������� � EEPROM �������� �����, ��� �������� ������
void Profile_SaveLength(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());

  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__LENGTH, data);    // ����������: ����������� ����� = ����� * <����� ���������� � �������>
  assert_param(ProfileStatus == FLASH_COMPLETE);

  assert_param(HAL_OK == HAL_FLASH_Lock());
}



//-------------------------------------
// ��������� �� EEPROM �������� ����������, ��� �������� ������
uint16_t Profile_LoadCount(void)
{
  uint16_t data = 100;      // �������� �� ��������� (������������, ���� ������ �� ��������� �� EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__COUNT, &data);    // ����������: ����������� ����� = ����� * <����� ���������� � �������>
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());
  
  return data;
}



//-------------------------------------
// ��������� � EEPROM �������� ����������, ��� �������� ������
void Profile_SaveCount(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__COUNT, data);    // ����������: ����������� ����� = ����� * <����� ���������� � �������>
  assert_param(ProfileStatus == FLASH_COMPLETE);
  
  assert_param(HAL_OK == HAL_FLASH_Lock());
}




//============================================================================
// ������ ����������� ������
//============================================================================


//-------------------------------------
// ��������� �� EEPROM �������� "���������� �����"
uint16_t Profile_LoadKnives(void)
{
  uint16_t data = 2;        // �������� �� ��������� (������������, ���� ������ �� ��������� �� EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__KNIVES, &data);   // ����������: ����������� ����� = ������ ���������, �� ������ �������, ������������ �������� ��������.
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());
  
  return data;
}



//-------------------------------------
// ��������� � EEPROM �������� "���������� �����"
void Profile_SaveKnives(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__KNIVES, data);   // ����������: ����������� ����� = ������ ���������, �� ������ �������, ������������ �������� ��������.
  assert_param(ProfileStatus == FLASH_COMPLETE);
  
  assert_param(HAL_OK == HAL_FLASH_Lock());
}



//-------------------------------------
// ��������� �� EEPROM �������� "������������� �����������"
uint16_t Profile_LoadCalibration(void)
{
  uint16_t data = 0;        // �������� �� ��������� (������������, ���� ������ �� ��������� �� EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__CALIBRATION, &data);   // ����������: ����������� ����� = ������ ���������, �� ������ �������, ������������ �������� ��������.
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());
  
  return data;
}



//-------------------------------------
// ��������� � EEPROM �������� "������������� �����������"
void Profile_SaveCalibration(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__CALIBRATION, data);   // ����������: ����������� ����� = ������ ���������, �� ������ �������, ������������ �������� ��������.
  assert_param(ProfileStatus == FLASH_COMPLETE);
  
  assert_param(HAL_OK == HAL_FLASH_Lock());
}



//-------------------------------------
// ��������� �� EEPROM �������� "������� �����"
uint16_t Profile_LoadFrequency(void)
{
  uint16_t data = 1000;        // �������� �� ��������� (������������, ���� ������ �� ��������� �� EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__FREQUENCY, &data);   // ����������: ����������� ����� = ��������� ���������, �� ������ �������, ������������ �������� ��������.
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());
  
  return data;
}



//-------------------------------------
// ��������� � EEPROM �������� "������� �����"
void Profile_SaveFrequency(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__FREQUENCY, data);   // ����������: ����������� ����� = ��������� ���������, �� ������ �������, ������������ �������� ��������.
  assert_param(ProfileStatus == FLASH_COMPLETE);
  
  assert_param(HAL_OK == HAL_FLASH_Lock());
}



//-------------------------------------
// ��������� �� EEPROM �������� "���������� ����������"
uint16_t Profile_LoadFlipAngle(void)
{
  uint16_t data = 110;        // �������� �� ��������� (������������, ���� ������ �� ��������� �� EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__FLIPANGLE, &data);   // ����������: ����������� ����� = ��������� ���������, �� ������ �������, ������������ �������� ��������.
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());
  
  return data;
}



//-------------------------------------
// ��������� � EEPROM �������� "���������� ����������"
void Profile_SaveFlipAngle(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__FLIPANGLE, data);   // ����������: ����������� ����� = ��������� ���������, �� ������ �������, ������������ �������� ��������.
  assert_param(ProfileStatus == FLASH_COMPLETE);
  
  assert_param(HAL_OK == HAL_FLASH_Lock());
}


