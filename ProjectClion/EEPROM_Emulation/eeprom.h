#ifndef EEPROM_H
#define EEPROM_H

#include "stm32l1xx.h"

#include "general_types.h"

//-------------------------------------
// ����������� ������
//-------------------------------------


// ������������ �������� ����������� �������, ��� ����������� ����������
//  �����������: 
//    ��� �������� ����������, ������������ ����������� ������� ����������.
//    ������ ������������ ������������� ��� ������� �����.
//    ����������� ����� 0xFFFF ��������! �.�. ������������ ����� <= 0xFFFE
#define EEPROM_VIRTUAL_ADDRESS_MIN  1
#define EEPROM_VIRTUAL_ADDRESS_MAX  65



//-------------------------------------
// API ������
//-------------------------------------


// ���������� ��������� ����, ������������ �������� �������� EEPROM:
#undef FLASH_ERROR_PG
#undef FLASH_ERROR_WRP
//typedef enum
//{
//  FLASH_BUSY      = HAL_BUSY,
//  FLASH_ERROR_PG  = HAL_ERROR,
//  FLASH_ERROR_WRP = HAL_ERROR,
//  FLASH_COMPLETE  = HAL_OK,
//  FLASH_TIMEOUT   = HAL_TIMEOUT
//} FLASH_Status;


// ������������� ������
uint16_t EE_Init(void);

// ������ ������
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data);

// ���������� ������
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data);

// BUG: ��������! � ������ ����������� ���������� EE_Init(), ��� ������ ��������� �������, ������� �������� ��������� "������� �������� ������", ����������� ����� ���������� FLASH ������ (������� � ��� �� �����).

#endif  // EEPROM_H
