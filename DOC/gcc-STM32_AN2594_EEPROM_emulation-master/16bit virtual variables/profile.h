#ifndef PROFILE_H
#define PROFILE_H


//-------------------------------------
// ����������������
//-------------------------------------

//��������: ��������� �������� ���� ���������������� �������� � ��������� ����������� ��������� ����������� ������!
//�������������, ��� ����� ����������� � ����������������� ������ ������ - ����������!

#define PROFILE_MODE_MIN      1
#define PROFILE_MODE_MAX     30

#define VARIABLES_IN_PROFILE  2



//-------------------------------------
// ��������� ������
//-------------------------------------

// ������������� ������ (������� ������ ���� ������� ��������, ��� ������ ���������)
void Profile_Init(void);

// ������� �������� ������� ��������� ������������ �������� 
uint16_t Profile_GetStatus(void);
  


//-------------------------------------
// ������������ �������
//-------------------------------------

// ������� �������� �������� ������
uint16_t Profile_GetMode(void);

// ������������� �� ����� �����...
void Profile_GotoMode(uint16_t mode);

// ������������� �� ���������� �����...
void Profile_PrevMode(void);

// ������������� �� ��������� �����...
void Profile_NextMode(void);


// ��������� �� EEPROM �������� �������� ������
void Profile_LoadMode(void);

// ��������� � EEPROM �������� �������� ������
void Profile_SaveMode(void);



//-------------------------------------
// ���������� �������
//-------------------------------------

// ��������� �� EEPROM �������� �����, ��� �������� ������
uint16_t Profile_LoadLength(void);

// ��������� � EEPROM �������� �����, ��� �������� ������
void Profile_SaveLength(uint16_t data);


// ��������� �� EEPROM �������� ����������, ��� �������� ������
uint16_t Profile_LoadCount(void);

// ��������� � EEPROM �������� ����������, ��� �������� ������
void Profile_SaveCount(uint16_t data);



//-------------------------------------
// ������ ����������� ������
//-------------------------------------

// ��������� �� EEPROM �������� "���������� �����"
uint16_t Profile_LoadKnives(void);

// ��������� � EEPROM �������� "���������� �����"
void Profile_SaveKnives(uint16_t data);

// ��������� �� EEPROM �������� "������������� �����������"
uint16_t Profile_LoadCalibration(void);

// ��������� � EEPROM �������� "������������� �����������"
void Profile_SaveCalibration(uint16_t data);

// ��������� �� EEPROM �������� "������� �����"
uint16_t Profile_LoadFrequency(void);

// ��������� � EEPROM �������� "������� �����"
void Profile_SaveFrequency(uint16_t data);

// ��������� �� EEPROM �������� "���������� ����������"
uint16_t Profile_LoadFlipAngle(void);

// ��������� � EEPROM �������� "���������� ����������"
void Profile_SaveFlipAngle(uint16_t data);



#endif  // PROFILE_H
