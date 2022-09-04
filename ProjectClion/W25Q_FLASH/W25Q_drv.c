//**************************************************************************************************
// @Module        W25Q
// @Filename      W25Q_drv.c
//--------------------------------------------------------------------------------------------------
// @Platform      stm32
//--------------------------------------------------------------------------------------------------
// @Compatible    stm32
//--------------------------------------------------------------------------------------------------
// @Description   Implementation of the W25Q functionality.
//
//
//                Abbreviations:
//                  None.
//
//
//                Global (public) functions:
//
//
//                Local (private) functions:
//
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          xx.xx.xxxx
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************



//**************************************************************************************************
// Project Includes
//**************************************************************************************************

// Native header
#include "W25Q_drv.h"

// LL HAL
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_gpio.h"

#include "Init.h"
#include "FreeRTOS.h"
#include "task.h"

//**************************************************************************************************
// Verification of the imported configuration parameters
//**************************************************************************************************

// None.


//**************************************************************************************************
// Definitions of global (public) variables
//**************************************************************************************************

// None.


//**************************************************************************************************
// Declarations of local (private) data types
//*************************************************************************************************

// Level CS.
typedef enum SPI_CS_LEVEL_enum
{
    LOW =0,
    HIGH
}SPI_CS_LEVEL;

typedef struct W25Q_TIMES_ITEM_str
{
    uint32_t nTimeout;
    uint32_t nQuantityTimeout;
}W25Q_TIMES_ITEM;

typedef struct W25Q_TIMES_str
{
    W25Q_TIMES_ITEM nPageProgram;
    W25Q_TIMES_ITEM nErase4K;
    W25Q_TIMES_ITEM nErase32K;
    W25Q_TIMES_ITEM nErase64K;
    W25Q_TIMES_ITEM nEraseChip;
    W25Q_TIMES_ITEM nNone;
}W25Q_TIMES;


//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

// W25Q128 chip commands. Standard SPI instruction.
// Write enable
#define W25Q_CMD_WRITE_EN               (0x6U)
// Volatile SR Write enable
#define W25Q_CMD_SR_WRITE_EN            (0x50U)
// Write disable
#define W25Q_CMD_WRITE_DIS              (0x04U)
// Release Power-down/ID
#define W25Q_CMD_RELEASE_POWER_DOWN     (0xABU)
// Manufacturer/Device ID
#define W25Q_CMD_DEVICE_ID              (0x90U)
// JEDEC ID
#define W25Q_CMD_JEDEC_ID               (0x9FU)
// Read unique ID
#define W25Q_CMD_READ_UNIQUE_ID         (0x4BU)
// Read data
#define W25Q_CMD_READ_DATA              (0x03U)
// Fast read
#define W25Q_CMD_FAST_READ              (0x0BU)
// Page program
#define W25Q_CMD_PAGE_PROGRAM           (0x02U)
// Sector erase 4 KB
#define W25Q_CMD_SECTOR_ERASE           (0x20U)
// Block erase 32 KB
#define W25Q_CMD_BLOCK_ERASE_32         (0x52U)
// Block erase 64 KB
#define W25Q_CMD_BLOCK_ERASE_64         (0xD8U)
// Chip erase
#define W25Q_CMD_CHIP_ERASE             (0xC7U)//0x60U
// Read status reg-1
#define W25Q_CMD_READ_STATUS_REG_1      (0x05U)
// Write status reg-1
#define W25Q_CMD_WRITE_STATUS_REG_1     (0x01U)
// Read status reg-2
#define W25Q_CMD_READ_STATUS_REG_2      (0x35U)
// Write status reg-2
#define W25Q_CMD_WRITE_STATUS_REG_2     (0x31U)
// Read status reg-3
#define W25Q_CMD_READ_STATUS_REG_3      (0x15U)
// Write status reg-3
#define W25Q_CMD_WRITE_STATUS_REG_3     (0x11U)
// Read SFDP reg
#define W25Q_CMD_READ_SFDP_REG          (0x5AU)
// Erase security reg
#define W25Q_CMD_ERASE_STY_REG          (0x44U)
// Program security reg
#define W25Q_CMD_PRM_STY_REG            (0x42U)
// Read security reg
#define W25Q_CMD_READ_STY_REG           (0x48U)
// Global block lock
#define W25Q_CMD_GLOBAL_BLOCK_LOCK      (0x7EU)
// Global block unlock
#define W25Q_CMD_GLOBAL_BLOCK_UNLOCK    (0x98U)
// Read block Lock
#define W25Q_CMD_READ_BLOCK_LOCK        (0x3DU)
// Individual block lock
#define W25Q_CMD_INL_BLOCK_LOCK         (0x36U)
// Individual block unlock
#define W25Q_CMD_INL_BLOCK_UNLOCK       (0x36U)
// Erase/Program suspend
#define W25Q_CMD_ERASE_PRM_SUSPEND      (0x75U)
// Erase/Program Resume
#define W25Q_CMD_ERASE_PRM_RESUME       (0x7AU)
// Power down
#define W25Q_CMD_POWER_DOWN             (0xB9U)
// Enable reset
#define W25Q_CMD_RST_EN                 (0x66U)
// Reset Device
#define W25Q_CMD_RST_DEVICE             (99U)


