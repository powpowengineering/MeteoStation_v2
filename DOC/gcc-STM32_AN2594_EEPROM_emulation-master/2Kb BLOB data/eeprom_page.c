/******************************************************************************
 * Модуль:      eeprom_page.c
 * Автор:       Celeron (c) 2017
 * Назначение:  Драйвер эмуляции EEPROM памяти во встроенном FLASH.
 *              Вариант выделения памяти: ПОСТРАНИЧНО, т.е. большими массивами целиком.
 ******************************************************************************/
 
#include "stm32f1xx_hal.h"          // подключаем HAL API
#include "flash.h"                  // Низкоуровневые функции работы с FLASH памятью
#include "eeprom_page.h"            // прототипы локальных функций и типов



//============================================================================
// Разметка FLASH памяти
//============================================================================

// Разметить физическую память под сохраняемые данные: 
//  Внимание: эти макроопределения никак не связаны с настройками Linker! Поэтому последний может запросто распределить рабочий код программы на этих страницах, если программа большая и кода много...
//  Поэтому, чтобы <снизить вероятность> перекрытия памяти - под EEPROM следует выделять страницы в самом конце FLASH памяти. 
//  Однако, чтобы железно забить размеченные страницы под EEPROM - следует в настройках Project\Linker(Target) ограничить диапазон <имеющей в микроконтроллере FLASH памяти>, прописанный в IROM (Internal ROM) константах.
//  Также, следует убедиться, что не используете системные области, зарезервированные под Custom BootLoader... Вот, в STM32F103VE нет таких областей <в конце памяти> - поэтому не паримся.


// Определить размер страницы FLASH памяти, доступный для данного микроконтроллера:
//  Обычно, это 1K=0x400 (для слабеньких микроконтроллеров) или 2K=0x800 (для микроконтроллеров средней мощности).
//  Просто взять из библиотеки HAL (всегда корректно - за этим следят инженеры ST) = (uint16_t) FLASH_PAGE_SIZE


// Замечу: микроконтроллеры STM32 XL-серии (у которых более 512K FLASH) имеют два банка памяти - возьмем последний.
#ifdef STM32F10X_XL
  #define EEPAGE_BANK_END    FLASH_BANK2_END
#else
  #define EEPAGE_BANK_END    FLASH_BANK1_END
#endif


// Граничные адреса страниц в памяти
//  Примечание: порядок следования страниц в памяти получится такой:  idx9.page0 idx9.page1  idx8.page0 idx8.page1  idx7.page0 idx7.page1  ...  idx0.page0 idx0.page1 
#define EEPAGE0_START_ADDRESS(CHUNK_IDX)   ((EEPAGE_BANK_END - EEPAGE_PRESERVED_TRAILING_PAGES * FLASH_PAGE_SIZE - 2*FLASH_PAGE_SIZE + 1) - (2 * FLASH_PAGE_SIZE) * (CHUNK_IDX))
#define EEPAGE0_END_ADDRESS  (CHUNK_IDX)   ((EEPAGE_BANK_END - EEPAGE_PRESERVED_TRAILING_PAGES * FLASH_PAGE_SIZE -   FLASH_PAGE_SIZE    ) - (2 * FLASH_PAGE_SIZE) * (CHUNK_IDX))
#define EEPAGE1_START_ADDRESS(CHUNK_IDX)   ((EEPAGE_BANK_END - EEPAGE_PRESERVED_TRAILING_PAGES * FLASH_PAGE_SIZE -   FLASH_PAGE_SIZE + 1) - (2 * FLASH_PAGE_SIZE) * (CHUNK_IDX))
#define EEPAGE1_END_ADDRESS  (CHUNK_IDX)   ((EEPAGE_BANK_END - EEPAGE_PRESERVED_TRAILING_PAGES * FLASH_PAGE_SIZE                        ) - (2 * FLASH_PAGE_SIZE) * (CHUNK_IDX))



//============================================================================
// Вспомогательные функции
//============================================================================


// За "время жизни" страница памяти проходит стадии:
typedef enum  {
  eepageCLEAN       = ((uint16_t)0xFFFF),   // сразу после стирания страницы памяти (получает автоматически)
  eepageVALID       = ((uint16_t)0xEEEE),   // "данные актуальны" - присваивается после УСПЕШНОГО завершения пакетной записи (программирования) данных в ЭТУ страницу
  eepageINVALID     = ((uint16_t)0x0000),   // "данные устарели" - присваивается старой странице, перед тем как начинать сохранение новых данных в "парную страницу"...  
} TEEPageWorkflow;

