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

#include "Init.h"

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
#define W25Q_TIMEOUT_US             (12U)
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
#define W25Q_4KB_ER_TIME_US         (400000UL)
#define W25Q_32KB_ER_TIME_US        (1600000UL)
#define W25Q_64KB_ER_TIME_US        (2000000UL)
#define W25Q_CHIP_ER_TIME_US        (200000000UL)

//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

void (*pW25Q_Delay)(uint32_t us);


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
    // Init GPIO
    // SCK
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin  = W25Q_PIN_SCK;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(W25Q_GPIO_PORT_SCK, &GPIO_InitStruct);
    GPIO_PinAFConfig(W25Q_GPIO_PORT_SCK, W25Q_SCK_PinSource, W25Q_GPIO_AF);

    // MISO
    GPIO_InitStruct.GPIO_Pin  = W25Q_PIN_MISO;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(W25Q_GPIO_PORT_MISO, &GPIO_InitStruct);
    GPIO_PinAFConfig(W25Q_GPIO_PORT_MISO, W25Q_SCK_PinSource, W25Q_GPIO_AF);

    // MOSI
    GPIO_InitStruct.GPIO_Pin  = W25Q_PIN_MOSI;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(W25Q_GPIO_PORT_MOSI, &GPIO_InitStruct);
    GPIO_PinAFConfig(W25Q_GPIO_PORT_MOSI, W25Q_SCK_PinSource, W25Q_GPIO_AF);

    // CS
    GPIO_InitStruct.GPIO_Pin  = W25Q_PIN_CS;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(W25Q_GPIO_PORT_CS, &GPIO_InitStruct);

    // Init SPI
    SPI_InitTypeDef SPI_InitStruct;
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial = 7;
    SPI_Init(W25Q_SPI,&SPI_InitStruct);

    SPI_Cmd(W25Q_SPI, ENABLE);

    pW25Q_Delay = W25Q_Delay;
}// end of W25Q_Init()



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
    uint8_t dataPut[W25Q_SIZE_CMD_WORD_BYTES+W25Q_SIZE_ADR_WORD_BYTES];

    //check BUSY W25Q
    if (RESULT_OK == W25Q_ReadStatusReg(W25Q_STATUS_REG1,status))
    {
        if ((status & W25Q_REG1_BUSY_BIT) == 0)
        {
            dataPut[0] = (uint8_t)W25Q_CMD_READ_DATA;
            dataPut[1] = (uint8_t)(adr>>2);
            dataPut[2] = (uint8_t)(adr>>1);
            dataPut[3] = (uint8_t)adr;

            if (RESULT_NOT_OK == W25Q_ReadWriteSPI(dataPut, W25Q_SIZE_CMD_WORD_BYTES + W25Q_SIZE_ADR_WORD_BYTES, \
                                        data, len))
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
    uint32_t len_write=0;
    uint8_t status=0;
    uint32_t timeoutCnt=0;

    // check capacity
    if((adr + len) < W25Q_CAPACITY_ALL_MEMORY_BYTES)
    {
        while(len != 0)
        {
            //check BUSY W25Q
            cmd = (uint8_t) W25Q_CMD_READ_STATUS_REG_1;
            W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, &status, 1);
            timeoutCnt = 2;
            while(((status & W25Q_REG1_BUSY_BIT) == W25Q_REG1_BUSY_BIT) && (timeoutCnt!=0))
            {
                pW25Q_Delay(W25Q_PAGE_PRM_TIME_US+W25Q_PAGE_PRM_TIME_US/10);
                W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, &status, 1);
                timeoutCnt--;
            }
            if ((status & W25Q_REG1_BUSY_BIT) == W25Q_REG1_BUSY_BIT)
            {
                result = RESULT_NOT_OK;
                break;
            }
            // Write enable instruction
            cmd = (uint8_t) W25Q_CMD_WRITE_EN;
            if (RESULT_OK == W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, 0, 0))
            {
                cmd = (uint8_t) W25Q_CMD_PAGE_PROGRAM;
                len_write = W25Q_SIZE_PAGE_BYTES - (adr & 0xff);
                if (len >= len_write)
                {
                    if (RESULT_OK == W25Q_ReadWriteSPI(&cmd,
                                                   W25Q_SIZE_CMD_WORD_BYTES + W25Q_SIZE_ADR_WORD_BYTES,
                                                   data, len_write))
                    {
                        adr += len_write;
                        len -= len_write;
                    }
                    else
                    {
                        result = RESULT_NOT_OK;
                        break;
                    }
                }
                else
                {
                    if (RESULT_OK == W25Q_ReadWriteSPI(&cmd,
                                                       W25Q_SIZE_CMD_WORD_BYTES + W25Q_SIZE_ADR_WORD_BYTES,
                                                       data, len))
                    {
                        len=0;
                    }
                    else
                    {
                        result = RESULT_NOT_OK;
                        break;
                    }
                }
            }
            else
            {
                result = RESULT_NOT_OK;
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
    uint8_t dataPut[W25Q_SIZE_CMD_WORD_BYTES+W25Q_SIZE_ADR_WORD_BYTES];

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
                    case W25Q_BLOCK_MEMORY_4KB: cmd = (uint8_t)W25Q_CMD_SECTOR_ERASE;break;
                    case W25Q_BLOCK_MEMORY_32KB: cmd = (uint8_t)W25Q_CMD_BLOCK_ERASE_32;break;
                    case W25Q_BLOCK_MEMORY_64KB: cmd = (uint8_t)W25Q_CMD_BLOCK_ERASE_64;break;
                    case W25Q_BLOCK_MEMORY_ALL: cmd = (uint8_t)W25Q_CMD_CHIP_ERASE;break;
                    default:cmd = (uint8_t)W25Q_CMD_SECTOR_ERASE;
                }
                dataPut[0] = cmd;
                dataPut[1] = (uint8_t)(adr>>2);
                dataPut[2] = (uint8_t)(adr>>1);
                dataPut[3] = (uint8_t)adr;

                if (RESULT_OK == W25Q_ReadWriteSPI(dataPut, W25Q_SIZE_CMD_WORD_BYTES+W25Q_SIZE_ADR_WORD_BYTES,
                                                   0, 0))
                {
                    // wait for erasure
                    // check BUSY W25Q
                    cmd = (uint8_t) W25Q_CMD_READ_STATUS_REG_1;
                    W25Q_ReadWriteSPI(&cmd, W25Q_SIZE_CMD_WORD_BYTES, &status, 1);
                    timeoutCnt = 2;
                    while(((status & W25Q_REG1_BUSY_BIT) == W25Q_REG1_BUSY_BIT) && (timeoutCnt!=0))
                    {
                        pW25Q_Delay(W25Q_4KB_ER_TIME_US+W25Q_4KB_ER_TIME_US/10);
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
        case W25Q_STATUS_REG1: cmd = (uint8_t)W25Q_CMD_WRITE_STATUS_REG_1;break;
        case W25Q_STATUS_REG2: cmd = (uint8_t)W25Q_CMD_WRITE_STATUS_REG_2;break;
        case W25Q_STATUS_REG3: cmd = (uint8_t)W25Q_CMD_WRITE_STATUS_REG_3;break;
        default:result = RESULT_NOT_OK;
    }

    if (RESULT_OK == result)
    {
        if (RESULT_NOT_OK == W25Q_ReadWriteSPI(&cmd, 1, &status, 1))
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
            if (RESET == SPI_I2S_GetFlagStatus(W25Q_SPI,SPI_I2S_FLAG_TXE))
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
        if (i < lenPut)
        {
            SPI_I2S_SendData(W25Q_SPI, *dataPut);
            dataPut++;
        }
        else
        {
            // Send byte(garbage data)
            SPI_I2S_SendData(W25Q_SPI, 0xff);
        }

        // wait RXNE flag
        cntTimeout=0;
        while(1)
        {
            if (SET == SPI_I2S_GetFlagStatus(W25Q_SPI,SPI_I2S_FLAG_RXNE))
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
        // read byte
        if (i >= lenPut)
        {
            if (dataGet != 0)
            {
                *dataGet = SPI_I2S_ReceiveData(W25Q_SPI);
                dataGet++;
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
        if (RESET == SPI_I2S_GetFlagStatus(W25Q_SPI,SPI_I2S_FLAG_BSY))
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
            if (SET == SPI_I2S_GetFlagStatus(W25Q_SPI,SPI_I2S_FLAG_TXE))
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
        SPI_I2S_SendData(W25Q_SPI, &data);
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
        W25Q_GPIO_PORT_CS->BSRRH = W25Q_PIN_CS;
    }
    else
    {
        W25Q_GPIO_PORT_CS->BSRRL = W25Q_PIN_CS;
    }
}// end of W25Q_SPI_SetCS()


//****************************************** end of file *******************************************