// timeout value in US
#define W25Q_TIMEOUT_US             (1U)
// Quantity timeouts
#define W25Q_QTY_TIMEOUT            (10U)
// Status reg number
#define W25Q_STATUS_REG1            (0)
#define W25Q_STATUS_REG2            (1U)
#define W25Q_STATUS_REG3            (2U)

// Bit definition status reg-1
#define W25Q_REG1_BUSY_BIT          (1<<0)
#define W25Q_REG1_WEL_BIT           (1<<1U)
#define W25Q_REG1_BP0_BIT           (1<<2U)
#define W25Q_REG1_BP1_BIT           (1<<3U)
#define W25Q_REG1_BP2_BIT           (1<<4U)
#define W25Q_REG1_TB_BIT            (1<<5U)
#define W25Q_REG1_SEC_BIT           (1<<6U)
#define W25Q_REG1_SRP_BIT           (1<<7U)

#define W25Q_SIZE_ADR_WORD_BYTES    (3U)
#define W25Q_SIZE_CMD_WORD_BYTES    (1U)
// Page size bytes
#define W25Q_SIZE_PAGE_BYTES        (256U)

// Timings
#define W25Q_PAGE_PRM_TIME_US       (3000UL)
#define W25Q_4KB_ER_TIME_US         (10000UL)//(400000UL)
#define W25Q_32KB_ER_TIME_US        (1600000UL)
#define W25Q_64KB_ER_TIME_US        (2000000UL)
#define W25Q_CHIP_ER_TIME_US        (200000000UL)
#define W25Q_NONE_TIME_US           (10000UL)

#define W25Q_PAGE_PRM_QTY_TIMEOUT       (10U)
#define W25Q_ER_4K_QTY_TIMEOUT          (10U)
#define W25Q_ER_32K_QTY_TIMEOUT         (5U)
#define W25Q_ER_64K_QTY_TIMEOUT         (5U)
#define W25Q_ER_CHIP_QTY_TIMEOUT        (5U)
#define W25Q_NONE_QTY_TIMEOUT           (20U)



//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

void (*pW25Q_Delay)(uint32_t us);
static const W25Q_TIMES W25Q_Times= {
        {W25Q_PAGE_PRM_TIME_US,W25Q_PAGE_PRM_QTY_TIMEOUT},
        {W25Q_4KB_ER_TIME_US,W25Q_ER_4K_QTY_TIMEOUT},
        {W25Q_32KB_ER_TIME_US,W25Q_ER_32K_QTY_TIMEOUT},
        {W25Q_64KB_ER_TIME_US,W25Q_ER_64K_QTY_TIMEOUT},
        {W25Q_CHIP_ER_TIME_US,W25Q_ER_CHIP_QTY_TIMEOUT},
        {W25Q_NONE_TIME_US,W25Q_NONE_QTY_TIMEOUT}
        };

// SPI handler
SPI_HandleTypeDef SpiHandle;



//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// Set CS pin level.
static void W25Q_SPI_SetCS(SPI_CS_LEVEL csLevel);
// Read data from SPI.
static STD_RESULT W25Q_ReadWriteSPI(uint8_t *dataPut, const uint32_t lenPut, uint8_t *dataGet, uint32_t lenGet);
// read status regs.
static STD_RESULT W25Q_ReadStatusReg(const uint8_t regNumber,uint8_t *const status);
// write spi data.
static STD_RESULT W25Q_WriteSPI(uint8_t *data, const uint32_t len);



