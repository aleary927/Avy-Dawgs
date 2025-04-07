cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM and C languages
enable_language(C ASM)

# variables for driver dirs
set(HAL_DRIVER_DIR ${CMAKE_SOURCE_DIR}/../STM32CubeF7/Drivers/STM32F7xx_HAL_Driver)
set(CMSIS_DRIVER_DIR ${CMAKE_SOURCE_DIR}/../STM32CubeF7/Drivers/CMSIS)
set(BSP_DIR ${CMAKE_SOURCE_DIR}/../STM32CubeF7/Drivers/BSP/STM32F7xx_Nucleo_144)

# STM32CubeMX generated symbols (macros)
set(Defines_Syms 
	USE_HAL_DRIVER 
  USE_FULL_LL_DRIVER
	STM32F722xx
  ARM_MATH_CM7
  __FPU_PRESENT=1U
    $<$<CONFIG:Debug>:DEBUG>
)

# only include these files from this directory by copying to current binary dir
configure_file(${CMSIS_DRIVER_DIR}/Device/ST/STM32F7xx/Include/stm32f7xx.h stm32f7xx.h COPYONLY)
configure_file(${CMSIS_DRIVER_DIR}/Device/ST/STM32F7xx/Include/stm32f722xx.h stm32f722xx.h COPYONLY)
configure_file(${CMSIS_DRIVER_DIR}/Device/ST/STM32F7xx/Include/system_stm32f7xx.h system_stm32f7xx.h COPYONLY)

# Include paths
set(Include_Dirs
    ${CMAKE_SOURCE_DIR}/Inc
    ${HAL_DRIVER_DIR}/Inc
    ${HAL_DRIVER_DIR}/Inc/Lecacy
    ${CMSIS_DRIVER_DIR}/Include
    ${CMSIS_DRIVER_DIR}/DSP/Include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${BSP_DIR}
)

# STM32CubeMX generated application sources

# STM32 HAL/LL Drivers
set(Drivers_Src
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_cortex.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_gpio.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_uart.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_uart_ex.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_exti.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_rcc.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_rcc_ex.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_pcd.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_pcd_ex.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_pwr.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_pwr_ex.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_flash.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_flash_ex.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_i2c.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_hal_i2c_ex.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_ll_dma.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_ll_adc.c
    ${HAL_DRIVER_DIR}/Src/stm32f7xx_ll_usb.c
    ${CMAKE_SOURCE_DIR}/Src/system_stm32f7xx.c
    ${CMSIS_DRIVER_DIR}/DSP/Source/TransformFunctions/arm_rfft_init_q15.c
    ${CMSIS_DRIVER_DIR}/DSP/Source/TransformFunctions/arm_rfft_q15.c
    ${CMSIS_DRIVER_DIR}/DSP/Source/TransformFunctions/arm_cfft_q15.c
    ${CMSIS_DRIVER_DIR}/DSP/Source/TransformFunctions/arm_cfft_radix4_q15.c
    ${CMSIS_DRIVER_DIR}/DSP/Source/TransformFunctions/arm_bitreversal.c
    ${CMSIS_DRIVER_DIR}/DSP/Source/TransformFunctions/arm_bitreversal2.S
    ${CMSIS_DRIVER_DIR}/DSP/Source/CommonTables/arm_const_structs.c
    ${CMSIS_DRIVER_DIR}/DSP/Source/CommonTables/arm_common_tables.c
    ${CMSIS_DRIVER_DIR}/DSP/Source/FastMathFunctions/arm_sin_f32.c
    ${CMSIS_DRIVER_DIR}/DSP/Source/FastMathFunctions/arm_cos_f32.c
)

# Drivers Midllewares



# Link directories setup
set(LINK_DIRS

)
# Project static libraries
set(LINK_LIBS 
    STM32_Drivers
    
)
# Interface library for includes and symbols
add_library(stm32cube INTERFACE)
target_include_directories(stm32cube INTERFACE ${Include_Dirs})
target_compile_definitions(stm32cube INTERFACE ${Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cube)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${LINK_DIRS})

# Add libraries to the project
target_link_libraries(${CMAKE_PROJECT_NAME} ${LINK_LIBS})

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
