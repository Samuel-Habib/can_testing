#include "watchdog.h"

const osThreadAttr_t watchdog_attributes = {
    .name = "watchdog",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,

};

/* USER CODE BEGIN Header_StartTask05 */
/**
 * @brief Function implementing the watchdog thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask05 */
void Watchdog_Task(void *argument) {
  /* USER CODE BEGIN StartTask05 */
  /* Infinite loop */
  for (;;) {
    HAL_IWDG_Refresh(&hiwdg1);
    osDelay(100);
  }
  /* USER CODE END StartTask05 */
}

/* USER CODE BEGIN Header_StartTask06 */
/**
 * @brief Function implementing the can_handler thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask06 */
void StartTask06(void *argument) {
  /* USER CODE BEGIN StartTask06 */
  /* Infinite loop */
  // can6666
  char *testString = "this is a test of the char uart buffer scheme";
  xQueueReset(xLogQueue);
  for (;;) {

    osDelay(100);
    // xQueueSendToBack(xLogQueue, testString, 0);
  }
  /* USER CODE END StartTask06 */
}
