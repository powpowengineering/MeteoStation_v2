#ifndef PROFILE_H
#define PROFILE_H


//-------------------------------------
// Макроопределения
//-------------------------------------

//Внимание: изменение значений этих макроопределений приведет к изменению виртуальной адресации сохраняемых данных!
//Следовательно, все ранее сохраненные в энергонезависимую память данные - потеряются!

#define PROFILE_MODE_MIN      1
#define PROFILE_MODE_MAX     30

#define VARIABLES_IN_PROFILE  2



//-------------------------------------
// Системные методы
//-------------------------------------

// Инициализация модуля (функция должна быть вызвана единожды, при старте программы)
void Profile_Init(void);

// Вернуть значение Статуса последней отработанной операции 
uint16_t Profile_GetStatus(void);
  


//-------------------------------------
// Переключение профиля
//-------------------------------------

// Вернуть значение текущего Режима
uint16_t Profile_GetMode(void);

// Переключиться на Режим номер...
void Profile_GotoMode(uint16_t mode);

// Переключиться на предыдущий Режим...
void Profile_PrevMode(void);

// Переключиться на следующий Режим...
void Profile_NextMode(void);


// Загружить из EEPROM значение текущего Режима
void Profile_LoadMode(void);

// Сохранить в EEPROM значение текущего Режима
void Profile_SaveMode(void);



//-------------------------------------
// Содержимое профиля
//-------------------------------------

// Загружить из EEPROM значение Длина, для текущего Режима
uint16_t Profile_LoadLength(void);

// Сохранить в EEPROM значение Длина, для текущего Режима
void Profile_SaveLength(uint16_t data);


// Загружить из EEPROM значение Количество, для текущего Режима
uint16_t Profile_LoadCount(void);

// Сохранить в EEPROM значение Количество, для текущего Режима
void Profile_SaveCount(uint16_t data);



//-------------------------------------
// Другие сохраняемые данные
//-------------------------------------

// Загрузить из EEPROM значение "Количество ножей"
uint16_t Profile_LoadKnives(void);

// Сохранить в EEPROM значение "Количество ножей"
void Profile_SaveKnives(uint16_t data);

// Загрузить из EEPROM значение "Калибровочный коэффициент"
uint16_t Profile_LoadCalibration(void);

// Сохранить в EEPROM значение "Калибровочный коэффициент"
void Profile_SaveCalibration(uint16_t data);

// Загрузить из EEPROM значение "Частота Звука"
uint16_t Profile_LoadFrequency(void);

// Сохранить в EEPROM значение "Частота Звука"
void Profile_SaveFrequency(uint16_t data);

// Загрузить из EEPROM значение "Расстояние переворота"
uint16_t Profile_LoadFlipAngle(void);

// Сохранить в EEPROM значение "Расстояние переворота"
void Profile_SaveFlipAngle(uint16_t data);



#endif  // PROFILE_H
