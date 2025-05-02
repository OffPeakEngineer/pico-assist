cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(ctxdemo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()



add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(demo
        demo.c
        ctx.h
        Adafruit_ILI9341.h
        Adafruit_ILI9341.cpp
        ctx-pico-ili9341.c
        # ctx-pico-st7789.c
        )
target_include_directories(demo PUBLIC
        ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/fonts)
target_link_libraries(demo
                      pico_stdlib
                      pico_multicore
                      hardware_pio)

pico_enable_stdio_usb(demo 1)
pico_enable_stdio_uart(demo 0)
pico_add_extra_outputs(demo)

add_executable(stdio
        ctx-stdio.c
        ctx.h
        Adafruit_ILI9341.h
        Adafruit_ILI9341.cpp
        ctx-pico-ili9341.c
        # ctx-pico-st7789.c
        )
target_include_directories(stdio PUBLIC
        ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/fonts)
target_link_libraries(stdio
                      pico_stdlib
                      pico_multicore
                      hardware_pio)

pico_enable_stdio_usb(stdio 1)
pico_enable_stdio_uart(stdio 0)

pico_add_extra_outputs(stdio)
