#ifndef EEPAGE_H
#define EEPAGE_H


//-------------------------------------
// Виртуальные адреса
//-------------------------------------


// Модуль позволяет сохранять несколько отдельных Массивов - каждый в отдельной странице памяти.
// Для ясности, сохраняемые Массивы будут доступны по ZERO-BASED индексу: VirtAddress = [0..EEPAGE_CHUNKS_COUNT-1]
#define EEPAGE_CHUNKS_COUNT   20

// Размер массива данных (в байтах), хранимого в странице памяти = должен быть равен размеру страницы, за вычетом служебного заголовка (под который зарезервирована 1 ячейка FLASH = 2 байта = 1 полуслово).
// Внимание: указатель Data, передаваемый в API-методы, должен указывать на уже выделенную память достаточного размера (EEPAGE_CHUNK_SIZE байт), размещенную в ОЗУ (SRAM)!
// Рекомендация: примеры статического выделения памяти под массив
//    uint8_t  Data[EEPAGE_CHUNK_SIZE];  
//    uint16_t Data[EEPAGE_CHUNK_SIZE/2];
//    uint32_t Data[EEPAGE_CHUNK_SIZE / sizeof(uint32_t)];
#define EEPAGE_CHUNK_SIZE   (FLASH_PAGE_SIZE - sizeof(uint16_t))


// Резервирование: Отступить от края FLASH, оставив несколько страниц памяти вконце нетронутыми (если они используются для других целей)
#define EEPAGE_PRESERVED_TRAILING_PAGES    2



//-------------------------------------
// API модуля
//-------------------------------------


// Инициализация модуля
HAL_StatusTypeDef EEPAGE_Init(void);

// Загрузка массива данных в ОЗУ
HAL_StatusTypeDef EEPAGE_LoadData(uint8_t VirtAddress, void* Data);

// Сохранение массива данных во FLASH
HAL_StatusTypeDef EEPAGE_SaveData(uint8_t VirtAddress, void* Data);



#endif  // EEPAGE_H
