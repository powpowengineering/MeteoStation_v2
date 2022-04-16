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
# Binary file extension
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
# Linker script file
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/Config/STM32L151XB_FLASH.ld)
# Set startup file
set(STARTUP_FILE ${CMAKE_SOURCE_DIR}/StartUp/startup_stm32l151xb.s OneWire/OneWire.c OneWire/OneWire.h OneWire/OneWire_cfg.h)

# Adding compiler/linker options
include(compiler.cmake)

add_definitions(
        -DSTM32L151CBU6
        -DUSE_STDPERIPH_DRIVER)

add_compile_definitions(STM32L1XX_MD)

set(FREERTOS_DIR FreeRTOS/Source/)

file(GLOB_RECURSE USER_SOURCES "${CMAKE_SOURCE_DIR}/Users/src/*.c")
file(GLOB_RECURSE ONE_WIRE_SOURCES "${CMAKE_SOURCE_DIR}/OneWire/OneWire.c")
file(GLOB_RECURSE HAL_SOURCES "${CMAKE_SOURCE_DIR}/../STM32L1xx_StdPeriph_Lib_V1.4.0/Libraries/STM32L1xx_StdPeriph_Driver/src/*.c")
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
include_directories("${CMAKE_SOURCE_DIR}/../STM32L1xx_StdPeriph_Lib_V1.4.0/Libraries/STM32L1xx_StdPeriph_Driver/inc")
include_directories("${CMAKE_SOURCE_DIR}/../STM32L1xx_StdPeriph_Lib_V1.4.0/Libraries/CMSIS/Include")
include_directories("${CMAKE_SOURCE_DIR}/../STM32L1xx_StdPeriph_Lib_V1.4.0/Libraries/CMSIS/Device/ST/STM32L1xx/Include")

include_directories(${FREERTOS_DIR})
include_directories(${FREERTOS_DIR}/include)
include_directories(${FREERTOS_DIR}/portable/GCC/ARM_CM3/)
#include_directories(${FREERTOS_DIR}/portable/IAR/ARM_CM3/)
include_directories(${FREERTOS_DIR}/portable/MemMang)
#include_directories(${FREERTOS_DIR}/portable/Common)


add_executable(${PROJECT_NAME} ${STARTUP_FILE} ${USER_ONE_WIRE} ${USER_SOURCES} ${HAL_SOURCES} ${FREERTOS_SOURCES})




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