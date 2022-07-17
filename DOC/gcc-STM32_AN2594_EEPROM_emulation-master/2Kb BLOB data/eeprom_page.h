#ifndef EEPAGE_H
#define EEPAGE_H


//-------------------------------------
// ����������� ������
//-------------------------------------


// ������ ��������� ��������� ��������� ��������� �������� - ������ � ��������� �������� ������.
// ��� �������, ����������� ������� ����� �������� �� ZERO-BASED �������: VirtAddress = [0..EEPAGE_CHUNKS_COUNT-1]
#define EEPAGE_CHUNKS_COUNT   20

// ������ ������� ������ (� ������), ��������� � �������� ������ = ������ ���� ����� ������� ��������, �� ������� ���������� ��������� (��� ������� ��������������� 1 ������ FLASH = 2 ����� = 1 ���������).
// ��������: ��������� Data, ������������ � API-������, ������ ��������� �� ��� ���������� ������ ������������ ������� (EEPAGE_CHUNK_SIZE ����), ����������� � ��� (SRAM)!
// ������������: ������� ������������ ��������� ������ ��� ������
//    uint8_t  Data[EEPAGE_CHUNK_SIZE];  
//    uint16_t Data[EEPAGE_CHUNK_SIZE/2];
//    uint32_t Data[EEPAGE_CHUNK_SIZE / sizeof(uint32_t)];
#define EEPAGE_CHUNK_SIZE   (FLASH_PAGE_SIZE - sizeof(uint16_t))


// ��������������: ��������� �� ���� FLASH, ������� ��������� ������� ������ ������ ����������� (���� ��� ������������ ��� ������ �����)
#define EEPAGE_PRESERVED_TRAILING_PAGES    2



//-------------------------------------
// API ������
//-------------------------------------


// ������������� ������
HAL_StatusTypeDef EEPAGE_Init(void);

// �������� ������� ������ � ���
HAL_StatusTypeDef EEPAGE_LoadData(uint8_t VirtAddress, void* Data);

// ���������� ������� ������ �� FLASH
HAL_StatusTypeDef EEPAGE_SaveData(uint8_t VirtAddress, void* Data);



#endif  // EEPAGE_H
