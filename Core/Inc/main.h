/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "queue.h"
#include "semphr.h"
#include <stdbool.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

#define HIGH_VOLTAGE_DISCONNECT_Pin GPIO_PIN_6
#define HIGH_VOLTAGE_DISCONNECT_GPIO_Port GPIOG
#define mbaCONTROL_MESSAGE_BUFFER_SIZE (24)
#define UART_BUFFER_SIZE (512)
#define MAX_MESSAGE_LEN (128)
#define CURRENT_RATING 100
#define ADC_CURRENT_SAMPLE_COUNT 100
#define SHORT_CURRENT_SAMPLE_THRESHOLD 3
#define SHORT_CIRCUIT_THRESHOLD CURRENT_RATING * 3
#define t                                                                      \
  (160000000 / (1599 + 1)) // 1/ (main clock / ARR+1) or 1/trigger_frequency

extern uint32_t riemann_sum_total;
extern volatile unsigned int current_sensor_readings[ADC_CURRENT_SAMPLE_COUNT];
extern volatile uint32_t overcurrent_samples_count;

extern unsigned char uart_buffer[UART_BUFFER_SIZE];
extern uint16_t buffer_total;
extern uint16_t head;
extern uint16_t tail;
extern bool uart_buffer_full;

extern uint32_t current_sample;
extern uint32_t small_overcurrent_sanples_count;

extern osThreadId_t batteryHandle;
extern QueueHandle_t xLogQueue;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