//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      W25Q_Init()
//--------------------------------------------------------------------------------------------------
// @Description   Init SPI interface.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void W25Q_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct = {0};

    // Configure the CS pin SPI for W25Q
    GPIO_InitStruct.Pin   = W25Q_SPI_CS_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(W25Q_SPI_CS_PORT, &GPIO_InitStruct);

    // Configure the SCK pin SPI for W25Q
    GPIO_InitStruct.Pin        = W25Q_SPI_SCK_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLUP;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = W25Q_SPI_SCK_AF;
    HAL_GPIO_Init(W25Q_SPI_SCK_PORT, &GPIO_InitStruct);

    // Configure the MOSI pin SPI for W25Q
    GPIO_InitStruct.Pin        = W25Q_SPI_MOSI_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = W25Q_SPI_MOSI_AF;
    HAL_GPIO_Init(W25Q_SPI_MOSI_PORT, &GPIO_InitStruct);

    // Configure the MISO pin SPI for W25Q
    GPIO_InitStruct.Pin        = W25Q_SPI_MISO_PIN;
    GPIO_InitStruct.Mode       = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull       = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed      = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate  = W25Q_SPI_MISO_AF;
    HAL_GPIO_Init(W25Q_SPI_MISO_PORT, &GPIO_InitStruct);

    // Init SPI
    SpiHandle.Instance               = W25Q_SPI_NUM;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;
    SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
    SpiHandle.Init.Mode              = SPI_MODE_MASTER;
    HAL_SPI_Init(&SpiHandle);

    LL_SPI_Enable(SpiHandle.Instance);

    pW25Q_Delay = W25Q_Delay;
}// end of W25Q_Init()



//**************************************************************************************************
// @Function      W25Q_ReadUniqueID()
//--------------------------------------------------------------------------------------------------
// @Description   Read Unique ID W25Q.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    ID - get Id W25Q
//**************************************************************************************************
STD_RESULT W25Q_ReadUniqueID(uint64_t* ID)
{
    STD_RESULT result = RESULT_OK;

    uint8_t dataPut[W25Q_SIZE_CMD_WORD_BYTES+4U];

    dataPut[0] = (uint8_t)W25Q_CMD_READ_UNIQUE_ID;
    dataPut[1] = (uint8_t)0xff;
    dataPut[2] = (uint8_t)0xff;
    dataPut[3] = (uint8_t)0xff;
    dataPut[4] = (uint8_t)0xff;

    result = W25Q_ReadWriteSPI(dataPut,5U, (uint8_t*)ID, 8U);

    return result;
}//end of W25Q_ReadUniqueID

//**************************************************************************************************
// @Function      W25Q_ReadManufactureID()
//--------------------------------------------------------------------------------------------------
// @Description   Read Manufacture ID
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    ID - get Manufacture Id W25Q
//**************************************************************************************************
STD_RESULT W25Q_ReadManufactureID(uint16_t* ID)
{
    STD_RESULT result = RESULT_OK;

    uint8_t dataPut[W25Q_SIZE_CMD_WORD_BYTES+4U];

    dataPut[0] = (uint8_t)W25Q_CMD_DEVICE_ID;
    dataPut[1] = (uint8_t)(0);
    dataPut[2] = (uint8_t)(0);
    dataPut[3] = (uint8_t)(0);

    result = W25Q_ReadWriteSPI(dataPut,4U, (uint8_t*)ID, 2U);

    return result;
}// W25Q_ReadManufactureID


