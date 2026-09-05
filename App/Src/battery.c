#include "battery.h"
#include "main.h"
#include "projdefs.h"
#include "stm32h7xx_hal_gpio.h"

const osThreadAttr_t battery_attributes = {
    .name = "battery",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityISR,
};

bool battery_task_awake = false;
static uint32_t time = 0;
bool run_battery_task(volatile unsigned int *curr_sensor_readings) {

  uint32_t t_delta = 100000;
  uint32_t max_time = t_delta * 2;
  if (battery_task_awake == false)
    time = 0;
  battery_task_awake = true;
  uint32_t max_sustained = square(80 - 75) * t_delta *
                           2; /* Σ (75-80)^2 * 2  = 50 A^2 * s // maximum  */
  // char str[128] = "First Message \r \n \t";
  //
  // xQueueSendToBack(xLogQueue, (void *)str, pdMS_TO_TICKS(1000));

  // time_delta = 1/100,000 or 10 micro seconds
  // to keep the math in integers, 1 time delta will be treated as 1

  /* Σ (80 - I_measured)^2 * delta_t */

  // (2 second curve) t_dela to ms to s
  if (time < max_time) {
    for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
      // no need to multiply by 100k here since we only need one time delta
      // and one time delta is 1
      riemann_sum_total += square(curr_sensor_readings[i] - 75);
      if (riemann_sum_total > max_sustained) {
        return true;
      }
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

      ADC1->IER &= ~(1 << 8); // note this is watchdog 2 the over 100 watchdog
                              // not the over 300 watchdog
    }
  }
}
