#include "fake_stm32_hal.h"
#include "main.h"

/* Define FFF global storage */
DEFINE_FFF_GLOBALS;

/* =========================================================================
 * FFF Fake Function Definitions for STM32 HAL
 * ========================================================================= */
DEFINE_FAKE_VOID_FUNC3(HAL_GPIO_WritePin, GPIO_TypeDef *, uint16_t,
                       GPIO_PinState);
DEFINE_FAKE_VOID_FUNC2(HAL_GPIO_TogglePin, GPIO_TypeDef *, uint16_t);
DEFINE_FAKE_VALUE_FUNC2(GPIO_PinState, HAL_GPIO_ReadPin, GPIO_TypeDef *,
                        uint16_t);
DEFINE_FAKE_VOID_FUNC2(HAL_GPIO_Init, GPIO_TypeDef *, const GPIO_InitTypeDef *);

DEFINE_FAKE_VALUE_FUNC4(HAL_StatusTypeDef, HAL_UART_Transmit,
                        UART_HandleTypeDef *, const uint8_t *, uint16_t,
                        uint32_t);
DEFINE_FAKE_VALUE_FUNC4(HAL_StatusTypeDef, HAL_UART_Receive,
                        UART_HandleTypeDef *, uint8_t *, uint16_t, uint32_t);

DEFINE_FAKE_VALUE_FUNC1(HAL_StatusTypeDef, HAL_IWDG_Refresh,
                        IWDG_HandleTypeDef *);

DEFINE_FAKE_VALUE_FUNC3(HAL_StatusTypeDef, HAL_FDCAN_AddMessageToTxFifoQ,
                        FDCAN_HandleTypeDef *, const FDCAN_TxHeaderTypeDef *,
                        const uint8_t *);
DEFINE_FAKE_VALUE_FUNC2(uint32_t, HAL_FDCAN_GetRxFifoFillLevel,
                        FDCAN_HandleTypeDef *, uint32_t);
DEFINE_FAKE_VALUE_FUNC4(HAL_StatusTypeDef, HAL_FDCAN_GetRxMessage,
                        FDCAN_HandleTypeDef *, uint32_t,
                        FDCAN_RxHeaderTypeDef *, uint8_t *);
DEFINE_FAKE_VALUE_FUNC3(HAL_StatusTypeDef, HAL_FDCAN_ConfigInterruptLines,
                        FDCAN_HandleTypeDef *, uint32_t, uint32_t);
DEFINE_FAKE_VOID_FUNC2(HAL_FDCAN_RxFifo0Callback, FDCAN_HandleTypeDef *,
                       uint32_t);

DEFINE_FAKE_VOID_FUNC1(HAL_Delay, uint32_t);
DEFINE_FAKE_VALUE_FUNC0(uint32_t, HAL_GetTick);

/* =========================================================================
 * FFF Fake Function Definitions for FreeRTOS / CMSIS
 * ========================================================================= */
DEFINE_FAKE_VALUE_FUNC1(osStatus_t, osDelay, uint32_t);
DEFINE_FAKE_VALUE_FUNC0(uint32_t, osKernelGetTickCount);

DEFINE_FAKE_VALUE_FUNC3(BaseType_t, xQueueSend, QueueHandle_t, const void *,
                        TickType_t);
DEFINE_FAKE_VALUE_FUNC3(BaseType_t, xQueueSendToBack, QueueHandle_t,
                        const void *, TickType_t);
DEFINE_FAKE_VALUE_FUNC3(BaseType_t, xQueueReceive, QueueHandle_t, void *,
                        TickType_t);
DEFINE_FAKE_VALUE_FUNC1(BaseType_t, xQueueReset, QueueHandle_t);
DEFINE_FAKE_VALUE_FUNC1(UBaseType_t, uxQueueMessagesWaiting,
                        const QueueHandle_t);

DEFINE_FAKE_VOID_FUNC0(taskENTER_CRITICAL);
DEFINE_FAKE_VOID_FUNC0(taskEXIT_CRITICAL);
DEFINE_FAKE_VALUE_FUNC4(BaseType_t, xTaskNotifyWait, uint32_t, uint32_t,
                        uint32_t *, TickType_t);

DEFINE_FAKE_VOID_FUNC0(ADC_IRQHandler);
DEFINE_FAKE_VOID_FUNC0(DMA1_Stream0_IRQHandler);
DEFINE_FAKE_VOID_FUNC0(DMA1_Stream1_IRQHandler);
DEFINE_FAKE_VOID_FUNC0(Error_Handler);

/* =========================================================================
 * Peripheral Instance Mock Objects
 * ========================================================================= */