//**************************************************************************************************
// @Function      W25Q_ReadData()
//--------------------------------------------------------------------------------------------------
// @Description   Read W25Q Flash Data.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    adr - absolute address flash memory
//                data - pointer data
//                len - length data
//**************************************************************************************************
STD_RESULT W25Q_ReadData(const uint32_t adr,uint8_t* data, const uint32_t len)
{
    STD_RESULT result = RESULT_OK;
    uint8_t status=0;
    uint8_t cmd = 0;
    uint32_t timeoutCnt=0;
    uint8_t dataPut[W25Q_SIZE_CMD_WORD_BYTES+W25Q_SIZE_ADR_WORD_BYTES];

    //check BUSY W25Q
    cmd = (uint8_t) W25Q_CMD_READ_STATUS_REG_1;
    status = 0xff;
    for(uint32_t nQtyTimeout=0;nQtyTimeout < W25Q_Times.nNone.nQuantityTimeout;nQtyTimeout++)
    {
        if (RESULT_OK == W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, &status, 1))
        {
            if ((status & W25Q_REG1_BUSY_BIT) == 0)
            {
                // Read data
                dataPut[0] = (uint8_t)W25Q_CMD_READ_DATA;
                dataPut[1] = (uint8_t)(adr>>16);
                dataPut[2] = (uint8_t)(adr>>8);
                dataPut[3] = (uint8_t)adr;

                if (RESULT_NOT_OK == W25Q_ReadWriteSPI(dataPut,
                                                       W25Q_SIZE_CMD_WORD_BYTES + W25Q_SIZE_ADR_WORD_BYTES, \
                                                data,
                                                len))
                {
                    result = RESULT_NOT_OK;
                }
                break;
            }
            else
            {
                pW25Q_Delay(W25Q_Times.nNone.nTimeout+W25Q_Times.nNone.nTimeout/10);
            }
        }
    }

    return result;

}// end of W25Q_ReadData()



