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

// Macros for SPI structure SPL
#define W25Q_SPI                     SPI1
// Specify port SCK
#define W25Q_GPIO_PORT_SCK          (GPIOB)
// Specify port MOSI
#define W25Q_GPIO_PORT_MOSI         (GPIOB)
// Specify port MISO
#define W25Q_GPIO_PORT_MISO         (GPIOB)
// Specify port CS
#define W25Q_GPIO_PORT_CS           (GPIOB)
// Specify pin SCK
#define W25Q_PIN_SCK                (GPIO_Pin_10)
// Specify pin MOSI
#define W25Q_PIN_MOSI               (GPIO_Pin_10)
// Specify pin MISO
#define W25Q_PIN_MISO               (GPIO_Pin_10)
// Specify pin CS
#define W25Q_PIN_CS                 (GPIO_Pin_10)
// Specify SCK PinSource
#define W25Q_SCK_PinSource          (GPIO_PinSource0)
// Specify MOSI PinSource
#define W25Q_MOSI_PinSource         (GPIO_PinSource0)
// Specify MISO PinSource
#define W25Q_MISO_PinSource         (GPIO_PinSource0)
// Specify GPIO_AF
#define W25Q_GPIO_AF                (GPIO_AF_SPI1)


#endif // #ifndef W25Q_CFG_H

//****************************************** end of file *******************************************