

#include "SEGGER_SYSVIEW_FreeRTOS.h"
#include "can.h"
#include "main.h"
#include "stm32h7xx_hal_fdcan.h"

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument) {
  /* USER CODE BEGIN 5 */
  SEGGER_SYSVIEW_Start();

  osThreadNew(StartTask02, NULL, &logging_attributes);

  static const uint8_t tx_data_bufer[] = "x\r\n";
  static uint8_t rx_data_bufer[8];
  // 11111
  /* Infinite loop */
  for (;;) {
    //  HAL_Delay(200);
    //    static uint8_t h[16] = "FIRST TASK \r \n";
    //   HAL_UART_Transmit(&huart1, h, sizeof(h), 1000);

    // [ ] Set the mpu
    // [ ] uart
    // [ ] can
    osDelay(100000);

    can_poll_rx(&hfdcan1, rx_data_bufer);
    can_tx(&hfdcan1, tx_data_bufer);

    osDelay(200);
    HAL_UART_Transmit(&huart1, rx_data_bufer, sizeof(rx_data_bufer), 1000);
    osDelay(200);
  }
  /* USER CODE END 5 */
}