//**************************************************************************************************
// @Function      W25Q_WriteData()
//--------------------------------------------------------------------------------------------------
// @Description   Write W25Q Flash Data.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    adr - absolute address flash memory
//                data - pointer data
//                len - length data
//**************************************************************************************************
STD_RESULT W25Q_WriteData(uint32_t adr,uint8_t* data, uint32_t len)
{
    STD_RESULT result = RESULT_OK;
    uint8_t cmd = 0;
    uint8_t dataPut[W25Q_SIZE_CMD_WORD_BYTES+W25Q_SIZE_ADR_WORD_BYTES];
    uint32_t len_write=0;
    uint8_t status=0;
    uint32_t timeoutCnt=0;
    uint32_t indexBuf=0;

    // check capacity
    if((adr + len - 1U) < W25Q_CAPACITY_ALL_MEMORY_BYTES)
    {
        while(len != 0)
        {
            //check BUSY W25Q
            cmd = (uint8_t) W25Q_CMD_READ_STATUS_REG_1;
            status = 0xff;
            for(uint32_t nQtyTimeout=0;nQtyTimeout < W25Q_Times.nPageProgram.nQuantityTimeout;nQtyTimeout++)
            {
                if (RESULT_OK ==  W25Q_ReadWriteSPI(&cmd,
                                                    W25Q_SIZE_CMD_WORD_BYTES,
                                                    &status,
                                                    1))
                {
                    if ((status & W25Q_REG1_BUSY_BIT) == 0U)
                    {
                        // Write enable instruction
                        cmd = (uint8_t) W25Q_CMD_WRITE_EN;
                        if (RESULT_OK == W25Q_ReadWriteSPI(&cmd,
                                                           W25Q_SIZE_CMD_WORD_BYTES,
                                                           0,
                                                           0))
                        {
                            cmd = (uint8_t) W25Q_CMD_PAGE_PROGRAM;
                            len_write = W25Q_SIZE_PAGE_BYTES - (adr & 0xff);
                            if (len >= len_write)
                            {
                                dataPut[0] = cmd;
                                dataPut[1] = (uint8_t)(adr>>16);
                                dataPut[2] = (uint8_t)(adr>>8);
                                dataPut[3] = (uint8_t)adr;
                                if (RESULT_OK == W25Q_ReadWriteSPI(dataPut,
                                                                   W25Q_SIZE_CMD_WORD_BYTES + W25Q_SIZE_ADR_WORD_BYTES,
                                                                   &data[indexBuf],
                                                                   len_write))
                                {
                                    adr += len_write;
                                    len -= len_write;
                                    indexBuf += len_write;
                                    break;
                                }
                            }
                            else
                            {
                                dataPut[0] = cmd;
                                dataPut[1] = (uint8_t)(adr>>16);
                                dataPut[2] = (uint8_t)(adr>>8);
                                dataPut[3] = (uint8_t)adr;
                                if (RESULT_OK == W25Q_ReadWriteSPI(dataPut,
                                                                   W25Q_SIZE_CMD_WORD_BYTES + W25Q_SIZE_ADR_WORD_BYTES,
                                                                   &data[indexBuf],
                                                                   len))
                                {
                                    len=0;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        pW25Q_Delay(W25Q_Times.nPageProgram.nTimeout + W25Q_Times.nPageProgram.nTimeout/10);
                    }
                }
            }
            if ((status & W25Q_REG1_BUSY_BIT) == W25Q_REG1_BUSY_BIT)
            {
                result = RESULT_NOT_OK;
            }
            if (RESULT_NOT_OK == result)
            {
                break;
            }
        }
    }
    else
    {
        result = RESULT_NOT_OK;
    }

    return result;

}//end of W25Q_WriteData()


//**************************************************************************************************
// @Function      W25Q_EraseBlock()
//--------------------------------------------------------------------------------------------------
// @Description   Erase block 4KB,32KB,64KB or all memory.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    adr - absolute address flash memory.
//                typeBlock - W25Q_BLOCK_MEMORY_4KB,W25Q_BLOCK_MEMORY_32KB,W25Q_BLOCK_MEMORY_64KB,
//                            W25Q_BLOCK_MEMORY_ALL
//**************************************************************************************************
STD_RESULT W25Q_EraseBlock(const uint32_t adr, W25Q_TYPE_BLOCKS typeBlock)
{
    STD_RESULT result = RESULT_OK;
    uint8_t cmd = 0;
    uint8_t status=0;
    uint32_t timeoutCnt=0;
    uint32_t timeErase=0;
    uint8_t dataPut[W25Q_SIZE_CMD_WORD_BYTES+W25Q_SIZE_ADR_WORD_BYTES];
    uint8_t lenWrite=0;

    // check adr
    if(adr < W25Q_CAPACITY_ALL_MEMORY_BYTES)
    {
        //check BUSY W25Q
        cmd = (uint8_t) W25Q_CMD_READ_STATUS_REG_1;
        W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, &status, 1);
        if ((status & W25Q_REG1_BUSY_BIT) == 0)
        {
            // Write enable instruction
            cmd = (uint8_t) W25Q_CMD_WRITE_EN;
            if (RESULT_OK == W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, 0, 0))
            {
                // erase BLOCK
                switch(typeBlock)
                {
                    case W25Q_BLOCK_MEMORY_4KB:
                        cmd = (uint8_t)W25Q_CMD_SECTOR_ERASE;
                        timeErase = W25Q_Times.nErase4K.nTimeout + W25Q_Times.nErase4K.nTimeout / 10;
                        timeoutCnt = W25Q_Times.nErase4K.nQuantityTimeout;
                        lenWrite = W25Q_SIZE_CMD_WORD_BYTES+W25Q_SIZE_ADR_WORD_BYTES;
                        break;
                    case W25Q_BLOCK_MEMORY_32KB:
                        cmd = (uint8_t)W25Q_CMD_BLOCK_ERASE_32;
                        timeErase = W25Q_Times.nErase32K.nTimeout + W25Q_Times.nErase32K.nTimeout / 10;
                        timeoutCnt = W25Q_Times.nErase32K.nQuantityTimeout;
                        lenWrite = W25Q_SIZE_CMD_WORD_BYTES+W25Q_SIZE_ADR_WORD_BYTES;
                        break;
                    case W25Q_BLOCK_MEMORY_64KB:
                        cmd = (uint8_t)W25Q_CMD_BLOCK_ERASE_64;
                        timeErase = W25Q_Times.nErase64K.nTimeout + W25Q_Times.nErase64K.nTimeout / 10;
                        timeoutCnt = W25Q_Times.nErase64K.nQuantityTimeout;
                        lenWrite = W25Q_SIZE_CMD_WORD_BYTES+W25Q_SIZE_ADR_WORD_BYTES;
                        break;
                    case W25Q_BLOCK_MEMORY_ALL:
                        cmd = (uint8_t)W25Q_CMD_CHIP_ERASE;
                        timeErase = W25Q_Times.nEraseChip.nTimeout + W25Q_Times.nEraseChip.nTimeout / 10;
                        timeoutCnt = W25Q_Times.nEraseChip.nQuantityTimeout;
                        lenWrite = W25Q_SIZE_CMD_WORD_BYTES;
                        break;
                    default:cmd = (uint8_t)W25Q_CMD_SECTOR_ERASE;
                }
                dataPut[0] = cmd;
                dataPut[1] = (uint8_t)(adr>>16);
                dataPut[2] = (uint8_t)(adr>>8);
                dataPut[3] = (uint8_t)adr;

                if (RESULT_OK == W25Q_ReadWriteSPI(dataPut, lenWrite,0, 0))
                {
                    // wait for erasure
                    // check BUSY W25Q
                    cmd = (uint8_t) W25Q_CMD_READ_STATUS_REG_1;
                    W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, &status, 1);
                    while(((status & W25Q_REG1_BUSY_BIT) == W25Q_REG1_BUSY_BIT) && (timeoutCnt!=0))
                    {
                        pW25Q_Delay(timeErase);
                        W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, &status, 1);
                        timeoutCnt--;
                    }
                    if ((status & W25Q_REG1_BUSY_BIT) == 0)
                    {
                        result = RESULT_OK;
                    }
                    else
                    {
                        result = RESULT_NOT_OK;
                    }
                }
                else
                {
                    result = RESULT_NOT_OK;
                }
            }
            else
            {
                result = RESULT_NOT_OK;
            }
        }
        else
        {
            result = RESULT_NOT_OK;
        }
    }
    else
    {
        result = RESULT_NOT_OK;
    }

    return result;

}// end of W25Q_EraseBlock()



