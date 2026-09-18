#include "logging.h"

const osThreadAttr_t logging_attributes = {
    .name = "logging",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};

/* Car Subsystems:
 *  - MPPT
 *  - BMS
 *  - Motor Controller
 *  - Dashboard
 *  - Telemetry
 *      Speed, battery data, motor data, gps, temps,
 *      MPPT status, faults
 *  - Lights, Turn Signals
 *
 *  All goes into real time debugging metrics
 *  over CAN
 *
 * */

void send_log_message(const char *data) {
  if (data == NULL)
    return;
  char message_buffer[MAX_MESSAGE_LEN] = {0};
  strncpy(message_buffer, data, sizeof(message_buffer));
  xQueueSendToBack(xLogQueue, message_buffer, pdMS_TO_TICKS(100));
}

char buffer[128];

void Logging_Task(void *argument) {
  // gatekeeper task

  char data[128] = {0};
  xQueueReset(xLogQueue);
  for (;;) {
    if (xQueueReceive(xLogQueue, &data, pdMS_TO_TICKS(2000)) == pdPASS) {
      HAL_UART_Transmit(&huart1, (uint8_t *)data, strlen(data), 1000);
    }
  }
}
