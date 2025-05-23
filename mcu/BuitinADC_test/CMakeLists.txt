cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME NUCLEO-STM32F722ZE_Template)

# application source code
set(Application_Src
    ${CMAKE_SOURCE_DIR}/Src/main.c
    ${CMAKE_SOURCE_DIR}/Src/stm32f7xx_it.c
    ${CMAKE_SOURCE_DIR}/Src/stm32f7xx_hal_msp.c
    ${CMAKE_SOURCE_DIR}/Src/sysmem.c
    ${CMAKE_SOURCE_DIR}/Src/syscalls.c
    ${CMAKE_SOURCE_DIR}/startup_stm32f722xx.s
    ${CMAKE_SOURCE_DIR}/Src/app_main.c
    ${CMAKE_SOURCE_DIR}/Src/adc.c
    ${CMAKE_SOURCE_DIR}/Src/dsp.c
    ${CMAKE_SOURCE_DIR}/Src/UART.c
    ${CMAKE_SOURCE_DIR}/Src/guidance.c   
)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cube)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${Application_Src})

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    ARM_MATH_CM7
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cube

    # Add user defined libraries
)