//**************************************************************************************************
// @Function      W25Q_GetLock()
//--------------------------------------------------------------------------------------------------
// @Description   Get Block/Sector Lock
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    adr - address of block
//                lock - get lock byte
//**************************************************************************************************
STD_RESULT W25Q_GetLock(const uint32_t adr, uint8_t *const lock)
{
    STD_RESULT result = RESULT_OK;
    uint8_t status=0;
    uint8_t cmd = 0;
    uint32_t timeoutCnt=0;
    uint8_t dataPut[W25Q_SIZE_CMD_WORD_BYTES+W25Q_SIZE_ADR_WORD_BYTES];

    //check BUSY W25Q
    cmd = (uint8_t) W25Q_CMD_READ_STATUS_REG_1;
    status = 0xff;
    for(uint32_t nQtyTimeout=0;nQtyTimeout < W25Q_Times.nNone.nQuantityTimeout;nQtyTimeout++)
    {
        if (RESULT_OK == W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, &status, 1))
        {
            if ((status & W25Q_REG1_BUSY_BIT) == 0)
            {
                // Read data
                dataPut[0] = (uint8_t)W25Q_CMD_READ_BLOCK_LOCK;
                dataPut[1] = (uint8_t)(adr>>16);
                dataPut[2] = (uint8_t)(adr>>8);
                dataPut[3] = (uint8_t)adr;

                if (RESULT_NOT_OK == W25Q_ReadWriteSPI(dataPut,
                                                       W25Q_SIZE_CMD_WORD_BYTES + W25Q_SIZE_ADR_WORD_BYTES, \
                                                lock,
                                                       1U))
                {
                    result = RESULT_NOT_OK;
                }
                break;
            }
            else
            {
                pW25Q_Delay(W25Q_Times.nNone.nTimeout+W25Q_Times.nNone.nTimeout/10);
            }
        }
    }

    return result;
}// end of W25Q_GetLock



//**************************************************************************************************
// @Function      W25Q_UnLockGlobal()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
STD_RESULT W25Q_UnLockGlobal(void)
{
    STD_RESULT result = RESULT_OK;
    uint8_t status = 0;
    uint8_t cmd = 0;
    uint32_t timeoutCnt = 0;
    uint8_t dataPut = 0;

    //check BUSY W25Q
    cmd = (uint8_t) W25Q_CMD_READ_STATUS_REG_1;
    status = 0xff;
    for(uint32_t nQtyTimeout=0;nQtyTimeout < W25Q_Times.nNone.nQuantityTimeout;nQtyTimeout++)
    {
        if (RESULT_OK == W25Q_ReadWriteSPI(&cmd,
                                           W25Q_SIZE_CMD_WORD_BYTES,
                                           &status,
                                           1))
        {
            // Write enable instruction
            cmd = (uint8_t) W25Q_CMD_WRITE_EN;
            if (RESULT_OK == W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, 0, 0))
            {
                if ((status & W25Q_REG1_BUSY_BIT) == 0)
                {
                    // Write cmd
                    dataPut = (uint8_t) W25Q_CMD_GLOBAL_BLOCK_UNLOCK;

                    if (RESULT_NOT_OK == W25Q_ReadWriteSPI(&dataPut,
                                                           W25Q_SIZE_CMD_WORD_BYTES,
                                                           0,
                                                           0))
                    {
                        result = RESULT_NOT_OK;
                    }
                    break;
                }
            }
        }
        else
        {
            pW25Q_Delay(W25Q_Times.nNone.nTimeout + W25Q_Times.nNone.nTimeout / 10);
        }
    }

    return result;
}// end of W25Q_UnLockGlobal



