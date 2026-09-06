#include "battery.h"
#include "fake_stm32_hal.h"
#include "fff.h"
#include "main.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdio.h>
#define ADC_CURRENT_SAMPLE_COUNT (100)
// FAKE_VOID_FUNC(run_battery_task, volatile unsigned int *);

void setUp() {
  fake_stm32_hal_reset_all();
  reset_battery_current_test();
}
void tearDown() {
  fake_stm32_hal_reset_all();
  reset_battery_current_test();
}
int get_secure_random(uint32_t *buffer, size_t count) {
  FILE *fp = fopen("/dev/urandom", "r");
  if (!fp)
    return -1;

  size_t read_bytes = fread(buffer, sizeof(uint32_t), count, fp);
  fclose(fp);

  return (read_bytes == count) ? 0 : -1;
}

uint32_t buffer[1];

void intenseSpikes(void) {
  bool result;
  int j = 0;
  int total = 0;

  volatile unsigned int sensor_readings[100];
  sensor_readings[0] = 75;

  for (int i = 1; i < 4; ++i) {
    sensor_readings[i] = 75;
  }

  for (int i = 4; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
    sensor_readings[i] = 75;
  }
  printf("sensor_readings: %d \r \n", sensor_readings[1]);
  printf("battery_task_awake %d \r \n", battery_task_awake);
  result = run_battery_task(sensor_readings);
  TEST_ASSERT_EQUAL_INT(0, result);
  while (j < 19) {
    for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
      get_secure_random(buffer, 1);
      sensor_readings[i] = 79.9999; //+ (buffer[0] % 3);
      printf("rand number: %d \r \n", buffer[0] % 3);
      total += (sensor_readings[i] - 75) * (sensor_readings[i] - 75);
    }
    result = run_battery_task(sensor_readings);

    j += 1;
    if (result) {
      printf("disconnect \r \n");
      break;
    }
  }

  uint32_t max_sustained = (80 - 75) * (80 - 75) * 100000 * 2;
  printf("max:   %d \r \n", max_sustained);
  printf("total: %d \r \n", total);
  //  TEST_ASSERT_EQUAL_INT(0, result);
}

void noisy_test(void) {
  //  TEST_ASSERT_EQUAL_INT(1, battery_task_awake);
  volatile unsigned int sensor_readings[100];
  bool result;
  int j = 0;
  int total = 0;
  while (j < 20) {
    for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
      get_secure_random(buffer, 1);
      sensor_readings[i] = 78; //+ (buffer[0] % 3);
      //      printf("rand number: %d \r \n", buffer[0] % 3);
      total += (sensor_readings[i] - 75) * (sensor_readings[i] - 75);
    }
    result = run_battery_task(sensor_readings);

    j += 1;
    if (result) {
      printf("disconnect \r \n");
      break;
    }
  }

  uint32_t max_sustained = (80 - 75) * (80 - 75) * 2000;
  printf("max:   %d \r \n", max_sustained);
  printf("total: %d \r \n", total);
  TEST_ASSERT_EQUAL_INT(0, result);
}

void uniform_overcurrent_test(void) {
  //  TEST_ASSERT_EQUAL_INT(1, battery_task_awake);
  volatile unsigned int sensor_readings[100];
  bool result;
  int j = 0;
  int total = 0;
  while (j < 20) {
    for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
      sensor_readings[i] = 81;
    }
    result = run_battery_task(sensor_readings);
    j += 1;
    if (result) {
      printf("disconnect \r \n");
      break;
    }
  }

  uint32_t max_sustained = (80 - 75) * (80 - 75) * 2000;
  TEST_ASSERT_EQUAL_INT(1, result);
}

void test_overcurrent_protection(void) {
  volatile unsigned int sensor_readings[100];
  bool result = false;
  int shutoff_current = 80;
  while (!result) {
    for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
      sensor_readings[i] = shutoff_current;
    }
    result = run_battery_task(sensor_readings);
    if (result) {
      printf("shutoff_current: %d \r\n", shutoff_current);
      break;
    }

    shutoff_current++;
  }
  TEST_ASSERT_EQUAL_INT(1, result);
  //  TEST_ASSERT_EQUAL_INT(1, battery_task_awake);
}

void test_normal_current(void) {
  volatile unsigned int sensor_readings[100];
  bool result = false;
  for (int j = 0; j < 1; j++) {

    //   battery_test_debug();
    for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
      sensor_readings[i] = 79;
    }

    result = run_battery_task(sensor_readings);

    if (result) {
      printf("disconnect \r\n");
      break;
    }
    //    battery_test_debug();
  }
  TEST_ASSERT_EQUAL_INT(0, result);
  //  TEST_ASSERT_EQUAL_INT(0, battery_task_awake);
}

void test_low_current(void) {
  volatile unsigned int sensor_readings[100];
  bool result = false;
  for (int j = 0; j < 1; j++) {

    //   battery_test_debug();
    for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
      sensor_readings[i] = 63;
    }

    result = run_battery_task(sensor_readings);

    if (result) {
      printf("disconnect \r\n");
      break;
    }
    //    battery_test_debug();
  }
  TEST_ASSERT_EQUAL_INT(0, result);
  //  TEST_ASSERT_EQUAL_INT(0, battery_task_awake);
}

void test_undercurrent_point(void) {

  volatile unsigned int sensor_readings[100];
  bool result = false;
  battery_test_debug();
  int underflow_current = 75;

  while (!result) {
    for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
      sensor_readings[i] = underflow_current;
    }

    result = run_battery_task(sensor_readings);

    if (result) {
      printf("disconnected at: %d \r \n", underflow_current);
      break;
    }
    underflow_current--;
  }
  battery_test_debug();
  TEST_ASSERT_EQUAL_INT(1, result);
  //  TEST_ASSERT_EQUAL_INT(0, battery_task_awake);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(noisy_test);
  RUN_TEST(intenseSpikes);
  RUN_TEST(test_overcurrent_protection);
  RUN_TEST(test_normal_current);
  RUN_TEST(uniform_overcurrent_test);
  RUN_TEST(test_undercurrent_point);
  RUN_TEST(test_low_current);
  return UNITY_END();
}
