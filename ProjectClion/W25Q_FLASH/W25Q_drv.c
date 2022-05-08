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
//**************************************************************************************************

// None.


//**************************************************************************************************
// Definitions of local (private) constants
//**************************************************************************************************

// W25Q128 chip commands. Standard SPI instruction.
// Write enable
#define W25Q_WRITE_EN               (0x6U)
// Volatile SR Write enable
#define W25Q_SR_WRITE_EN            (0x50U)
// Write disable
#define W25Q_WRITE_DIS              (0x04U)
// Release Power-down/ID
#define W25Q_RELEASE_POWER_DOWN     (0xABU)
// Manufacturer/Device ID
#define W25Q_DEVICE_ID              (0x90U)
// JEDEC ID
#define W25Q_JEDEC_ID               (0x9FU)
// Read unique ID
#define W25Q_READ_UNIQUE_ID         (0x4BU)
// Read data
#define W25Q_READ_DATA              (0x03U)
// Fast read
#define W25Q_FAST_READ              (0x0BU)
// Page program
#define W25Q_PAGE_PROGRAM           (0x02U)
// Sector erase 4 KB
#define W25Q_SECTOR_ERASE           (0x20U)
// Block erase 32 KB
#define W25Q_BLOCK_ERASE_32         (0x52U)
// Block erase 64 KB
#define W25Q_BLOCK_ERASE_64         (0xD8U)
// Chip erase
#define W25Q_CHIP_ERASE             (0xC7U)//0x60U
// Read status reg-1
#define W25Q_READ_STATUS_REG_1      (0x05U)
// Write status reg-1
#define W25Q_WRITE_STATUS_REG_1     (0x01U)
// Read status reg-2
#define W25Q_READ_STATUS_REG_2      (0x35U)
// Write status reg-2
#define W25Q_WRITE_STATUS_REG_2     (0x31U)
// Read status reg-3
#define W25Q_READ_STATUS_REG_3      (0x15U)
// Write status reg-3
#define W25Q_WRITE_STATUS_REG_3     (0x11U)
// Read SFDP reg
#define W25Q_READ_SFDP_REG          (0x5AU)
// Erase security reg
#define W25Q_ERASE_STY_REG          (0x44U)
// Program security reg
#define W25Q_PRM_STY_REG            (0x42U)
// Read security reg
#define W25Q_READ_STY_REG           (0x48U)
// Global block lock
#define W25Q_GLOBAL_BLOCK_LOCK      (0x7EU)
// Global block unlock
#define W25Q_GLOBAL_BLOCK_UNLOCK    (0x98U)
// Read block Lock
#define W25Q_READ_BLOCK_LOCK        (0x3DU)
// Individual block lock
#define W25Q_INL_BLOCK_LOCK         (0x36U)
// Individual block unlock
#define W25Q_INL_BLOCK_UNLOCK       (0x36U)
// Erase/Program suspend
#define W25Q_ERASE_PRM_SUSPEND      (0x75U)
// Erase/Program Resume
#define W25Q_ERASE_PRM_RESUME       (0x7AU)
// Power down
#define W25Q_POWER_DOWN             (0xB9U)
// Enable reset
#define W25Q_RST_EN                 (0x66U)
// Reset Device
#define W25Q_RST_DEVICE             (99U)


//**************************************************************************************************
// Definitions of static global (private) variables
//**************************************************************************************************

// None.


//**************************************************************************************************
// Declarations of local (private) functions
//**************************************************************************************************

// None.


//**************************************************************************************************
//==================================================================================================
// Definitions of global (public) functions
//==================================================================================================
//**************************************************************************************************

//**************************************************************************************************
// @Function      W25Q_init()
//--------------------------------------------------------------------------------------------------
// @Description   Init SPI interface.
//--------------------------------------------------------------------------------------------------
// @Notes         None.
//--------------------------------------------------------------------------------------------------
// @ReturnValue   None.
//--------------------------------------------------------------------------------------------------
// @Parameters    None.
//**************************************************************************************************
void W25Q_init(void)
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

}// end of W25Q_init()



//**************************************************************************************************
//==================================================================================================
// Definitions of local (private) functions
//==================================================================================================
//**************************************************************************************************

// None.


//****************************************** end of file *******************************************