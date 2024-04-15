cmake_minimum_required(VERSION 3.5)
project(emu-chip8 C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# default compiler
if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER clang)
endif()

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(SOURCES
    src/main.c
    src/core/chip.c
)

add_executable(emu-chip8 ${SOURCES})
target_link_libraries(emu-chip8 ${SDL2_LIBRARIES})

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(emu-chip8 PRIVATE -std=c99 -O3 -Wall -Wextra -pedantic -fsanitize=address,undefined)
endif()
