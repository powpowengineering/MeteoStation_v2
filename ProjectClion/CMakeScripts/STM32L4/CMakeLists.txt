#=== Prepare environment ===#
cmake_minimum_required(VERSION 3.16)

# Include system environment
include(toolchain.cmake)

# Set project name
project(STM32L476RG C ASM)

# Set project composition method
# Valid values: [COMP_PER_FILES ; COMP_PER_FOLDERS]
set(PROJECT_COMPOSITION COMP_PER_FOLDERS)

# Set C standard
set(CMAKE_C_STANDARD 99)
# Set microcontroller
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)
# Binary file extension
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
# Linker script file
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/../../Config/STM32L476RGTx_FLASH.ld)
# Set startup file
set(STARTUP_FILE ${CMAKE_SOURCE_DIR}/../../StartUp/startup_stm32l476xx.s)
set(SRC_RTT_ASM_DIR ${CMAKE_SOURCE_DIR}/../../SEGGER_RTT/SEGGER_RTT_ASM_ARMv7M.S)
set(HAL "${CMAKE_SOURCE_DIR}/../../../STM32CubeL4-master")

# Adding compiler/linker options
include(compiler.cmake)

add_definitions(

        -DSTM32L476xx
        -DMQTT_DO_NOT_USE_CUSTOM_CONFIG
        )

set(FREERTOS_DIR ${CMAKE_SOURCE_DIR}/../../../STM32CubeL4-master/Middlewares/Third_Party/FreeRTOS/Source)

#file(GLOB_RECURSE USER_SOURCES "${CMAKE_SOURCE_DIR}/Users/srcStm32F4/*.c")
#file(GLOB_RECURSE E_PAPER_FONTS "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Fonts/*.c")
#file(GLOB_RECURSE E_PAPER_GUI "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/GUI/*.c")
#file(GLOB_RECURSE E_PAPER "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/e-Paper/*.c")
#file(GLOB_RECURSE E_PAPER_CONFIG "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Config/*.c")
#file(GLOB_RECURSE E_PAPER_TEST "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Examples/EPD_2in13_test.c")
#file(GLOB_RECURSE E_PAPER_GUI "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/GUI/*.c")


file(GLOB_RECURSE ONE_WIRE_SOURCES "${CMAKE_SOURCE_DIR}/../../OneWire/OneWire.c")
file(GLOB_RECURSE DS18B20_SOURCES "${CMAKE_SOURCE_DIR}/../../DS18B20/ds18b20.c")
file(GLOB_RECURSE AM2305_SOURCES "${CMAKE_SOURCE_DIR}/../../AM2305/am2305_drv.c")
file(GLOB_RECURSE RECORD_MAN "${CMAKE_SOURCE_DIR}/../../RecordManager/record_manager.c")
file(GLOB_RECURSE CheckSum_SOURCES "${CMAKE_SOURCE_DIR}/../../CheckSum/checksum.c")
file(GLOB_RECURSE PRINTF_SOURCES "${CMAKE_SOURCE_DIR}/../../printf-master/printf.c")
file(GLOB_RECURSE TERMINAL "${CMAKE_SOURCE_DIR}/../../TERMINAL/*.c")
#file(GLOB_RECURSE USART_DRIVER_SOURCES "${CMAKE_SOURCE_DIR}/USART_Driver/usart_drv.c")
file(GLOB_RECURSE W25Q_FLASH_DRIVER_SOURCES "${CMAKE_SOURCE_DIR}/../../W25Q_FLASH/W25Q_drv.c")
file(GLOB_RECURSE BMP280 "${CMAKE_SOURCE_DIR}/../../BMP2-Sensor-API-master/bmp2.c")
#file(GLOB_RECURSE TF02_PRO "${CMAKE_SOURCE_DIR}/TF02_PRO/tf02Pro_drv.c")
file(GLOB_RECURSE CORE_MQTT "${CMAKE_SOURCE_DIR}/../../coreMQTT/source/*.c")
#file(GLOB_RECURSE EEPROM_EMULATION "${CMAKE_SOURCE_DIR}/../../EEPROM_Emulation/*.c")
file(GLOB_RECURSE EEPROM_EMULATION "${CMAKE_SOURCE_DIR}/../../EEPROM_Emulation_new/Implementation/Latest/*.c")
file(GLOB_RECURSE TIME "${CMAKE_SOURCE_DIR}/../../TIME/*.c")
file(GLOB_RECURSE CIR_BUF "${CMAKE_SOURCE_DIR}/../../Circular_Buffer/Latest/*.c")
#file(GLOB_RECURSE BMP280 "${CMAKE_SOURCE_DIR}/lwgsm-0.1.0/lwgsm/src/api/*.c")
#file(GLOB_RECURSE BMP280 "${CMAKE_SOURCE_DIR}/lwgsm-0.1.0/lwgsm/src/apps/mqtt/*.c")
#file(GLOB_RECURSE BMP280 "${CMAKE_SOURCE_DIR}/lwgsm-0.1.0/lwgsm/src/lwgsm/*.c")
#file(GLOB_RECURSE BMP280 "${CMAKE_SOURCE_DIR}/lwgsm-0.1.0/lwgsm/src/system/lwgsm_ll_stm32.c")
#file(GLOB_RECURSE SYSCALL "${CMAKE_SOURCE_DIR}/lwgsm-0.1.0/syscalls.c")

