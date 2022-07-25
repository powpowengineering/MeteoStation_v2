//**************************************************************************************************
// @Module        W25Q
// @Filename      W25Q_drv_cfg.h
//--------------------------------------------------------------------------------------------------
// @Description   Configuration of the required functionality of the W25Q module.
//
//--------------------------------------------------------------------------------------------------
// @Version       1.0.0
//--------------------------------------------------------------------------------------------------
// @Date          XX.XX.XXXX
//--------------------------------------------------------------------------------------------------
// @History       Version  Author      Comment
// XX.XX.XXXX     1.0.0    KPS         First release.
//**************************************************************************************************

#ifndef W25Q_CFG_H
#define W25Q_CFG_H



//**************************************************************************************************
// Definitions of global (public) constants
//**************************************************************************************************

// Enable/disable the development error detection feature of the program module.
// Used only for debug purposes and should be disabled in the final release.
// Valid values: ON / OFF
#define MODULE_DEVELOPMENT_ERROR_DETECTION      (OFF)

// User can enable/disable the internal diagnostic of the program module.
// Used only for debug purposes and should be disabled in the final release.
// Valid values: ON / OFF
#define MODULE_INTERNAL_DIAGNOSTICS             (OFF)

// Confugure SPI for W25Q
#define W25Q_SPI_NUM                       SPI1
#define W25Q_SPI_SCK_PIN                   GPIO_PIN_3
#define W25Q_SPI_SCK_PORT                  GPIOB
#define W25Q_SPI_MISO_PIN                  GPIO_PIN_6
#define W25Q_SPI_MISO_PORT                 GPIOA
#define W25Q_SPI_MOSI_PIN                  GPIO_PIN_7
#define W25Q_SPI_MOSI_PORT                 GPIOA
#define W25Q_SPI_CS_PIN                    GPIO_PIN_8
#define W25Q_SPI_CS_PORT                   GPIOA
#define W25Q_SPI_SCK_AF                    GPIO_AF5_SPI1
#define W25Q_SPI_MOSI_AF                   GPIO_AF5_SPI1
#define W25Q_SPI_MISO_AF                   GPIO_AF5_SPI1

// User specify pointer delay function
#define W25Q_Delay                   INIT_Delay

// Capacity W25Q
// Total memory capacity
#define W25Q_CAPACITY_ALL_MEMORY_BYTES    (16777216UL)// (134217728UL) bits
// Quantity of blocks
#define W25Q_QTY_BLOCKS                   (256UL)
// Capacity of block
#define W25Q_CAPACITY_BLOCK               (65536UL)
// Quantity sectors
#define W25Q_QTY_SECTORS                  (16*W25Q_QTY_BLOCKS)
// Capacity sector
#define W25Q_CAPACITY_SECTOR_BYTES        (4096UL)

#endif // #ifndef W25Q_CFG_H

//****************************************** end of file *******************************************
