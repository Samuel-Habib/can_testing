
#include "main.h"

/* USER CODE BEGIN Header_StartTask04 */
/**
 * @brief Function implementing the wifi thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask04 */
void StartTask04(void *argument) {
  /* USER CODE BEGIN StartTask04 */
  /* Infinite loop */
  // wifi11111

  xQueueReset(xLogQueue);
  for (;;) {
    osDelay(10);
    static uint8_t task4str[60] = "========== task 4 enterd  =========\r \n";
    HAL_UART_Transmit(&huart1, task4str, sizeof(task4str), 1000);
    static char sampleData[128] = "Sample data \r \n";
    static char str1[128] = "string1 \r \n";
    static char str2[128] = "string2 \r \n";
    static char str3[128] = "string3 \r \n";
    static char str4[128] = "string4 \r \n";
    static char str5[128] = {0};

    xQueueSend(xLogQueue, (void *)&str5, portMAX_DELAY);
    osDelay(1000);
    HAL_Delay(1000);
  }
  /* USER CODE END StartTask04 */
}
