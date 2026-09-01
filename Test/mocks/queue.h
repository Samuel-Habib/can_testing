#ifndef MOCK_QUEUE_H
#define MOCK_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"

typedef void * QueueHandle_t;

BaseType_t xQueueSend(QueueHandle_t xQueue, const void * pvItemToQueue, TickType_t xTicksToWait);
BaseType_t xQueueSendToBack(QueueHandle_t xQueue, const void * pvItemToQueue, TickType_t xTicksToWait);
BaseType_t xQueueReceive(QueueHandle_t xQueue, void * pvBuffer, TickType_t xTicksToWait);
BaseType_t xQueueReset(QueueHandle_t xQueue);
UBaseType_t uxQueueMessagesWaiting(const QueueHandle_t xQueue);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_QUEUE_H */
