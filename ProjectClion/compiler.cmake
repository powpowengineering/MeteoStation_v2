#=== Compiler and Linker options file ===#

#=== Set compiler options ===#
# Common options
add_compile_options(-std=c${CMAKE_C_STANDARD} -mlong-calls -ffreestanding)
add_link_options(--specs=nosys.specs)
# Assembler options
#add_compile_options(-Wa,-mcpu=${CMAKE_SYSTEM_PROCESSOR} -Wa,-g3 -Wa,-c -Wa,-mfloat-abi=softfp -Wa,-mthumb)
# Debugging options
add_compile_options(-g3 -gdwarf)
# Developer options
add_compile_options(-fno-strict-aliasing -fstack-usage)
# Code optimization
add_compile_options(-O0 -ffunction-sections -fdata-sections)
# Warnings options
add_compile_options(-Wall -Wno-pragmas)
# Stop build on warnings
#add_compile_options(-Werror)
# Machine-dependent options
add_compile_options(-mcpu=${CMAKE_SYSTEM_PROCESSOR} -mlittle-endian -mthumb -mfloat-abi=softfp -Wl,-u,vfprintf -Wl,-u,_printf_float -Wl,-u,_scanf_float )#-mfpu=fpv4-sp-d16 )



#=== Set linker options ===#
# Common options
#add_link_options(-static -nostdlib -T ${LINKER_SCRIPT})
add_link_options(-static  -T ${LINKER_SCRIPT})

# Linker optimizations
add_link_options(-Wl,--gc-sections)

# Map-file options
add_link_options(-Wl,-Map=${PROJECT_NAME}.map -Wl,--cref)
# Machine-dependent options
add_link_options(-mcpu=${CMAKE_SYSTEM_PROCESSOR})

# Linker launch pattern
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <LINK_FLAGS> -o <TARGET> <OBJECTS>")
