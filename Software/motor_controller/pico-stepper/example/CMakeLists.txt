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

# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Add pico-stepper directory
add_subdirectory(../lib stepper)

project(stepper-basic-example C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable.
add_executable(stepper-basic-example stepper-basic-example.c)

# This stdio configuration alows to see the Pico output when connected to a
# picoprobe. Enable usb and disable uart for standalone operation (direct USB
# connection).
pico_enable_stdio_uart(stepper-basic-example 0)
pico_enable_stdio_usb(stepper-basic-example 1)

# Standard and additional libraries
target_link_libraries(
	stepper-basic-example
	pico_stdlib
    pico-stepper)

pico_add_extra_outputs(stepper-basic-example)