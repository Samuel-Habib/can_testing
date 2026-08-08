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

# Enhanced diagnostic compiler flags for helpful error messages and tracing redefinitions
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -fdiagnostics-color=always        # Preserve colors in compiler messages
        -fmacro-backtrace-limit=0         # Trace through macro expansions to pinpoint initial definitions
        -fdiagnostics-show-option        # Show compiler flags associated with errors/warnings
    )
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(
            -fdiagnostics-show-template-tree
        )
    endif()
elseif (MSVC)
    add_compile_options(/diagnostics:caret)
endif()

# Enable CMake message context scopes in trace outputs
set(CMAKE_MESSAGE_CONTEXT_SHOW ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME ru-tel)

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
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
    ./Core/Inc/
)

# Automatically discover and add user source files
file(GLOB_RECURSE USER_SOURCES CONFIGURE_DEPENDS "App/Src/*.c")
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${USER_SOURCES})

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    App/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# OpenOCD Flashing configuration and targets
set(OPENOCD "openocd" CACHE STRING "Path to OpenOCD executable")
set(OPENOCD_INTERFACE "interface/stlink.cfg" CACHE STRING "OpenOCD interface configuration file")
set(OPENOCD_TARGET "target/stm32h7x.cfg" CACHE STRING "OpenOCD target configuration file")

add_custom_target(flash
    COMMAND ${OPENOCD} -f ${OPENOCD_INTERFACE} -f ${OPENOCD_TARGET} -c "program $<TARGET_FILE:${CMAKE_PROJECT_NAME}> verify reset exit"
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMENT "Flashing ${CMAKE_PROJECT_NAME}.elf to target MCU using OpenOCD..."
    USES_TERMINAL
)
add_custom_target(openocd-flash DEPENDS flash)
