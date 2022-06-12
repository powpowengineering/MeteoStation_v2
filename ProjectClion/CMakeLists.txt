#=== Prepare environment ===#
cmake_minimum_required(VERSION 3.17)

# Include system environment
include(toolchain.cmake)

# Set project name
project(RAK811 C ASM)

# Set project composition method
# Valid values: [COMP_PER_FILES ; COMP_PER_FOLDERS]
set(PROJECT_COMPOSITION COMP_PER_FOLDERS)

# Set C standard
set(CMAKE_C_STANDARD 99)
# Set microcontroller
set(CMAKE_SYSTEM_PROCESSOR cortex-m3)
#set(CMAKE_SYSTEM_PROCESSOR cortex-m4)
# Binary file extension
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
# Linker script file
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/Config/STM32L151XB_FLASH.ld)
#set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/Config/stm32f4xx_flash.ld)

# Set startup file
set(STARTUP_FILE ${CMAKE_SOURCE_DIR}/StartUp/startup_stm32l151xb.s )
#set(STARTUP_FILE "${CMAKE_SOURCE_DIR}/../STM32F4 Standard Peripheral Library/Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40xx.s")


# Adding compiler/linker options
include(compiler.cmake)

add_definitions(
        -DSTM32L151CBU6
        #-DSTM32F40XX
        -DUSE_STDPERIPH_DRIVER)

add_compile_definitions(STM32L1XX_MD)
#add_compile_definitions(STM32F4XX_MD)


set(FREERTOS_DIR FreeRTOS/Source/)
#file(GLOB_RECURSE USER_SOURCES "${CMAKE_SOURCE_DIR}/Users/srcStm32F4/*.c")
file(GLOB_RECURSE E_PAPER_FONTS "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Fonts/*.c")
file(GLOB_RECURSE E_PAPER_GUI "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/GUI/*.c")
file(GLOB_RECURSE E_PAPER "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/e-Paper/*.c")
file(GLOB_RECURSE E_PAPER_CONFIG "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Config/*.c")
file(GLOB_RECURSE E_PAPER_TEST "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Examples/EPD_2in13_test.c")
file(GLOB_RECURSE E_PAPER_GUI "${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/GUI/*.c")

file(GLOB_RECURSE USER_SOURCES "${CMAKE_SOURCE_DIR}/Users/src/*.c")
file(GLOB_RECURSE ONE_WIRE_SOURCES "${CMAKE_SOURCE_DIR}/OneWire/OneWire.c")
file(GLOB_RECURSE DS18B20_SOURCES "${CMAKE_SOURCE_DIR}/DS18B20/ds18b20.c")
file(GLOB_RECURSE AM2305_SOURCES "${CMAKE_SOURCE_DIR}/AM2305/am2305_drv.c")
file(GLOB_RECURSE CheckSum_SOURCES "${CMAKE_SOURCE_DIR}/CheckSum/checksum.c")
file(GLOB_RECURSE PRINTF_SOURCES "${CMAKE_SOURCE_DIR}/printf-master/printf.c")
file(GLOB_RECURSE USART_DRIVER_SOURCES "${CMAKE_SOURCE_DIR}/USART_Driver/usart_drv.c")
file(GLOB_RECURSE W25Q_FLASH_DRIVER_SOURCES "${CMAKE_SOURCE_DIR}/W25Q_FLASH/W25Q_drv.c")
file(GLOB_RECURSE BME280 "${CMAKE_SOURCE_DIR}/BME280/bme280.c")


file(GLOB_RECURSE HAL_SOURCES "${CMAKE_SOURCE_DIR}/../STM32L1xx_StdPeriph_Lib_V1.4.0/Libraries/STM32L1xx_StdPeriph_Driver/src/*.c")
#file(GLOB_RECURSE HAL_SOURCES "${CMAKE_SOURCE_DIR}/../STM32F4 Standard Peripheral Library/Libraries/STM32F4xx_StdPeriph_Driver/src/*.c")

