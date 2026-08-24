#include "battery.h"

const osThreadAttr_t battery_attributes = {
    .name = "battery",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityISR,
};

/* USER CODE BEGIN Header_StartTask03 */
/**
 * @brief Function implementing the battery thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask03 */
void Battery_Task(void *argument) {
  /* USER CODE BEGIN StartTask03 */

  uint32_t ulNotifiedValue;
  uint32_t t_delta = 100000;
  uint32_t max_time = t_delta * 2;
  uint32_t time = 0;
  uint32_t max_sustained = square(80 - 75) * t_delta *
                           2; /* Σ (75-80)^2 * 2  = 50 A^2 * s // maximum  */

  for (;;) {
    xTaskNotifyWait(0x00,             /* Don't clear any bits on entry. */
                    ULONG_MAX,        /* Clear all bits on exit. */
                    &ulNotifiedValue, /* Receives the notification value. */
                    portMAX_DELAY);   /* Block indefinitely. */

    // time_delta = 1/100,000 or 10 micro seconds
    // to keep the math in integers, 1 time delta will be treated as 1

    /* Σ (80 - I_measured)^2 * delta_t */

    // (2 second curve) t_dela to ms to s
    if (time < max_time) {
      for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
        // no need to multiply by 100k here since we only need one time delta
        // and one time delta is 1
        riemann_sum_total += square(current_sensor_readings[i] - 75);
        if (riemann_sum_total > max_sustained) {
          HAL_GPIO_WritePin(HIGH_VOLTAGE_DISCONNECT_GPIO_Port,
                            HIGH_VOLTAGE_DISCONNECT_Pin, GPIO_PIN_SET);
        }
      }
      time += ADC_CURRENT_SAMPLE_COUNT;
    } else {
      time = 0;
      riemann_sum_total = 0;
    }
    // restart watchdog and block for more samples
    ADC1->IER &= ~(1 << 8);
    // next todo: implment state so this can survive for two seconds
  }
  /* USER CODE END StartTask03 */
}
