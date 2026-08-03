cmake_minimum_required(VERSION 3.20)

project(VirtualMemoryManager
    VERSION 0.1.0
    DESCRIPTION "Virtual Memory Manager Simulator in C17"
    LANGUAGES C
)

# Force C17 Standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Export compile commands for clangd / language servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler warnings and options
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wdouble-promotion
        -Wformat=2
        -Wno-unused-parameter
    )
endif()

# Build Type Default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# Include Directories
include_directories(include)

# Subdirectories / Modules
add_subdirectory(src)

# Enable testing framework if needed in the future
enable_testing()
add_subdirectory(tests)