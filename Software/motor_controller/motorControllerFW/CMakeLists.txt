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
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(motorControllerFW C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add pico-stepper directory
add_subdirectory(../pico-stepper/lib stepper)

# Add executable. Default name is the project name, version 0.1
add_executable(motorControllerFW 
    # The main program file
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
    common/base_classes/CurrentSense.cpp
    common/base_classes/FOCMotor.cpp

    # Current Sensor Drivers
    current_sense/InlineCurrentSense.cpp
    current_sense/rp2040_mcu.cpp

    # PWM Drivers
    drivers/StepperDriver4PWM.cpp
    drivers/rp2040_mcu.cpp

    # Sensor Drivers
    sensors/MT6701_I2C.cpp

    # FOC Controller
    StepperMotor.cpp

    # FOC Serial Command Interface
    communication/Commander.cpp
    )

pico_set_program_name(motorControllerFW "motorControllerFW")
pico_set_program_version(motorControllerFW "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(motorControllerFW 0)
pico_enable_stdio_usb(motorControllerFW 1)

# Add libraries to the build
target_link_libraries(motorControllerFW
    pico_stdlib
    pico_multicore
    hardware_i2c
    hardware_adc
    hardware_irq
    hardware_pwm
    hardware_dma
    cmsis_core
    )

# Add the standard include files to the build
target_include_directories(motorControllerFW PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(motorControllerFW)

