#include "battery.h"

const osThreadAttr_t battery_attributes = {
    .name = "battery",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityISR,
};

bool battery_task_awake = false;
static uint32_t time = 0;

void reset_battery_current_test(void) {
  time = 0;
  battery_task_awake = false;
  riemann_sum_total = 0;
}

uint32_t max_time = 1000 * 2;

int32_t max_sustained = (80 - 75) * (80 - 75) * 2000;
/* Σ (75-80)^2 * 2  = 50 A^2 * s // maximum */

void battery_test_debug(void) {
  printf("\n riemann_sum_total: %d \r \n", riemann_sum_total);
  printf("\n max_sustained: %d \r \n", max_sustained);
}

bool run_battery_task(volatile unsigned int *curr_sensor_readings) {

  // t_delta = 1ms or 1000 us
  if (battery_task_awake == false)
    time = 0;
  battery_task_awake = true;
  signed int sign = 1;
  if (time < max_time) {
    // 300-75 = 225
    // 225^2 *100 = 5062500 which is within the bounds of signed 32 bit int
    for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
      if (curr_sensor_readings[i] < 75)
        sign = -1;
      else
        sign = 1;
      riemann_sum_total += square(curr_sensor_readings[i] - 75) * sign;
      if (riemann_sum_total > max_sustained) {
        battery_test_debug();
        return true;
      } else if (riemann_sum_total < 0)
        riemann_sum_total = 0;
    }
    time += ADC_CURRENT_SAMPLE_COUNT;
  } else {
    battery_task_awake = false;
    time = 0;
    riemann_sum_total = 0;
  }

  return false;
}

void Battery_Task(void *argument) {
  for (;;) {

    uint32_t ulNotifiedValue;
    xTaskNotifyWait(0x00,             /* Don't clear any bits on entry. */
                    ULONG_MAX,        /* Clear all bits on exit. */
                    &ulNotifiedValue, /* Receives the notification value. */
                    portMAX_DELAY);   /* Block indefinitely. */
    if (run_battery_task(current_sensor_readings)) {
      HAL_GPIO_WritePin(HIGH_VOLTAGE_DISCONNECT_GPIO_Port,
                        HIGH_VOLTAGE_DISCONNECT_Pin, GPIO_PIN_SET);
    } else {
      if (!battery_task_awake)
        ADC1->IER |= (1 << 8); // adc wdg 2 enable
    }
  }
}
