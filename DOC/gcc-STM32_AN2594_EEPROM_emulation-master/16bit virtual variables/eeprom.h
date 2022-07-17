#ifndef EEPROM_H
#define EEPROM_H


//-------------------------------------
// Виртуальные адреса
//-------------------------------------


// Используемый диапазон виртуальных адресов, для сохраняемых переменных
//  Особенности: 
//    Для простоты реализации, пространство виртуальных адресов непрерывно.
//    Нельзя использовать отрицательные или дробные числа.
//    Виртуальный адрес 0xFFFF запрещен! Т.е. максимальный адрес <= 0xFFFE
#define EEPROM_VIRTUAL_ADDRESS_MIN  1
#define EEPROM_VIRTUAL_ADDRESS_MAX  65



//-------------------------------------
// API модуля
//-------------------------------------


// Формальные статусные коды, возвращаемые методами эмуляции EEPROM:
#undef FLASH_ERROR_PG
#undef FLASH_ERROR_WRP
typedef enum
{ 
  FLASH_BUSY      = HAL_BUSY,
  FLASH_ERROR_PG  = HAL_ERROR,
  FLASH_ERROR_WRP = HAL_ERROR,
  FLASH_COMPLETE  = HAL_OK,
  FLASH_TIMEOUT   = HAL_TIMEOUT
} FLASH_Status;



// Инициализация модуля
uint16_t EE_Init(void);

// Чтение данных
uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data);

// Сохранение данных
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data);

// BUG: Внимание! В данной говнокодной реализации EE_Init(), при каждом включении питания, впустую повторно стирается "теневая страница памяти", вырабатывая циклы перезаписи FLASH памяти (которых и так не много).

#endif  // EEPROM_H
