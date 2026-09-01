#ifndef MOCK_TASK_H
#define MOCK_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"

typedef void * TaskHandle_t;

void taskENTER_CRITICAL(void);
void taskEXIT_CRITICAL(void);

BaseType_t xTaskNotifyWait(uint32_t ulBitsToClearOnEntry,
                           uint32_t ulBitsToClearOnExit,
                           uint32_t *pulNotificationValue,
                           TickType_t xTicksToWait);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_TASK_H */
