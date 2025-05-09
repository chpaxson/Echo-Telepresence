# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(MotorControllerCustomFOC C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(MotorControllerCustomFOC 
    
    main.cpp 

    # can2040 Lib
    can/can2040.c

    # Simple FOC Port
    # common
    common/pid.cpp
    common/foc_utils.cpp
    common/lowpass_filter.cpp

    # Main classes
    common/base_classes/Sensor.cpp

    # Sensor Drivers
    sensors/MT6701_I2C.cpp
)

pico_set_program_name(MotorControllerCustomFOC "MotorControllerCustomFOC")
pico_set_program_version(MotorControllerCustomFOC "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(MotorControllerCustomFOC 0)
pico_enable_stdio_usb(MotorControllerCustomFOC 1)

# Add the standard library to the build
target_link_libraries(MotorControllerCustomFOC
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(MotorControllerCustomFOC PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(MotorControllerCustomFOC 
        hardware_i2c
        pico_multicore
        hardware_adc
        hardware_irq
        hardware_pwm
        hardware_dma
        cmsis_core
        )

pico_add_extra_outputs(MotorControllerCustomFOC)