file(GLOB_RECURSE MAIN_SOURCES
        "${CMAKE_SOURCE_DIR}/../../Users/src/ftoa.c"
        "${CMAKE_SOURCE_DIR}/../../Users/src/Init.c"
        "${CMAKE_SOURCE_DIR}/../../Users/src/system_stm32l4xx.c"
        "${CMAKE_SOURCE_DIR}/../../Users/src/main.c"
        "${CMAKE_SOURCE_DIR}/../../Users/src/stm32l4xx_it.c"
#        "${CMAKE_SOURCE_DIR}/../../Users/src/task_test_flash.c"
#        "${CMAKE_SOURCE_DIR}/../../Users/src/task_test_EE.c"
#        "${CMAKE_SOURCE_DIR}/../../Users/src/task_mqtt.c"
        "${CMAKE_SOURCE_DIR}/../../Users/src/task_read_sensors.c"
        "${CMAKE_SOURCE_DIR}/../../Users/src/task_master.c"
        "${CMAKE_SOURCE_DIR}/../../Users/src/task_GSM.c"
        )

file(GLOB_RECURSE FREERTOS_SOURCES
        "${FREERTOS_DIR}/CMSIS_RTOS/cmsis_os.c"
        "${FREERTOS_DIR}/portable/GCC/ARM_CM4F/port.c"
        "${FREERTOS_DIR}/portable/MemMang/heap_1.c"
        "${FREERTOS_DIR}/portable/Common/mpu_wwrappers.c"
        "${FREERTOS_DIR}/croutine.c"
        "${FREERTOS_DIR}/event_groups.c"
        "${FREERTOS_DIR}/list.c"
        "${FREERTOS_DIR}/queue.c"
        "${FREERTOS_DIR}/stream_buffer.c"
        "${FREERTOS_DIR}/tasks.c"
        "${FREERTOS_DIR}/timers.c"
)

file(
        GLOB_RECURSE
        HAL_RECURSE
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_usart.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_usart_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c"
        "${HAL}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_i2c.c"
        "${HAL}/Drivers/BSP/STM32L4xx_Nucleo/*.c"
)


aux_source_directory(
        ${CMAKE_SOURCE_DIR}/../../SEGGER_RTT
        SRC_RTT_DIR
)

