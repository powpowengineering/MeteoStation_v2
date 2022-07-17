/******************************************************************************
 * Модуль:      program.c
 * Автор:       Celeron (c) 2017
 * Назначение:  Вспомогательный модуль поддержки Модели Данных (адаптеры для работы с EEPROM)
 ******************************************************************************/


#include "stm32f1xx_hal.h"          // подключаем HAL API
#include "eeprom_page.h"            // Подключаем EEPROM emulation (энергонезависимую память)
#include "program.h"                // прототипы локальных функций и типов



//============================================================================
// Данные
//============================================================================


// Данные Текущей Программы (массив пауз)
//  Критично: массива должен быть как минимум размером EEPAGE_CHUNK_SIZE байт!
//  Коррекция: память под массив обязательно нужно выделить +1 шаг, чтобы скомпенсировать округление при делении в выражении PROGRAM_STEPS_COUNT, выше...
volatile static uint16_t ProgramData[(PROGRAM_STEPS_COUNT+1)*2];


// Статистика по Текущей Программе (перерасчитывается при загрузке и редактировании программы)
volatile static TProgramStatistics Program_Statistics;


// Макрос для индексации массива данных Текущей Программы: 
//  STEP  = номер шага, начиная с единицы!
//  PHASE = фаза 0-мочим/1-сушим
#define PROGRAM_DATA(STEP, PHASE)  (ProgramData[ (STEP-1)*2 +PHASE ])


//// Проверка корректности данных
////  где: EEPAGE_CHUNK_SIZE=(FLASH_PAGE_SIZE-2)... изврат! ну, нельзя использовать оператор sizeof в выражениях макропроцессора :(
//#if  ((FLASH_PAGE_SIZE-2)/4 +1)*2*2 < (FLASH_PAGE_SIZE-2)
//#error "Ошибка: недостаточно памяти выделено под Массив Программы!"
//#endif


// Стандартный вспомогательный приемчик: вычисляет количество элементов в массиве.
// См. справку по "Препроцессору Си"... http://ru-wiki.org/wiki/%D0%9F%D1%80%D0%B5%D0%BF%D1%80%D0%BE%D1%86%D0%B5%D1%81%D1%81%D0%BE%D1%80_%D0%A1%D0%B8
#define NUMBER_OF_ARRAY_ITEMS( array ) ( sizeof( array ) / sizeof( *(array) ) )




//============================================================================
// Статистика
//============================================================================


//-------------------------------------
// Перерассчитать Статистику по Текущей Программе
void Program_RefreshStatistics(void)
{
  // Проверка корректности данных
  //  Критично: размер массива должен быть как минимум EEPAGE_CHUNK_SIZE (FLASH_PAGE_SIZE-2) байт!
  //  где EEPAGE_CHUNK_SIZE = FLASH_PAGE_SIZE-2  (обычно, 2048-2 = 2046 байт, т.е. 1023 ячеек по uint16_t - что хватит для хранения 511 полных шагов, по 2 ячейки)
  assert_param(sizeof(ProgramData) >= EEPAGE_CHUNK_SIZE);
  
  
  // Перерасчитать статистику
  uint32_t sum  = 0;
  uint16_t step = 0;
  
  for(step=1; step <= PROGRAM_STEPS_COUNT 
              && PROGRAM_DATA(step,0) != PROGRAM_END 
              && PROGRAM_DATA(step,1) != PROGRAM_END; step++)
    sum += PROGRAM_DATA(step,0) + PROGRAM_DATA(step,1);
  
  Program_Statistics.TotalSec  = sum;       // Время всего (сек)
  Program_Statistics.TotalStep = step-1;    // Шагов всего (шт)
}



//-------------------------------------
// Вернуть статистику по Текущей Программе
extern __inline TProgramStatistics Program_GetStatistics(void)
{
  return Program_Statistics;
}




//============================================================================
// Массив данных
//============================================================================


//-------------------------------------
// Загрузить данные Текущей Программы из FLASH в ОЗУ, и перерасчитать статистику...
HAL_StatusTypeDef Program_ReloadData(void)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  // Проверка корректности данных
  //  Критично: размер массива должен быть как минимум EEPAGE_CHUNK_SIZE (FLASH_PAGE_SIZE-2) байт!
  //  где EEPAGE_CHUNK_SIZE = FLASH_PAGE_SIZE-2  (обычно, 2048-2 = 2046 байт, т.е. 1023 ячеек по uint16_t - что хватит для хранения 511 полных шагов, по 2 ячейки)
  assert_param(sizeof(ProgramData) >= EEPAGE_CHUNK_SIZE);

  // Загрузить Данные Программы из FLASH в ОЗУ
  //  Примечание: Массивы доступны по ZERO-BASED индексу: VirtAddress = [0..EEPAGE_CHUNKS_COUNT-1]
  status = EEPAGE_LoadData( Profile_GetProgram()-1, (void*) ProgramData );
  assert_param(status == HAL_OK);

  // В случае ошибки загрузки массива из FLASH - забьем все ячейки массива "пустыми" данными
  if(status != HAL_OK)
    for(uint16_t i=0; i<NUMBER_OF_ARRAY_ITEMS(ProgramData); i++)
      ProgramData[i] = PROGRAM_END;

  // Обновить Статистику
  Program_RefreshStatistics();
  
  return status;
}



//-------------------------------------
// Сохранить данные Текущей Программы из ОЗУ во FLASH...
HAL_StatusTypeDef Program_SaveData(void)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  // Проверка корректности данных
  //  Критично: размер массива должен быть как минимум EEPAGE_CHUNK_SIZE (FLASH_PAGE_SIZE-2) байт!
  //  где EEPAGE_CHUNK_SIZE = FLASH_PAGE_SIZE-2  (обычно, 2048-2 = 2046 байт, т.е. 1023 ячеек по uint16_t - что хватит для хранения 511 полных шагов, по 2 ячейки)
  assert_param(sizeof(ProgramData) >= EEPAGE_CHUNK_SIZE);
  
  // Обновить Статистику
  Program_RefreshStatistics();

  status = HAL_FLASH_Unlock();
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;

  // Сохранить Данные Программы во FLASH из ОЗУ
  //  Примечание: Массивы доступны по ZERO-BASED индексу: VirtAddress = [0..EEPAGE_CHUNKS_COUNT-1]
  status = EEPAGE_SaveData( Profile_GetProgram()-1, (void*) ProgramData );
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;

  status = HAL_FLASH_Lock();
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  return status;
}




//============================================================================
// Редактирование элемента массива
//============================================================================


//-------------------------------------
// Получить ячейку данных Текущей Программы (только из ОЗУ)
extern __inline uint16_t Program_GetData(uint16_t step, uint8_t phase)
{
  return PROGRAM_DATA(step, phase);
}



//-------------------------------------
// Сохранить ячейку данных Текущей Программы (только в ОЗУ)
extern __inline void Program_SetData(uint16_t step, uint8_t phase, uint16_t value)
{
  PROGRAM_DATA(step, phase) = value;
}



//-------------------------------------
// Инкрементировать ячейку данных Текущей Программы (только в ОЗУ)
extern __inline void Program_IncData(uint16_t step, uint8_t phase, uint16_t value)
{
  PROGRAM_DATA(step, phase) += value;
}



//-------------------------------------
// Декрементировать ячейку данных Текущей Программы (только в ОЗУ)
extern __inline void Program_DecData(uint16_t step, uint8_t phase, uint16_t value)
{
  PROGRAM_DATA(step, phase) -= value;
}



