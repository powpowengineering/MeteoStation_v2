/******************************************************************************
 * Модуль:      profile.c
 * Автор:       Celeron (c) 2015
 * Назначение:  Пример использования драйвера eeprom.c
 *              Модуль содержит обёртки "Getters" и "Setters" for Virtual variables.
 *              Такой модуль гораздо удобнее использовать из прикладного кода, чем непосредственно низкоуровневые вызовы из eeprom.c...
 *              Disclaimer: этот модуль взят из реального (рабочего) проекта и специально как пример не задумывался - поэтому кода больше, чем было бы достаточно для простого обрисовывания идеи.
 ******************************************************************************/

#include "stm32f1xx_hal.h"            // подключаем HAL API
#include "eeprom.h"                   // подключаем драйвер EEPROM emulation (энергонезависимую память)
#include "profile.h"                  // прототипы локальных функций и типов


// Примечание: Каждая операция ЗАПИСИ в EEPROM требует транзакции HAL_FLASH_Unlock/HAL_FLASH_Lock...
// Примечание: Этот модуль не поддерживает многопоточность (FreeRTOS и мьютексы) - поэтому вызывайте методы API только из одного потока!


//============================================================================
// Виртуальные адреса
//============================================================================

//Выражения подсчитывающие виртуальные адреса вынесены сюда, в макроопределения, для облегчения отладки:

#define PROFILE_VIRTUAL_ADDRESS__LENGTH        (CurrentMode     -PROFILE_MODE_MIN)  *VARIABLES_IN_PROFILE  +1
#define PROFILE_VIRTUAL_ADDRESS__COUNT         (CurrentMode     -PROFILE_MODE_MIN)  *VARIABLES_IN_PROFILE  +2
#define PROFILE_VIRTUAL_ADDRESS__MODE          (PROFILE_MODE_MAX-PROFILE_MODE_MIN+1)*VARIABLES_IN_PROFILE  +1
#define PROFILE_VIRTUAL_ADDRESS__KNIVES        (PROFILE_MODE_MAX-PROFILE_MODE_MIN+1)*VARIABLES_IN_PROFILE  +2
#define PROFILE_VIRTUAL_ADDRESS__CALIBRATION   (PROFILE_MODE_MAX-PROFILE_MODE_MIN+1)*VARIABLES_IN_PROFILE  +3
#define PROFILE_VIRTUAL_ADDRESS__FREQUENCY     (PROFILE_MODE_MAX-PROFILE_MODE_MIN+1)*VARIABLES_IN_PROFILE  +4
#define PROFILE_VIRTUAL_ADDRESS__FLIPANGLE     (PROFILE_MODE_MAX-PROFILE_MODE_MIN+1)*VARIABLES_IN_PROFILE  +5

// Важно: не забудь скорректировать значения констант [EEPROM_VIRTUAL_ADDRESS_MIN..EEPROM_VIRTUAL_ADDRESS_MAX] в модуле драйвера памяти <eeprom.h> !


// Модель данных: данный модуль эмулирует массив из 30 записей (профилей) по 2 ячейки (length + count). 
// И отдельно от массива, вдобавок, ещё 5 отдельных ячеек-переменных (mode, knives, calibration, frequency, flipangle).
// Все конечные ячейки имеют тип uint16_t (2 байта - это обусловлено структурой FLASH памяти). В них, без проблем, можно было бы сохранять и 1-байтные переменные.
// Но если нужно сохранять данные большей разрядности (типа uint32_t), то в Setter-методах данного модуля, следовало бы разбивать оригинальное значение на фрагменты по 2-байта и сохранять их в отдельных ячейках, а в Getter-методах собирать их обратно.



//============================================================================
// Системные методы
//============================================================================


static uint16_t ProfileStatus = 0;    // Кодировку возвращаемых статус-кодов смотри в модуле eeprom.c  (в описании к функциям EE_ReadVariable / EE_WriteVariable)
static uint16_t CurrentMode   = 1;