GPIO_TypeDef mock_GPIOG;
USART_TypeDef mock_USART1;
ADC_TypeDef mock_ADC1;
DMA_Stream_TypeDef mock_DMA1_Stream0;
DMA_Stream_TypeDef mock_DMA1_Stream1;
DMA_TypeDef mock_DMA1;
TIM_TypeDef mock_TIM1;
TIM_TypeDef mock_TIM2;
FDCAN_GlobalTypeDef mock_FDCAN1;
IWDG_TypeDef mock_IWDG1;

/* =========================================================================
 * Application Global Variables (declared in main.h)
 * ========================================================================= */
int32_t riemann_sum_total = 0;
volatile unsigned int current_sensor_readings[ADC_CURRENT_SAMPLE_COUNT] = {0};
volatile uint32_t overcurrent_samples_count = 0;

unsigned char uart_buffer[UART_BUFFER_SIZE] = {0};
uint16_t buffer_total = 0;
uint16_t head = 0;
uint16_t tail = 0;
bool uart_buffer_full = false;

uint32_t current_sample = 0;
uint32_t small_overcurrent_sanples_count = 0;

osThreadId_t batteryHandle = NULL;
QueueHandle_t xLogQueue = (QueueHandle_t)0x1234;
UART_HandleTypeDef huart1 = {.Instance = &mock_USART1};
IWDG_HandleTypeDef hiwdg1 = {.Instance = &mock_IWDG1};
FDCAN_HandleTypeDef hfdcan1 = {.Instance = &mock_FDCAN1};

void fake_stm32_hal_reset_all(void) {
  /* Reset HAL Fakes */
  RESET_FAKE(HAL_GPIO_WritePin);
  RESET_FAKE(HAL_GPIO_TogglePin);
  RESET_FAKE(HAL_GPIO_ReadPin);
  RESET_FAKE(HAL_GPIO_Init);
  RESET_FAKE(HAL_UART_Transmit);
  RESET_FAKE(HAL_UART_Receive);
  RESET_FAKE(HAL_IWDG_Refresh);
  RESET_FAKE(HAL_FDCAN_AddMessageToTxFifoQ);
  RESET_FAKE(HAL_FDCAN_GetRxFifoFillLevel);
  RESET_FAKE(HAL_FDCAN_GetRxMessage);
  RESET_FAKE(HAL_FDCAN_ConfigInterruptLines);
  RESET_FAKE(HAL_FDCAN_RxFifo0Callback);
  RESET_FAKE(HAL_Delay);
  RESET_FAKE(HAL_GetTick);

  /* Reset FreeRTOS / CMSIS Fakes */
  RESET_FAKE(osDelay);
  RESET_FAKE(osKernelGetTickCount);
  RESET_FAKE(xQueueSend);
  RESET_FAKE(xQueueSendToBack);
  RESET_FAKE(xQueueReceive);
  RESET_FAKE(xQueueReset);
  RESET_FAKE(uxQueueMessagesWaiting);
  RESET_FAKE(taskENTER_CRITICAL);
  RESET_FAKE(taskEXIT_CRITICAL);
  RESET_FAKE(xTaskNotifyWait);
  RESET_FAKE(ADC_IRQHandler);
  RESET_FAKE(DMA1_Stream0_IRQHandler);
  RESET_FAKE(DMA1_Stream1_IRQHandler);
  RESET_FAKE(Error_Handler);

  /* Reset Peripherals */
  memset(&mock_GPIOG, 0, sizeof(mock_GPIOG));
  memset(&mock_USART1, 0, sizeof(mock_USART1));
  memset(&mock_ADC1, 0, sizeof(mock_ADC1));
  memset(&mock_DMA1_Stream0, 0, sizeof(mock_DMA1_Stream0));
  memset(&mock_DMA1_Stream1, 0, sizeof(mock_DMA1_Stream1));
  memset(&mock_DMA1, 0, sizeof(mock_DMA1));
  memset(&mock_TIM1, 0, sizeof(mock_TIM1));
  memset(&mock_TIM2, 0, sizeof(mock_TIM2));
  memset(&mock_FDCAN1, 0, sizeof(mock_FDCAN1));
  memset(&mock_IWDG1, 0, sizeof(mock_IWDG1));

  /* Reset Application Globals */
  riemann_sum_total = 0;
  memset((void *)current_sensor_readings, 0, sizeof(current_sensor_readings));
  overcurrent_samples_count = 0;
  memset(uart_buffer, 0, sizeof(uart_buffer));
  buffer_total = 0;
  head = 0;
  tail = 0;
  uart_buffer_full = false;
  current_sample = 0;
  small_overcurrent_sanples_count = 0;
  batteryHandle = NULL;
  xLogQueue = (QueueHandle_t)0x1234;
  huart1.Instance = &mock_USART1;
  hiwdg1.Instance = &mock_IWDG1;
  hfdcan1.Instance = &mock_FDCAN1;
}