//**************************************************************************************************
// @Function      W25Q_IsAreaBlank()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
STD_RESULT W25Q_IsAreaBlank(const uint32_t nAddress, const uint32_t nSize, BOOLEAN *const bResult)
{
    STD_RESULT nResult = RESULT_OK;
    uint32_t nAdr = nAddress;
    uint8_t data = 0U;
    *bResult = TRUE;

    for (uint32_t i = 0; i < nSize; i++)
    {
        if (RESULT_OK == W25Q_ReadData(nAdr,&data, 1U))
        {
            if (0xFF != data)
            {
                *bResult = FALSE;
            }
        }
        else
        {
            nResult = RESULT_NOT_OK;
            break;
        }
    }

    return nResult;
} // end of W25Q_IsAreaBlank()


//**************************************************************************************************
// @Function      W25Q_PowerDown()
//--------------------------------------------------------------------------------------------------
// @Description   None.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
STD_RESULT W25Q_PowerDown(void)
{
    STD_RESULT enResult = RESULT_NOT_OK;
    uint8_t cmd = 0;
    uint8_t status = 0;

    //check BUSY W25Q
    cmd = (uint8_t) W25Q_CMD_READ_STATUS_REG_1;
    status = 0xff;

    if (RESULT_OK ==  W25Q_ReadWriteSPI(&cmd,
                                        W25Q_SIZE_CMD_WORD_BYTES,
                                        &status,
                                        1))
    {
        if ((status & W25Q_REG1_BUSY_BIT) == 0U)
        {
            // Power down instruction
            cmd = (uint8_t) W25Q_CMD_POWER_DOWN;
            if (RESULT_OK == W25Q_ReadWriteSPI(&cmd,
                                               W25Q_SIZE_CMD_WORD_BYTES,
                                               0,
                                               0))
            {
                enResult = RESULT_OK;
            }
            else
            {
                enResult = RESULT_NOT_OK;
            }
        }
        else
        {
            enResult = RESULT_NOT_OK;
        }
    }
    else
    {
        enResult = RESULT_NOT_OK;
    }


    return enResult;
} // end of W25Q_PowerDown()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************



//**************************************************************************************************
// @Function      W25Q_ReadStatusReg()
//--------------------------------------------------------------------------------------------------
// @Description   Reading W25Q status reg-1 or reg-2 or reg-3.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    status - status reg-1,2,3
//**************************************************************************************************
static STD_RESULT W25Q_ReadStatusReg(const uint8_t regNumber,uint8_t *const status)
{
    STD_RESULT result = RESULT_OK;
    uint8_t cmd=0;

    switch(regNumber)
    {
        case W25Q_STATUS_REG1: cmd = (uint8_t)W25Q_CMD_READ_STATUS_REG_1;break;
        case W25Q_STATUS_REG2: cmd = (uint8_t)W25Q_CMD_READ_STATUS_REG_2;break;
        case W25Q_STATUS_REG3: cmd = (uint8_t)W25Q_CMD_READ_STATUS_REG_3;break;
        default:result = RESULT_NOT_OK;
    }

    if (RESULT_OK == result)
    {
        if (RESULT_NOT_OK == W25Q_ReadWriteSPI(&cmd, 1, status, 1))
        {
            result = RESULT_NOT_OK;
        }
    }

    return result;
}// end of W25Q_ReadStatusReg()