// Пояснение: страница с устаревшими данными (eepageINVALID) пока не стирается, до успешного переноса данных в новую страницу (eepageVALID). 
//  Но данные в ней уже объявлены "устаревшими". Однако, они могут еще быть использованы как "резервная копия" для восстановления данных в "парной странице", 
//  если произойдет сбой по питанию при программировании последней - так хоть какие-то данные будут восстановлены "из резервной копии"...


// Размер "служебного заголовка" страницы памяти (первые несколько байт)
#define EEPAGE_HEADER_SIZE   sizeof(TEEPageWorkflow)



//-----------------------------------------------------------------------------
// Копирует вектор данных, размером EEPAGE_CHUNK_SIZE байт, порциями по 2 байта (uint16_t),
// данные вычитываются из источника начиная с адреса source (может быть как FLASH, так и ОЗУ),
// и программируются в приемник (предполагается, что это FLASH память) начиная с адреса target.

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
// Переключает "пару страниц" (инвалидирует старую, очищает и заполняет новую, очищает старую)
// Новые данные вычитываются из источника начиная с адреса DataSource (может быть как FLASH, так и ОЗУ),
// Два других параметра задают физические адреса начала сраниц в памяти FLASH.

static HAL_StatusTypeDef EEPAGE_MoveDataToNewPage_UINT16(uint32_t OldInvalidPageBase, uint32_t NewValidPageBase, uint32_t DataSource)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  // Прописать старой странице статус "невалидная" (возможно повторно, на всякий случай - сбоя не будет)
  status = AN2594_ProgramHalfWord( OldInvalidPageBase, eepageINVALID );
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // Стереть страницу-приемник
  status = AN2594_ErasePage(NewValidPageBase);
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // Заполнить новую страницу данными
  status = EEPAGE_CopyDataFromSourceToTarget_UINT16(DataSource /*OldInvalidPageBase + EEPAGE_HEADER_SIZE*/, NewValidPageBase + EEPAGE_HEADER_SIZE);
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // Прописать новой странице статус "валидная"
  status = AN2594_ProgramHalfWord( NewValidPageBase, eepageVALID );
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // Стереть страницу-источник
  status = AN2594_ErasePage(OldInvalidPageBase);
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;  
  
  return status;
}



//-----------------------------------------------------------------------------
// Проверяет "пару страниц" на ненормальные состояния, 
// и пытается их восстановить при необходимости

static HAL_StatusTypeDef EEPAGE_FixPageIfNecessary(uint8_t VirtAddress)
{
  HAL_StatusTypeDef status = HAL_OK;

  // Проверка параметров
  if(VirtAddress >= EEPAGE_CHUNKS_COUNT)
    return HAL_ERROR;
  
  
  // Сверим статусы "пары страниц"
  TEEPageWorkflow PageStatus0 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE0_START_ADDRESS(VirtAddress) );
  TEEPageWorkflow PageStatus1 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE1_START_ADDRESS(VirtAddress) );
  
  
  // Признак здоровой ситуации: одна из страниц должна быть "валидной", а другая "чистой".
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
  
  
  // Признак сбоя при прошлой записи: если одна из страниц помечена как "невалидная", а вторая имеет любой другой статус (исключая "валидный" - такую ситуацию мы уже закрыли на прошлом шаге)...
  if( PageStatus0 == eepageINVALID && 
      PageStatus1 != eepageINVALID )
  {
    // Восстановление "резервной копии" данных из страницы 0->1
    status = EEPAGE_MoveDataToNewPage_UINT16(EEPAGE0_START_ADDRESS(VirtAddress), 
                                             EEPAGE1_START_ADDRESS(VirtAddress), 
                                             EEPAGE0_START_ADDRESS(VirtAddress) + EEPAGE_HEADER_SIZE);
    assert_param(status == HAL_OK);
    return status;
  }
  if( PageStatus0 != eepageINVALID && 
      PageStatus1 == eepageINVALID )
  {
    // Восстановление "резервной копии" данных из страницы 1->0
    status = EEPAGE_MoveDataToNewPage_UINT16(EEPAGE1_START_ADDRESS(VirtAddress), 
                                             EEPAGE0_START_ADDRESS(VirtAddress), 
                                             EEPAGE1_START_ADDRESS(VirtAddress) + EEPAGE_HEADER_SIZE);
    assert_param(status == HAL_OK);
    return status;
  }  
  
  