//-------------------------------------
// Инициализация модуля
void Profile_Init(void)
{
  // EEPROM: Restore the pages to a known good state in case of page's status corruption after a power loss 
  // (требуется запускать единожды, при старте микроконтроллера)
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  EE_Init();
  assert_param(HAL_OK == HAL_FLASH_Lock());
  
  // Загрузить из EEPROM значение последнего активного Режима
  Profile_LoadMode();
}



//-------------------------------------
// Вернуть значение Статуса последней отработанной операции 
extern __inline uint16_t Profile_GetStatus(void)
{
  return ProfileStatus;
}


/*
//-------------------------------------
// Тест работы драйвера "EEPROM Emulation"
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
// Переключение профиля
//============================================================================


//-------------------------------------
// Вернуть значение текущего Режима
extern __inline uint16_t Profile_GetMode(void)
{
  return CurrentMode;
}



//-------------------------------------
// Переключиться на Режим номер...
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
// Переключиться на предыдущий Режим...
void Profile_PrevMode(void)
{
  if(CurrentMode > PROFILE_MODE_MIN)
    CurrentMode--;
  else
    CurrentMode = PROFILE_MODE_MAX;     // циклический переход
}



//-------------------------------------
// Переключиться на следующий Режим...
void Profile_NextMode(void)
{
  if(CurrentMode < PROFILE_MODE_MAX)
    CurrentMode++;
  else
    CurrentMode = PROFILE_MODE_MIN;     // циклический переход
}



//-------------------------------------
// Загрузить из EEPROM значение текущего Режима
//  (Примечание: если в EEPROM еще не записано значение этой переменной, то Режим останется прежним.)
void Profile_LoadMode(void)
{
  //assert_param(HAL_OK == HAL_FLASH_Unlock());

  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__MODE, &CurrentMode);    // Примечание: Виртуальный адрес = первый следующий, за блоком адресов, используемых массивом профилей.
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());

  // Проверка целостности данных
  if(CurrentMode > PROFILE_MODE_MAX ||
     CurrentMode < PROFILE_MODE_MIN)
    CurrentMode = 1;                    // значение по умолчанию
}



//-------------------------------------
// Сохранить в EEPROM значение текущего Режима
void Profile_SaveMode(void)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());

  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__MODE, CurrentMode);    // Примечание: Виртуальный адрес = первый следующий, за блоком адресов, используемых массивом профилей.
  assert_param(ProfileStatus == FLASH_COMPLETE);

  assert_param(HAL_OK == HAL_FLASH_Lock());
}




//============================================================================
// Содержимое профиля
//============================================================================


//-------------------------------------
// Загрузить из EEPROM значение Длина, для текущего Режима
uint16_t Profile_LoadLength(void)
{
  uint16_t data = 100;      // значение по умолчанию (возвращается, если данные не прочитаны из EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__LENGTH, &data);    // Примечание: Виртуальный адрес = Режим * <номер переменной в профиле>
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());

  return data;
}



//-------------------------------------
// Сохранить в EEPROM значение Длина, для текущего Режима
void Profile_SaveLength(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());

  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__LENGTH, data);    // Примечание: Виртуальный адрес = Режим * <номер переменной в профиле>
  assert_param(ProfileStatus == FLASH_COMPLETE);

  assert_param(HAL_OK == HAL_FLASH_Lock());
}



//-------------------------------------
// Загрузить из EEPROM значение Количество, для текущего Режима
uint16_t Profile_LoadCount(void)
{
  uint16_t data = 100;      // значение по умолчанию (возвращается, если данные не прочитаны из EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__COUNT, &data);    // Примечание: Виртуальный адрес = Режим * <номер переменной в профиле>
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());
  
  return data;
}



//-------------------------------------
// Сохранить в EEPROM значение Количество, для текущего Режима
void Profile_SaveCount(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__COUNT, data);    // Примечание: Виртуальный адрес = Режим * <номер переменной в профиле>
  assert_param(ProfileStatus == FLASH_COMPLETE);
  
  assert_param(HAL_OK == HAL_FLASH_Lock());
}




//============================================================================
// Другие сохраняемые данные
//============================================================================


//-------------------------------------
// Загрузить из EEPROM значение "Количество ножей"
uint16_t Profile_LoadKnives(void)
{
  uint16_t data = 2;        // значение по умолчанию (возвращается, если данные не прочитаны из EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__KNIVES, &data);   // Примечание: Виртуальный адрес = второй следующий, за блоком адресов, используемых массивом профилей.
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());
  
  return data;
}



//-------------------------------------
// Сохранить в EEPROM значение "Количество ножей"
void Profile_SaveKnives(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__KNIVES, data);   // Примечание: Виртуальный адрес = второй следующий, за блоком адресов, используемых массивом профилей.
  assert_param(ProfileStatus == FLASH_COMPLETE);
  
  assert_param(HAL_OK == HAL_FLASH_Lock());
}



//-------------------------------------
// Загрузить из EEPROM значение "Калибровочный коэффициент"
uint16_t Profile_LoadCalibration(void)
{
  uint16_t data = 0;        // значение по умолчанию (возвращается, если данные не прочитаны из EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__CALIBRATION, &data);   // Примечание: Виртуальный адрес = третий следующий, за блоком адресов, используемых массивом профилей.
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());
  
  return data;
}



//-------------------------------------
// Сохранить в EEPROM значение "Калибровочный коэффициент"
void Profile_SaveCalibration(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__CALIBRATION, data);   // Примечание: Виртуальный адрес = третий следующий, за блоком адресов, используемых массивом профилей.
  assert_param(ProfileStatus == FLASH_COMPLETE);
  
  assert_param(HAL_OK == HAL_FLASH_Lock());
}



//-------------------------------------
// Загрузить из EEPROM значение "Частота Звука"
uint16_t Profile_LoadFrequency(void)
{
  uint16_t data = 1000;        // значение по умолчанию (возвращается, если данные не прочитаны из EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__FREQUENCY, &data);   // Примечание: Виртуальный адрес = четвертый следующий, за блоком адресов, используемых массивом профилей.
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());
  
  return data;
}



//-------------------------------------
// Сохранить в EEPROM значение "Частота Звука"
void Profile_SaveFrequency(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__FREQUENCY, data);   // Примечание: Виртуальный адрес = четвертый следующий, за блоком адресов, используемых массивом профилей.
  assert_param(ProfileStatus == FLASH_COMPLETE);
  
  assert_param(HAL_OK == HAL_FLASH_Lock());
}



//-------------------------------------
// Загрузить из EEPROM значение "Расстояние переворота"
uint16_t Profile_LoadFlipAngle(void)
{
  uint16_t data = 110;        // значение по умолчанию (возвращается, если данные не прочитаны из EEPROM)
  
  //assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_ReadVariable( PROFILE_VIRTUAL_ADDRESS__FLIPANGLE, &data);   // Примечание: Виртуальный адрес = четвертый следующий, за блоком адресов, используемых массивом профилей.
  assert_param(ProfileStatus == 0);
  
  //assert_param(HAL_OK == HAL_FLASH_Lock());
  
  return data;
}



//-------------------------------------
// Сохранить в EEPROM значение "Расстояние переворота"
void Profile_SaveFlipAngle(uint16_t data)
{
  assert_param(HAL_OK == HAL_FLASH_Unlock());
  
  ProfileStatus = EE_WriteVariable( PROFILE_VIRTUAL_ADDRESS__FLIPANGLE, data);   // Примечание: Виртуальный адрес = четвертый следующий, за блоком адресов, используемых массивом профилей.
  assert_param(ProfileStatus == FLASH_COMPLETE);
  
  assert_param(HAL_OK == HAL_FLASH_Lock());
}