//**************************************************************************************************
// @Function      W25Q_ReadWriteSPI()
//--------------------------------------------------------------------------------------------------
// @Description   Read SPI data.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    data - pointer data.
//                len - length data.
//**************************************************************************************************
static STD_RESULT W25Q_ReadWriteSPI(uint8_t *dataPut, const uint32_t lenPut, uint8_t *dataGet, uint32_t lenGet)
{
    STD_RESULT result = RESULT_OK;
    uint32_t cntTimeout=0;

    // Set CS Low
    W25Q_SPI_SetCS(LOW);

    for (int i=0;i<lenPut+lenGet;i++)
    {
        // wait TXE flag
        cntTimeout=0;
        while(1)
        {
            if (SET == LL_SPI_IsActiveFlag_TXE(W25Q_SPI_NUM))
            {
                break;
            }
            if (cntTimeout > W25Q_QTY_TIMEOUT)
            {
                result = RESULT_NOT_OK;
                break;
            }
            pW25Q_Delay(W25Q_TIMEOUT_US);
            cntTimeout++;
        }
        if (RESULT_NOT_OK == result)
        {
            break;
        }
         // Clear RXNE
        LL_SPI_ReceiveData8(W25Q_SPI_NUM);

        if (i < lenPut)
        {
            taskENTER_CRITICAL();
            LL_SPI_TransmitData8(W25Q_SPI_NUM, *dataPut);
            taskEXIT_CRITICAL();
            dataPut++;
        }
        else
        {
            taskENTER_CRITICAL();
            // Send byte(garbage data)
            LL_SPI_TransmitData8(W25Q_SPI_NUM, *dataGet);
            taskEXIT_CRITICAL();
        }
        // read byte

        // wait RXNE flag
        cntTimeout=0;
        while(1)
        {
            if (SET == LL_SPI_IsActiveFlag_RXNE(W25Q_SPI_NUM))
            {
                // set test pin
//                GPIOA->BSRRL = GPIO_Pin_1;
                break;
            }
            if (cntTimeout > W25Q_QTY_TIMEOUT)
            {
                result = RESULT_NOT_OK;
                break;
            }
            pW25Q_Delay(W25Q_TIMEOUT_US);
            cntTimeout++;
        }
        if (RESULT_NOT_OK == result)
        {
            break;
        }
        if (i >= lenPut)
        {
            if (dataGet != 0)
            {
                *dataGet = LL_SPI_ReceiveData8(W25Q_SPI_NUM);
                dataGet++;
                //test pin low
//                GPIOA->BSRRH = GPIO_Pin_1;
            }
            else
            {
                result = RESULT_NOT_OK;
            }
        }

        if (RESULT_NOT_OK == result)
        {
            break;
        }
    }

    // wait BUSY SPI
    cntTimeout=0;
    while(1)
    {
        if (RESET == LL_SPI_IsActiveFlag_BSY(W25Q_SPI_NUM))
        {
            break;
        }
        if (cntTimeout > W25Q_QTY_TIMEOUT)
        {
            result = RESULT_NOT_OK;
            break;
        }
        pW25Q_Delay(W25Q_TIMEOUT_US);
        cntTimeout++;
    }

    // Set CS High
    W25Q_SPI_SetCS(HIGH);

    return result;
}// end of W25Q_ReadWriteSPI()



//**************************************************************************************************
// @Function      W25Q_WriteSPI()
//--------------------------------------------------------------------------------------------------
// @Description   Write SPI data.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    data - pointer data.
//                len - length data.
//**************************************************************************************************
static STD_RESULT W25Q_WriteSPI(uint8_t *data, const uint32_t len)
{
    STD_RESULT result = RESULT_OK;
    uint32_t cntTimeout=0;

    for (int i=0;i<len;i++)
    {
        // wait TXE bit
        cntTimeout=0;
        while(1)
        {
            if (SET == LL_SPI_IsActiveFlag_TXE(W25Q_SPI_NUM))
            {
                break;
            }
            if (cntTimeout > W25Q_QTY_TIMEOUT)
            {
                result = RESULT_NOT_OK;
                break;
            }
            pW25Q_Delay(W25Q_TIMEOUT_US);
            cntTimeout++;
        }
        if (RESULT_NOT_OK == result)
        {
            break;
        }
        // Send byte
        LL_SPI_TransmitData8(W25Q_SPI_NUM, *data);
        data++;
    }

    return result;
}// end of W25Q_WriteSPI()



//**************************************************************************************************
// @Function      W25Q_SPI_SetCS()
//--------------------------------------------------------------------------------------------------
// @Description   Setting CS level.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    csLevel - level CS. LOW or HIGH.
//**************************************************************************************************
static void W25Q_SPI_SetCS(SPI_CS_LEVEL csLevel)
{
    if (LOW == csLevel)
    {
        LL_GPIO_ResetOutputPin(W25Q_SPI_CS_PORT,W25Q_SPI_CS_PIN);
    }
    else
    {
        LL_GPIO_SetOutputPin(W25Q_SPI_CS_PORT,W25Q_SPI_CS_PIN);
    }
}// end of W25Q_SPI_SetCS()


//****************************************** end of file *******************************************