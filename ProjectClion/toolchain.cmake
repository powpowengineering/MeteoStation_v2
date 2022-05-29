#=== Toolchain settings file ===#

# Set cross-compilation variables
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

# Set paths to GCC
#set(GCC_ROOT_PATH "C:/soft/gcc-arm-none-eabi-10.3-2021.10-win32/gcc-arm-none-eabi-10.3-2021.10/bin")
set(GCC_ROOT_PATH "/Applications/ARM/bin")
set(GCC_PREFIX "arm-none-eabi")

set(CMAKE_C_COMPILER   ${GCC_ROOT_PATH}/${GCC_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${GCC_ROOT_PATH}/${GCC_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${GCC_ROOT_PATH}/${GCC_PREFIX}-gcc)
set(CMAKE_AR           ${GCC_ROOT_PATH}/${GCC_PREFIX}-ar)
set(CMAKE_OBJCOPY      ${GCC_ROOT_PATH}/${GCC_PREFIX}-objcopy)
set(CMAKE_OBJDUMP      ${GCC_ROOT_PATH}/${GCC_PREFIX}-objdump)
set(GCC_SIZE           ${GCC_ROOT_PATH}/${GCC_PREFIX}-size)

# Cross-compilation - do not try check compiler
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
