cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING 1)

# toolchain
set(CMAKE_CXX_COMPILER "$ENV{GCC_TOOLCHAIN_DIR}/bin/arm-none-eabi-g++")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AS_COMPILER "$ENV{GCC_TOOLCHAIN_DIR}/bin/arm-none-eabi-g++")
set(CMAKE_OBJCOPY "$ENV{GCC_TOOLCHAIN_DIR}/bin/arm-none-eabi-objcopy")
set(CMAKE_SIZE_UTIL "$ENV{GCC_TOOLCHAIN_DIR}/bin/arm-none-eabi-size")

# compile/asm/link flags
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/stm32g431kb.ld")
set(CMAKE_ASM_FLAGS "-c -O0 -mcpu=cortex-m4 -mthumb -Wall -fmessage-length=0")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m4 -mthumb -Wall -g3 -fmessage-length=0 --specs=nosys.specs -fno-exceptions -fno-unwind-tables -nostdlib -fno-rtti")
set(CMAKE_EXE_LINKER_FLAGS "-lgcc -T${LINKER_SCRIPT}")

# outputs
set(EXE_NAME "woof")
set(ELF_FILE ${CMAKE_BINARY_DIR}/${EXE_NAME}.elf)
set(BIN_FILE ${CMAKE_BINARY_DIR}/${EXE_NAME}.bin)

project(woof LANGUAGES CXX ASM)
file( GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.S"
)
add_executable(woof ${SOURCES})
set_target_properties(woof PROPERTIES OUTPUT_NAME "${EXE_NAME}.elf")

target_include_directories(woof PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

add_custom_command(TARGET woof POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -S -O binary ${ELF_FILE} ${BIN_FILE}
    COMMAND ${CMAKE_SIZE_UTIL} ${ELF_FILE}
    COMMENT "Building ${BIN_FILE}"
)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${BIN_FILE}")

add_custom_target(flash 
    DEPENDS woof
    COMMAND st-flash --reset write ${BIN_FILE} 0x08000000
)