#include_directories(${CMAKE_SOURCE_DIR}/Users/inc)
include_directories(${CMAKE_SOURCE_DIR}/../../OneWire)
include_directories(${CMAKE_SOURCE_DIR}/../../DS18B20)
include_directories(${CMAKE_SOURCE_DIR}/../../AM2305)
include_directories(${CMAKE_SOURCE_DIR}/../../W25Q_FLASH)
#include_directories(${CMAKE_SOURCE_DIR}/USART_Driver)
include_directories(${CMAKE_SOURCE_DIR}/../../printf-master)
include_directories(${CMAKE_SOURCE_DIR}/../../TERMINAL)
include_directories(${CMAKE_SOURCE_DIR}/../../CheckSum)
include_directories(${CMAKE_SOURCE_DIR}/../../RecordManager)
include_directories(${CMAKE_SOURCE_DIR}/../../BMP2-Sensor-API-master)
include_directories(${CMAKE_SOURCE_DIR}/../../coreMQTT/source/include)
include_directories(${CMAKE_SOURCE_DIR}/../../coreMQTT/source/interface)
#include_directories(${CMAKE_SOURCE_DIR}/TF02_PRO)
#include_directories(${CMAKE_SOURCE_DIR}/../../EEPROM_Emulation)
include_directories(${CMAKE_SOURCE_DIR}/../../EEPROM_Emulation_new)
include_directories(${CMAKE_SOURCE_DIR}/../../EEPROM_Emulation_new/Interface)
include_directories(${CMAKE_SOURCE_DIR}/../../EEPROM_Emulation_new/Implementation/Latest)
include_directories(${CMAKE_SOURCE_DIR}/../../SEGGER_RTT)
include_directories(${CMAKE_SOURCE_DIR}/../../TIME)
include_directories(${CMAKE_SOURCE_DIR}/../../Circular_Buffer/Latest)




#include_directories(${CMAKE_SOURCE_DIR}/lwgsm-0.1.0/lwgsm/src/include)
#include_directories(${CMAKE_SOURCE_DIR}/lwgsm-0.1.0/lwgsm/src/include/system)
#include_directories(${CMAKE_SOURCE_DIR}/lwgsm-0.1.0/lwgsm/src/include/system/port/cmsis_os)
#include_directories(${CMAKE_SOURCE_DIR}/lwgsm-0.1.0/snippets/include)

#include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/e-Paper")
#include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Fonts")
#include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/GUI")
#include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Config")
#include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Examples")
#include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/GUI")


include_directories("${CMAKE_SOURCE_DIR}/../../Users/inc")
include_directories("${HAL}/Inc")
include_directories("${HAL}/Drivers/STM32L4xx_HAL_Driver/Inc")
include_directories("${HAL}/Drivers/CMSIS/Device/ST/STM32L4xx/Include")
include_directories("${HAL}/Drivers/CMSIS/Core/Include")
include_directories("${HAL}/Drivers/BSP/STM32L4xx_Nucleo")
include_directories("${FREERTOS_DIR}/CMSIS_RTOS")
include_directories("${FREERTOS_DIR}/include")
include_directories("${FREERTOS_DIR}/portable/GCC/ARM_CM4F")
include_directories("${HAL}/Legacy/Inc")


add_executable( ${PROJECT_NAME}
        ${MAIN_SOURCES}
        ${HAL_RECURSE}
        ${STARTUP_FILE}
        ${RECORD_MAN}
        ${TERMINAL}
        ${ONE_WIRE_SOURCES}
#        ${USER_SOURCES}
        ${DS18B20_SOURCES}
        ${AM2305_SOURCES}
        ${BMP280}
        ${CORE_MQTT}
#        ${TF02_PRO}
#        ${USART_DRIVER_SOURCES}
#        ${HAL_SOURCES}
#        ${E_PAPER_FONTS}
#        ${E_PAPER_CONFIG}
#        ${E_PAPER}
#        ${E_PAPER_TEST}
#        ${E_PAPER_GUI}
        ${EEPROM_EMULATION}
        ${SRC_RTT_DIR}
        ${SRC_RTT_ASM_DIR}

        ${PRINTF_SOURCES}
        ${FREERTOS_SOURCES}
        ${W25Q_FLASH_DRIVER_SOURCES}
        ${CheckSum_SOURCES}
        ${TIME}
        ${CIR_BUF}
)


#=== Post-Build operations ===#

# Generate HEX-file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        ARGS -Oihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
        COMMENT "Generate ${PROJECT_NAME}.hex file"
        )

# Generate SREC-file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        ARGS -Osrec ${PROJECT_NAME}.elf ${PROJECT_NAME}.s19
        COMMENT "Generate ${PROJECT_NAME}.s19 file"
        )

# Generate BIN-file

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        ARGS -Obinary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
        COMMENT "Generate ${PROJECT_NAME}.bin file"
        )


# Show memory usage
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${GCC_SIZE}
        ARGS --format=berkeley -t ${PROJECT_NAME}.elf
        COMMENT "Memory usage:"
        )