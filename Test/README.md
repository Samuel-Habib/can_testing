# Host Unit Testing with Fake Function Framework (FFF) & GCC

This directory contains the host-based unit testing suite for the project, allowing you to compile and test embedded C application modules on your host machine using **GCC**, **Fake Function Framework (FFF)**, and **Unity**.

---

## Directory Structure

```
Test/
├── CMakeLists.txt        # CMake build configuration for host GCC
├── README.md             # This guide
├── framework/            # Test and mocking frameworks
│   ├── fff.h             # Fake Function Framework (single-header)
│   ├── unity.h           # Unity test framework
│   ├── unity.c
│   └── unity_internals.h
├── mocks/                # Stubs and FFF fakes for STM32 HAL & FreeRTOS
│   ├── fake_stm32_hal.h  # Declarations of all FFF fakes for HAL & FreeRTOS
│   ├── fake_stm32_hal.c  # Implementations of fakes, registers, and globals
│   ├── stm32h7xx_hal*.h  # Mock HAL headers & peripheral definitions
│   ├── FreeRTOS.h        # Mock FreeRTOS types and definitions
│   ├── cmsis_os2.h       # Mock CMSIS RTOS v2 types and definitions
│   ├── queue.h           # Mock FreeRTOS queue definitions
│   └── task.h            # Mock FreeRTOS task definitions
└── src/                  # Test source files
    └── test_example.c    # Example test demonstrating FFF usage
```

---

## Quick Start: Compiling Host Tests

### 1. Configure and Build with CMake & Ninja:
```bash
cmake -B build/test -S Test -G Ninja
cmake --build build/test
```

### 2. Run Tests with CTest or Directly:
```bash
ctest --test-dir build/test --output-on-failure
# Or run individual test binaries:
./build/test/test_example
```

---

## How to Write Unit Tests with FFF

### 1. Structure of a Test File
```c
#include "unity.h"
#include "fff.h"
#include "fake_stm32_hal.h"

// Include the application headers to test:
#include "can.h"
#include "logging.h"

void setUp(void) {
    // Reset all global HAL and FreeRTOS fakes before every test
    fake_stm32_hal_reset_all();
}

void tearDown(void) {
}
```

### 2. Controlling Fakes and Return Values
```c
void test_example(void) {
    // 1. Configure fake return values
    HAL_FDCAN_AddMessageToTxFifoQ_fake.return_val = HAL_OK;

    // 2. Call the code under test
    uint8_t data[8] = { 0x01, 0x02 };
    int status = can_tx(&hfdcan1, data);

    // 3. Verify return values & fake call statistics
    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_INT(1, HAL_FDCAN_AddMessageToTxFifoQ_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(data, HAL_FDCAN_AddMessageToTxFifoQ_fake.arg2_val);
}
```

### 3. FFF Cheat Sheet
- **Call count verification:** `my_func_fake.call_count`
- **Argument inspection:** `my_func_fake.arg0_val`, `my_func_fake.arg1_val`, ...
- **Return value setting:** `my_func_fake.return_val = <value>;`
- **Return sequence:** `SET_RETURN_SEQ(my_func, array_of_returns, array_len);`
- **Custom fake callback:** `my_func_fake.custom_fake = my_custom_handler;`
- **Defining a new fake in test:**
  - `FAKE_VOID_FUNC(func_name, arg1_type, arg2_type);`
  - `FAKE_VALUE_FUNC(return_type, func_name, arg1_type, arg2_type);`
- **Resetting a fake:** `RESET_FAKE(func_name);`

---

## Adding New Tests to CMake

In `Test/CMakeLists.txt`, use the `add_unit_test` helper:

```cmake
add_unit_test(test_my_module
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_my_module.c
    ${APP_SRC_DIR}/my_module.c
)
```
