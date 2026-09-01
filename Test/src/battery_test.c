#include "battery.h"
#include "fake_stm32_hal.h"
#include "fff.h"
#include "unity.h"
#define ADC_CURRENT_SAMPLE_COUNT 100
// FAKE_VOID_FUNC(run_battery_task, volatile unsigned int *);

void setUp() { fake_stm32_hal_reset_all(); }
void tearDown() { fake_stm32_hal_reset_all(); }

void test_overcurrent_protection(void) {
  volatile unsigned int sensor_readings[100];
  for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
    sensor_readings[i] = 200;
  }
  bool result = run_battery_task(sensor_readings);
  TEST_ASSERT_EQUAL_INT(result, 1);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_overcurrent_protection);
  return UNITY_END();
}
