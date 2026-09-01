#ifndef FAKE_STM32_HAL_H
#define FAKE_STM32_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fff.h"
#include "stm32h7xx_hal.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* =========================================================================
 * FFF Fake Function Declarations for STM32 HAL
 * ========================================================================= */
DECLARE_FAKE_VOID_FUNC3(HAL_GPIO_WritePin, GPIO_TypeDef*, uint16_t, GPIO_PinState);
DECLARE_FAKE_VOID_FUNC2(HAL_GPIO_TogglePin, GPIO_TypeDef*, uint16_t);
DECLARE_FAKE_VALUE_FUNC2(GPIO_PinState, HAL_GPIO_ReadPin, GPIO_TypeDef*, uint16_t);
DECLARE_FAKE_VOID_FUNC2(HAL_GPIO_Init, GPIO_TypeDef*, const GPIO_InitTypeDef*);

DECLARE_FAKE_VALUE_FUNC4(HAL_StatusTypeDef, HAL_UART_Transmit, UART_HandleTypeDef*, const uint8_t*, uint16_t, uint32_t);
DECLARE_FAKE_VALUE_FUNC4(HAL_StatusTypeDef, HAL_UART_Receive, UART_HandleTypeDef*, uint8_t*, uint16_t, uint32_t);

DECLARE_FAKE_VALUE_FUNC1(HAL_StatusTypeDef, HAL_IWDG_Refresh, IWDG_HandleTypeDef*);

DECLARE_FAKE_VALUE_FUNC3(HAL_StatusTypeDef, HAL_FDCAN_AddMessageToTxFifoQ, FDCAN_HandleTypeDef*, const FDCAN_TxHeaderTypeDef*, const uint8_t*);
DECLARE_FAKE_VALUE_FUNC2(uint32_t, HAL_FDCAN_GetRxFifoFillLevel, FDCAN_HandleTypeDef*, uint32_t);
DECLARE_FAKE_VALUE_FUNC4(HAL_StatusTypeDef, HAL_FDCAN_GetRxMessage, FDCAN_HandleTypeDef*, uint32_t, FDCAN_RxHeaderTypeDef*, uint8_t*);
DECLARE_FAKE_VALUE_FUNC3(HAL_StatusTypeDef, HAL_FDCAN_ConfigInterruptLines, FDCAN_HandleTypeDef*, uint32_t, uint32_t);
DECLARE_FAKE_VOID_FUNC2(HAL_FDCAN_RxFifo0Callback, FDCAN_HandleTypeDef*, uint32_t);

DECLARE_FAKE_VOID_FUNC1(HAL_Delay, uint32_t);
DECLARE_FAKE_VALUE_FUNC0(uint32_t, HAL_GetTick);

/* =========================================================================
 * FFF Fake Function Declarations for FreeRTOS / CMSIS
 * ========================================================================= */
DECLARE_FAKE_VALUE_FUNC1(osStatus_t, osDelay, uint32_t);
DECLARE_FAKE_VALUE_FUNC0(uint32_t, osKernelGetTickCount);

DECLARE_FAKE_VALUE_FUNC3(BaseType_t, xQueueSend, QueueHandle_t, const void*, TickType_t);
DECLARE_FAKE_VALUE_FUNC3(BaseType_t, xQueueSendToBack, QueueHandle_t, const void*, TickType_t);
DECLARE_FAKE_VALUE_FUNC3(BaseType_t, xQueueReceive, QueueHandle_t, void*, TickType_t);
DECLARE_FAKE_VALUE_FUNC1(BaseType_t, xQueueReset, QueueHandle_t);
DECLARE_FAKE_VALUE_FUNC1(UBaseType_t, uxQueueMessagesWaiting, const QueueHandle_t);

DECLARE_FAKE_VOID_FUNC0(taskENTER_CRITICAL);
DECLARE_FAKE_VOID_FUNC0(taskEXIT_CRITICAL);
DECLARE_FAKE_VALUE_FUNC4(BaseType_t, xTaskNotifyWait, uint32_t, uint32_t, uint32_t*, TickType_t);

DECLARE_FAKE_VOID_FUNC0(ADC_IRQHandler);
DECLARE_FAKE_VOID_FUNC0(DMA1_Stream0_IRQHandler);
DECLARE_FAKE_VOID_FUNC0(DMA1_Stream1_IRQHandler);
DECLARE_FAKE_VOID_FUNC0(Error_Handler);

/**
 * @brief Resets all fakes, mock peripheral registers, and application globals.
 * Call this in your test fixture setUp() function.
 */
void fake_stm32_hal_reset_all(void);

#ifdef __cplusplus
}
#endif

#endif /* FAKE_STM32_HAL_H */