//  // Если обе страницы чистые (как бывает при самом первом запуске), то одну из них нужно принудительно промаркировать как "валидную" (готовую к чтению), а другая останется "пустой".
//  if( PageStatus0 == eepageINVALID && 
//      PageStatus1 == eepageCLEAN )
//  {
//    status = AN2594_ProgramHalfWord( EEPAGE0_START_ADDRESS(VirtAddress), eepageVALID );
//    assert_param(status == HAL_OK);
//    return status;
//  }
  
  
  // Во всех других случаях (неожиданные комбинации статускодов - что-то явно пошло не так) разбираться не будем, а тупо переформатируем обе страницы.
  {
    // Стереть обе страницы
    status = AN2594_ErasePage(EEPAGE0_START_ADDRESS(VirtAddress));   //Замечу: в памяти, PAGE0 расположена раньше, а сразу за ней PAGE1...
    assert_param(status == HAL_OK);
    if(status != HAL_OK) return status;  

    status = AN2594_ErasePage(EEPAGE1_START_ADDRESS(VirtAddress));   //Замечу: в памяти, PAGE0 расположена раньше, а сразу за ней PAGE1...
    assert_param(status == HAL_OK);
    if(status != HAL_OK) return status;  

    // Прописать первой странице статус "валидная"
    status = AN2594_ProgramHalfWord( EEPAGE0_START_ADDRESS(VirtAddress), eepageVALID );
    assert_param(status == HAL_OK);
    if(status != HAL_OK) return status;
  }
  
  
  return status;
}




//============================================================================
// Методы API
//============================================================================

//  Возвращают статускод успешности операции: HAL_OK или HAL_ERROR, HAL_TIMEOUT.

//-----------------------------------------------------------------------------
// Инициализация модуля
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
// Загрузка массива данных в ОЗУ
HAL_StatusTypeDef EEPAGE_LoadData(uint8_t VirtAddress, void* Data)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  // Проверка параметров
  if(VirtAddress >= EEPAGE_CHUNKS_COUNT || Data == 0)
    return HAL_ERROR;   
  
  // Прежде чем работать со страницей - убедимся в целостности данных
  status = EEPAGE_FixPageIfNecessary(VirtAddress);
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // Сверим статусы "пары страниц"
  TEEPageWorkflow PageStatus0 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE0_START_ADDRESS(VirtAddress) );
  TEEPageWorkflow PageStatus1 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE1_START_ADDRESS(VirtAddress) );
  
  // Найти валидную страницу
  uint32_t source = NULL;
  if( PageStatus0 == eepageVALID )
    source = EEPAGE0_START_ADDRESS(VirtAddress) + EEPAGE_HEADER_SIZE;
  else if( PageStatus1 == eepageVALID )
    source = EEPAGE1_START_ADDRESS(VirtAddress) + EEPAGE_HEADER_SIZE;
  else
    return HAL_ERROR;

  // Загрузить данные
  __IO uint16_t* target = (uint16_t*) Data;
  for(uint16_t i = 0; i < (EEPAGE_CHUNK_SIZE / sizeof(uint16_t)); i++)
  {
    *target = FLASH_Read_UINT16(source);
    source += sizeof(uint16_t);
    target += 1;    //Примечание: т.к. target это не целочисленная переменная, хранящая адрес памяти (как source). Но target является указателем типа (uint16_t*), то по правилам "арифметики указателей" - указатель следует просто инкрементировать, чтобы он переместился на следующий адрес +sizeof(uint16_t);
  }
  
  return status;
}



//-----------------------------------------------------------------------------
// Сохранение массива данных во FLASH
HAL_StatusTypeDef EEPAGE_SaveData(uint8_t VirtAddress, void* Data)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  // Проверка параметров
  if(VirtAddress >= EEPAGE_CHUNKS_COUNT || Data == 0)
    return HAL_ERROR;   
  
  // Прежде чем работать со страницей - убедимся в целостности данных
  status = EEPAGE_FixPageIfNecessary(VirtAddress);
  assert_param(status == HAL_OK);
  if(status != HAL_OK) return status;
  
  // Сверим статусы "пары страниц"
  TEEPageWorkflow PageStatus0 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE0_START_ADDRESS(VirtAddress) );
  TEEPageWorkflow PageStatus1 = (TEEPageWorkflow) FLASH_Read_UINT16( EEPAGE1_START_ADDRESS(VirtAddress) );
  
  // Найти валидную страницу
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

  // Сохранить данные
  status = EEPAGE_MoveDataToNewPage_UINT16(OldInvalidPageBase, NewValidPageBase, (uint32_t) Data);
  assert_param(status == HAL_OK);
  return status;
}