file(
        GLOB_RECURSE
        FREERTOS_SOURCES
        "${FREERTOS_DIR}/croutine.c"
        "${FREERTOS_DIR}/event_groups.c"
        "${FREERTOS_DIR}/list.c"
        "${FREERTOS_DIR}/queue.c"
        "${FREERTOS_DIR}/stream_buffer.c"
        "${FREERTOS_DIR}/tasks.c"
        "${FREERTOS_DIR}/timers.c"
        "${FREERTOS_DIR}/portable/GCC/ARM_CM3/*.c"
        "${FREERTOS_DIR}/portable/MemMang/heap_1.c"
)


include_directories(${CMAKE_SOURCE_DIR}/Users/inc)
include_directories(${CMAKE_SOURCE_DIR}/OneWire)
include_directories(${CMAKE_SOURCE_DIR}/DS18B20)
include_directories(${CMAKE_SOURCE_DIR}/AM2305)
include_directories(${CMAKE_SOURCE_DIR}/W25Q_FLASH)
include_directories(${CMAKE_SOURCE_DIR}/USART_Driver)
include_directories(${CMAKE_SOURCE_DIR}/printf-master)
include_directories(${CMAKE_SOURCE_DIR}/CheckSum)
include_directories(${CMAKE_SOURCE_DIR}/BME280)

include_directories("${CMAKE_SOURCE_DIR}/../STM32L1xx_StdPeriph_Lib_V1.4.0/Libraries/STM32L1xx_StdPeriph_Driver/inc")
include_directories("${CMAKE_SOURCE_DIR}/../STM32L1xx_StdPeriph_Lib_V1.4.0/Libraries/CMSIS/Include")
include_directories("${CMAKE_SOURCE_DIR}/../STM32L1xx_StdPeriph_Lib_V1.4.0/Libraries/CMSIS/Device/ST/STM32L1xx/Include")

include_directories(${FREERTOS_DIR})
include_directories(${FREERTOS_DIR}/include)
include_directories(${FREERTOS_DIR}/portable/GCC/ARM_CM3/)
#include_directories(${FREERTOS_DIR}/portable/IAR/ARM_CM3/)
include_directories(${FREERTOS_DIR}/portable/MemMang)
#include_directories(${FREERTOS_DIR}/portable/Common)

#[[
include_directories(${CMAKE_SOURCE_DIR}/Users/incStm32F4)
include_directories("${CMAKE_SOURCE_DIR}/../STM32F4 Standard Peripheral Library/Libraries/STM32F4xx_StdPeriph_Driver/inc")
include_directories("${CMAKE_SOURCE_DIR}/../STM32F4 Standard Peripheral Library/Libraries/CMSIS/Include")
include_directories("${CMAKE_SOURCE_DIR}/../STM32F4 Standard Peripheral Library/Libraries/CMSIS/Device/ST/STM32F4xx/Include")
]]

include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/e-Paper")
include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Fonts")
include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/GUI")
include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Config")
include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/Examples")
include_directories("${CMAKE_SOURCE_DIR}/e-Paper-master/STM32/STM32-F103ZET6/User/GUI")


add_executable( ${PROJECT_NAME}
        ${STARTUP_FILE}
        ${ONE_WIRE_SOURCES}
        ${USER_SOURCES}
        ${DS18B20_SOURCES}
        ${AM2305_SOURCES}
        ${BME280}
        ${USART_DRIVER_SOURCES}
        ${HAL_SOURCES}
        ${E_PAPER_FONTS}
        ${E_PAPER_CONFIG}
        ${E_PAPER}
        ${E_PAPER_TEST}
        ${E_PAPER_GUI}

        ${PRINTF_SOURCES}
        ${FREERTOS_SOURCES}
        ${W25Q_FLASH_DRIVER_SOURCES}
        ${CheckSum_SOURCES}
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
#[[
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY}
        ARGS -Obinary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
        COMMENT "Generate ${PROJECT_NAME}.bin file"
        )
]]

# Show memory usage
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${GCC_SIZE}
        ARGS --format=berkeley -t ${PROJECT_NAME}.elf
        COMMENT "Memory usage:"
        )