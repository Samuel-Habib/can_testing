#include "battery.h"
#include "fake_stm32_hal.h"
#include "fff.h"
#include "main.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdio.h>
#define ADC_CURRENT_SAMPLE_COUNT 100
// FAKE_VOID_FUNC(run_battery_task, volatile unsigned int *);

void setUp() { fake_stm32_hal_reset_all(); }
void tearDown() { fake_stm32_hal_reset_all(); }

int get_secure_random(uint32_t *buffer, size_t count) {
  FILE *fp = fopen("/dev/urandom", "r");
  if (!fp)
    return -1;

  size_t read_bytes = fread(buffer, sizeof(uint32_t), count, fp);
  fclose(fp);

  return (read_bytes == count) ? 0 : -1;
}

uint32_t buffer[1];

void sustained90_test(void) {
  volatile unsigned int sensor_readings[100];
  //  TEST_ASSERT_EQUAL_INT(1, battery_task_awake);

  bool result;
  int j = 0;
  int total = 0;
  while (j < 200000) {
    for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
      get_secure_random(buffer, 1);
      sensor_readings[i] = 80.00001; //+ (buffer[0] % 3);
      printf("rand number: %d \r \n", buffer[0] % 3);
      total += (sensor_readings[i] - 75) * (sensor_readings[i] - 75);
    }
    result = run_battery_task(sensor_readings);

    j += 100; // <<<<< ==========================
    if (result)
      printf("disconnect \r \n");
  }

  uint32_t max_sustained = (80 - 75) * (80 - 75) * 100000 * 2;
  printf("max:   %d \r \n", max_sustained);
  printf("total: %d \r \n", total);
  TEST_ASSERT_EQUAL_INT(1, result);
}

void test_overcurrent_protection(void) {
  volatile unsigned int sensor_readings[100];
  for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
    sensor_readings[i] = 299;
  }
  bool result = run_battery_task(sensor_readings);
  TEST_ASSERT_EQUAL_INT(1, result);
  TEST_ASSERT_EQUAL_INT(1, battery_task_awake);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(sustained90_test);
  //  RUN_TEST(test_overcurrent_protection);
  return UNITY_END();
}
