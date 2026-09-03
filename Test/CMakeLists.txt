cmake_minimum_required(VERSION 3.20)

# Ensure GCC uses the system linker in /usr/bin if an older incompatible ld exists in /usr/local/bin
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -B/usr/bin/")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -B/usr/bin/")

project(HostUnitTests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags for host GCC
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -g
        -O0
        -fdiagnostics-color=always
    )
endif()

# Enable CTest integration
enable_testing()

# Include directories
set(TEST_FRAMEWORK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/framework)
set(TEST_MOCKS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mocks)
set(APP_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../App/Inc)
set(APP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../App/Src)
set(CORE_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Core/Inc)

# Common test framework library (Unity + FFF + Fake HAL / RTOS)
add_library(test_framework STATIC
    ${TEST_FRAMEWORK_DIR}/unity.c
    ${TEST_MOCKS_DIR}/fake_stm32_hal.c
)

target_include_directories(test_framework PUBLIC
    ${TEST_MOCKS_DIR}
    ${TEST_FRAMEWORK_DIR}
    ${APP_INC_DIR}
    ${CORE_INC_DIR}
)

# Helper macro to define a unit test executable
function(add_unit_test TEST_NAME TEST_SRC)
    add_executable(${TEST_NAME}
        ${TEST_SRC}
        ${ARGN}
    )
    target_link_libraries(${TEST_NAME} PRIVATE test_framework m)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# ---------------------------------------------------------------------------
# Test Targets
# ---------------------------------------------------------------------------
# Example test combining FFF + Unity with App sources
#add_unit_test(test_example
    #  ${CMAKE_CURRENT_SOURCE_DIR}/src/test_example.c
    #    ${APP_SRC_DIR}/can.c
    # ${APP_SRC_DIR}/logging.c
    #${APP_SRC_DIR}/watchdog.c
    # )

# Battery test target
add_unit_test(battery_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/battery_test.c
    ${APP_SRC_DIR}/battery.c
)


