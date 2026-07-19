/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32h7xx_it.c
 * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_it.h"
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "stm32h7xx_hal_cortex.h"
#include "task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern TIM_HandleTypeDef htim1;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1) {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void) {
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void) {
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void) {
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles DMA1 stream0 global interrupt.
 */
void DMA1_Stream0_IRQHandler(void) {
  /* USER CODE BEGIN DMA1_Stream0_IRQn 0 */

  // logging (lower priority)
  DMA1->LIFCR = DMA_LIFCR_CTCIF0; // ctcif
  DMA1_Stream0->CR &= 0;          // disable dma
  tail = (tail + MAX_MESSAGE_LEN) % UART_BUFFER_SIZE;

  uint32_t x = taskENTER_CRITICAL_FROM_ISR();
  buffer_total -= MAX_MESSAGE_LEN;
  taskEXIT_CRITICAL_FROM_ISR(x);

  uart_buffer_full = false;
  if (buffer_total == 0)
    DMA1_Stream0->CR &= 0; // en = 0;
  // disable the dma when the ring buffer is empty
  else {
    // reload ndtr and move on to the next message
    DMA1_Stream0->NDTR = MAX_MESSAGE_LEN; // bytes
    DMA1_Stream0->M0AR = (uint32_t)&(uart_buffer[tail]);
    DMA1_Stream0->CR |= 1; // en = 1;
  }

  /* USER CODE END DMA1_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA1_Stream0_IRQn 1 */

  /* USER CODE END DMA1_Stream0_IRQn 1 */
}

/**
 * @brief This function handles DMA1 stream1 global interrupt.
 */
void DMA1_Stream1_IRQHandler(void) {
  /* USER CODE BEGIN DMA1_Stream1_IRQn 0 */

  // dma1111
  //  clear the flag
  DMA1->LIFCR = DMA_LIFCR_CHTIF1;

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR(batteryHandle, 0, 0, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  /* all this does is notify the task at a regular interval set by timer 2*/
  /* the deffered task should finish executing well below this interrupt
   * which should fire every 1 ms*/

  /* USER CODE END DMA1_Stream1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Stream1_IRQn 1 */

  /* USER CODE END DMA1_Stream1_IRQn 1 */
}

/**
 * @brief This function handles ADC1 and ADC2 global interrupts.
 */
void ADC_IRQHandler(void) {
  /* USER CODE BEGIN ADC_IRQn 0 */

  // EOC
  // watchdog 1 isr
  if ((ADC1->ISR & (1 << 7)) & (ADC1->IER & (1 << 7))) {
    ADC1->ISR = (1 << 7);
    // eoc checks for consecutive samples
    // turn the watchdog off while the eoc is running
    // also disable other watchdogs as this has "higher priority"
    // this also means that both wdgs do not have access to the wdg_state
    // variable at the same time

    if (!(ADC1->IER & (1 << 2))) {
      ADC1->IER |= (1 << 2);
      ADC1->IER &= ~(1 << 7);
      ADC1->IER &= ~(1 << 8);
    } else {
    }
  }

  // watchdog 2 isr if current greater than 100%
  // adc111
  if ((ADC1->ISR & (1 << 8)) & (ADC1->IER & (1 << 8))) {
    ADC1->ISR = (1 << 8);

    if (!(ADC1->IER & (1 << 2))) {
      ADC1->IER |= (1 << 2);
      ADC1->IER &= ~(1 << 8);
    }
  }

  if ((ADC1->ISR & (1 << 2)) & (ADC1->IER & (1 << 2))) {
    ADC1->ISR = (1 << 2);
    current_sample = ADC1->DR;

    if (current_sample >= SHORT_CIRCUIT_THRESHOLD) {
      // this would result in false in the case of the second watchdog
      // or if the second watchdog missed it, it will record the sample
      overcurrent_samples_count++;

      if (overcurrent_samples_count > SHORT_CURRENT_SAMPLE_THRESHOLD) {
        HAL_GPIO_WritePin(HIGH_VOLTAGE_DISCONNECT_GPIO_Port,
                          HIGH_VOLTAGE_DISCONNECT_Pin, GPIO_PIN_SET);
        // disable isr calls to prevent starving the rest of the system
        // at this point wd2, wd2 and the eoc are all disabled
        ADC1->IER &= ~(1 << 2);
        overcurrent_samples_count = 0;
      }
    } else {
      // if short was transiet, stop the eoc and restart the watchdog to check
      // for shorts

      overcurrent_samples_count = 0;
      ADC1->IER &= ~(1 << 8);
      ADC1->IER |= (1 << 7);
    }

    if (current_sample > CURRENT_RATING) {
      small_overcurrent_sanples_count++;
      /* TODO: rename to current_sample_threshold */
      if (small_overcurrent_sanples_count > SHORT_CURRENT_SAMPLE_THRESHOLD) {
        // disable watchdog 2 and eoc until task finishes
        // enable dma interupts for regular sample readings to the deferred task
        ADC1->IER &= ~(1 << 8);
        ADC1->IER &= ~(1 << 2);
        DMA1_Stream1->CR |= DMA_SxCR_HTIE;
        small_overcurrent_sanples_count = 0;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTaskNotifyFromISR(batteryHandle, 0, 0, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
      }
    }
  }

  /* USER CODE END ADC_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc1);
  /* USER CODE BEGIN ADC_IRQn 1 */

  /* USER CODE END ADC_IRQn 1 */
}

/**
 * @brief This function handles TIM1 update interrupt.
 */
void TIM1_UP_IRQHandler(void) {
  /* USER CODE BEGIN TIM1_UP_IRQn 0 */

  TIM1->CR =
      /* USER CODE END TIM1_UP_IRQn 0 */
      HAL_TIM_IRQHandler(&htim1);
  /* USER CODE BEGIN TIM1_UP_IRQn 1 */

  /* USER CODE END TIM1_UP_IRQn 1 */
}

/* USER CODE BEGIN 1 */

void HAL_SYSTICK_IRQHandler()

    /* USER CODE END 1 */
