# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_PLATFORM rp2350)

set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Set the build type.")
endif()

if(NOT DEFINED PICO_PLATFORM)
    set(PICO_PLATFORM rp2350 CACHE STRING "Set the platform.")
endif()
# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# We also need PICO EXTRAS
include(pico_extras_import.cmake)

project(eldemo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(eldemo)

pico_generate_pio_header(eldemo ${CMAKE_CURRENT_LIST_DIR}/eldata.pio)

target_sources(eldemo PRIVATE
        #main.c
        main_3d_demo.c
        el.c
        )

pico_set_program_name(eldemo "eldemo")
pico_set_program_version(eldemo "0.1")

pico_enable_stdio_uart(eldemo 0)
pico_enable_stdio_usb(eldemo 0)

target_include_directories(eldemo PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Add the standard library to the build
target_link_libraries(eldemo pico_stdlib hardware_dma m)

# Add any user requested libraries
target_link_libraries(eldemo
        hardware_pio
        pico_multicore
        )

pico_add_extra_outputs(eldemo)

# Grayscale test executable
add_executable(grayscale_test)

pico_generate_pio_header(grayscale_test ${CMAKE_CURRENT_LIST_DIR}/eldata.pio)

target_sources(grayscale_test PRIVATE
        grayscale_test.c
        el.c
        )

pico_set_program_name(grayscale_test "grayscale_test")
pico_set_program_version(grayscale_test "0.1")

pico_enable_stdio_uart(grayscale_test 1)
pico_enable_stdio_usb(grayscale_test 1)

target_include_directories(grayscale_test PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(grayscale_test pico_stdlib hardware_dma m)

target_link_libraries(grayscale_test
        hardware_pio
        )

pico_add_extra_outputs(grayscale_test)

# Simple grayscale animation demo
add_executable(simple_gray_demo)

pico_generate_pio_header(simple_gray_demo ${CMAKE_CURRENT_LIST_DIR}/eldata.pio)

target_sources(simple_gray_demo PRIVATE
        simple_gray_demo.c
        el.c
        )

pico_set_program_name(simple_gray_demo "simple_gray_demo")
pico_set_program_version(simple_gray_demo "0.1")

pico_enable_stdio_uart(simple_gray_demo 1)
pico_enable_stdio_usb(simple_gray_demo 1)

target_include_directories(simple_gray_demo PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(simple_gray_demo pico_stdlib hardware_dma m)

target_link_libraries(simple_gray_demo
        hardware_pio
        )

pico_add_extra_outputs(simple_gray_demo)

