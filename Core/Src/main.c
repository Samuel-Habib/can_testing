/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SEGGER_SYSVIEW_FreeRTOS.h"
#include "can.h"
#include "cmsis_os2.h"
#include "logging.h"
#include "message_buffer.h"
#include "portmacro.h"
#include "projdefs.h"
#include "stm32h753xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_dma.h"
#include "stm32h7xx_hal_fdcan.h"
#include "stm32h7xx_hal_gpio.h"
#include "stm32h7xx_hal_uart.h"
#include "stm32h7xx_it.h"
#include "task.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

FDCAN_HandleTypeDef hfdcan1;

IWDG_HandleTypeDef hiwdg1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for logging */
osThreadId_t loggingHandle;
const osThreadAttr_t logging_attributes = {
    .name = "logging",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for battery */
osThreadId_t batteryHandle;
const osThreadAttr_t battery_attributes = {
    .name = "battery",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityRealtime,
};
/* Definitions for wifi */
osThreadId_t wifiHandle;
const osThreadAttr_t wifi_attributes = {
    .name = "wifi",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityBelowNormal,
};
/* Definitions for watchdog */
osThreadId_t watchdogHandle;
const osThreadAttr_t watchdog_attributes = {
    .name = "watchdog",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for can_handler */
osThreadId_t can_handlerHandle;
const osThreadAttr_t can_handler_attributes = {
    .name = "can_handler",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* USER CODE BEGIN PV */

char howMany[128];
osStatus_t status;
char buf[128];
uint32_t riemann_sum_total = 0;
volatile unsigned int current_sensor_readings[ADC_CURRENT_SAMPLE_COUNT];
volatile uint32_t overcurrent_samples_count = 0;

QueueHandle_t xLogQueue;
SemaphoreHandle_t testSemaphore = NULL;
unsigned char uart_buffer[UART_BUFFER_SIZE];
uint16_t buffer_total = 0;
uint16_t head = 0;
uint16_t tail = 0;
bool uart_buffer_full = false;

uint32_t current_sample = 0;
uint32_t small_overcurrent_sanples_count = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_IWDG1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);
void StartTask04(void *argument);
void StartTask05(void *argument);
void StartTask06(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static inline uint32_t square(int32_t a) { return a * a; }

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* USER CODE BEGIN 1 */
  long ispr = __get_IPSR();
  snprintf(buf, sizeof(buf), "(IPSR=%lu)\r\n", ispr);
  HAL_UART_Transmit(&huart1, (uint8_t *)buf, strlen(buf), 1000);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FDCAN1_Init();
  MX_USART1_UART_Init();
  MX_IWDG1_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  SEGGER_SYSVIEW_Conf();
  SEGGER_SYSVIEW_Start();

  const FDCAN_FilterTypeDef Can_ConfigFilter = {
      .IdType = FDCAN_STANDARD_ID,
      .FilterIndex = 0,
      .FilterType = FDCAN_FILTER_MASK,
      .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
      .FilterID1 = 0,
      .FilterID2 = 0x7FF
      //  .RxBufferIndex = ; not used
      //  .IsCalibrationMsg = ; not used

  };
  HAL_FDCAN_ConfigFilter(&hfdcan1, &Can_ConfigFilter);
  HAL_FDCAN_Start(&hfdcan1);

  // dma
  DMA_InitTypeDef hdma_init = {
      .Direction = DMA_PERIPH_TO_MEMORY,
      .PeriphDataAlignment = DMA_PDATAALIGN_WORD,
      .MemInc = DMA_MINC_ENABLE,
      .MemDataAlignment = DMA_MDATAALIGN_WORD,
      .Mode = DMA_CIRCULAR,
      .FIFOMode = DMA_FIFOMODE_ENABLE,
      .FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL,

  };
  // override watchdog threshold

  hdma_usart1_rx.Init = hdma_init;
  hdma_adc1.Init = hdma_init;

  DMA1_Stream1->PAR =
      (uint32_t)&(ADC1->DR); // replace with peripherial data address
  DMA1_Stream1->M0AR =
      (uint32_t)&current_sensor_readings; // begining of mem address

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */

  // extern SemaphoreHandle_t uartMutex;
  // uartMutex = xSemaphoreCreateMutex();
  //  why can't i do this
  //  extern SemaphoreHandle_t uartMutex = xSemaphoreCreateMutex();

  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  testSemaphore = xSemaphoreCreateBinary();
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */

  xLogQueue = xQueueCreate(10, sizeof(char) * MAX_MESSAGE_LEN);
  // try this later, right now focus on implementing a gatekeeper with a
  // regular queue
  //  static MessageBufferHandle_t xControlMessageBuffer;
  //  xControlMessageBuffer =
  //  xMessageBufferCreate(mbaCONTROL_MESSAGE_BUFFER_SIZE);

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle =
      osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of logging */
  loggingHandle = osThreadNew(StartTask02, NULL, &logging_attributes);

  /* creation of battery */
  batteryHandle = osThreadNew(StartTask03, NULL, &battery_attributes);

  /* creation of wifi */
  wifiHandle = osThreadNew(StartTask04, NULL, &wifi_attributes);

  /* creation of watchdog */
  watchdogHandle = osThreadNew(StartTask05, NULL, &watchdog_attributes);

  /* creation of can_handler */
  can_handlerHandle = osThreadNew(StartTask06, NULL, &can_handler_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  // this isn't in cmiss?
  /* SysTick->CTRL = 0; */
  /* SysTick->VAL = 0; */
  /* SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk; */
  /* __HAL_DBGMCU_FREEZE_TIM1(); */
  status = osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {

    // snprintf(buf, sizeof(buf), "Kernel start returned: 0x%X (IPSR=%lu)\r\n",
    //        status, __get_IPSR());
    // HAL_UART_Transmit(&huart1, (uint8_t *)buf, strlen(buf), 1000);
    // HAL_Delay(1000);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
   */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV4;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T2_TRGO;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.Oversampling.Ratio = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
   */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK) {
    Error_Handler();
  }

  /** Configure Analog WatchDog 1
   */
  AnalogWDGConfig.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
  AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
  AnalogWDGConfig.Channel = ADC_CHANNEL_2;
  AnalogWDGConfig.ITMode = ENABLE;
  AnalogWDGConfig.HighThreshold = 400;
  AnalogWDGConfig.LowThreshold = 0;
  if (HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig) != HAL_OK) {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_16CYCLES_5;
  sConfig.SingleDiff = ADC_DIFFERENTIAL_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief FDCAN1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_FDCAN1_Init(void) {

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 3;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 13;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.RxFifo0ElmtsNbr = 0;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxFifo1ElmtsNbr = 0;
  hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 0;
  hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 0;
  hfdcan1.Init.TxBuffersNbr = 0;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */
}

/**
 * @brief IWDG1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_IWDG1_Init(void) {

  /* USER CODE BEGIN IWDG1_Init 0 */

  /* USER CODE END IWDG1_Init 0 */

  /* USER CODE BEGIN IWDG1_Init 1 */

  /* USER CODE END IWDG1_Init 1 */
  hiwdg1.Instance = IWDG1;
  hiwdg1.Init.Prescaler = IWDG_PRESCALER_4;
  hiwdg1.Init.Window = 4095;
  hiwdg1.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg1) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG1_Init 2 */

  /* USER CODE END IWDG1_Init 2 */
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 159999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) !=
      HAL_OK) {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) !=
      HAL_OK) {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(HIGH_VOLTAGE_DISCONNECT_GPIO_Port,
                    HIGH_VOLTAGE_DISCONNECT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : HIGH_VOLTAGE_DISCONNECT_Pin */
  GPIO_InitStruct.Pin = HIGH_VOLTAGE_DISCONNECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HIGH_VOLTAGE_DISCONNECT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument) {
  /* USER CODE BEGIN 5 */

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

/* USER CODE BEGIN Header_StartTask02 */
/**
 * @brief Function implementing the logging thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument) {
  /* USER CODE BEGIN StartTask02 */
  // gatekeeper task
  // t2222

  char noMessages[19] = "No New Messages \r \n";
  char debugBuf[128];
  char data[128] = {0};
  xQueueReset(xLogQueue);
  for (;;) {

    xQueueReceive(xLogQueue, &data, pdMS_TO_TICKS(2000));

    if (!uart_buffer_full && uxQueueMessagesWaiting(xLogQueue) > 0) {
      taskENTER_CRITICAL();
      buffer_total += MAX_MESSAGE_LEN;
      taskEXIT_CRITICAL();
      if (log_module(data) == -1) {
        osDelay(300);
      }
    }

    snprintf(howMany, sizeof(howMany),
             "there are %lu items in the queue and %d bytes in the buffer, and "
             "buffer_total: %d \r \n",
             uxQueueMessagesWaiting(xLogQueue), sizeof(uart_buffer),
             buffer_total);
    HAL_UART_Transmit(&huart1, (uint8_t *)howMany, sizeof(howMany), 1000);
    HAL_UART_Transmit(&huart1, uart_buffer, sizeof(uart_buffer), 1000);

    osDelay(1);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
 * @brief Function implementing the battery thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument) {
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  // 3333
  uint32_t ulNotifiedValue;
  uint32_t t_delta = 100000;
  uint32_t max_time = t_delta * 2;
  uint32_t time = 0;
  uint32_t max_sustained = square(80 - 75) * t_delta *
                           2; /* Σ (75-80)^2 * 2  = 50 A^2 * s // maximum  */

  for (;;) {
    xTaskNotifyWait(0x00,             /* Don't clear any bits on entry. */
                    ULONG_MAX,        /* Clear all bits on exit. */
                    &ulNotifiedValue, /* Receives the notification value. */
                    portMAX_DELAY);   /* Block indefinitely. */

    // time_delta = 1/100,000 or 10 micro seconds
    // to keep the math in integers, 1 time delta will be treated as 1

    /* Σ (80 - I_measured)^2 * delta_t */

    // (2 second curve) t_dela to ms to s
    if (time < max_time) {
      for (int i = 0; i < ADC_CURRENT_SAMPLE_COUNT; ++i) {
        // no need to multiply by 100k here since we only need one time delta
        // and one time delta is 1
        riemann_sum_total += square(current_sensor_readings[i] - 75);
        if (riemann_sum_total > max_sustained) {
          HAL_GPIO_WritePin(HIGH_VOLTAGE_DISCONNECT_GPIO_Port,
                            HIGH_VOLTAGE_DISCONNECT_Pin, GPIO_PIN_SET);
        }
      }
      time += ADC_CURRENT_SAMPLE_COUNT;
    } else {
      time = 0;
      riemann_sum_total = 0;
    }
    // restart watchdog and block for more samples
    ADC1->IER &= ~(1 << 8);
    // next todo: implment state so this can survive for two seconds
  }
  /* USER CODE END StartTask03 */
}

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
    uint8_t task4str[60] = "========== task 4 enterd  =========\r \n";
    HAL_UART_Transmit(&huart1, task4str, sizeof(task4str), 1000);
    char sampleData[128] = "Sample data \r \n";
    char str1[128] = "string1 \r \n";
    char str2[128] = "string2 \r \n";
    char str3[128] = "string3 \r \n";
    char str4[128] = "string4 \r \n";
    char str5[128] = {0};

    /* snprintf(howMany, sizeof(howMany), "there are %lu items in the queue \r
     * \n", */
    /*          uxQueueMessagesWaiting(xLogQueue)); */
    /* HAL_UART_Transmit(&huart1, (uint8_t *)howMany, sizeof(howMany), 1000);
     */
    // ^^ this cuases a hard fault... why??

    // if (uxQueueMessagesWaiting(xLogQueue) == 0) {
    /* xQueueSend(xLogQueue, (void *)&sampleData, portMAX_DELAY); */
    /* xQueueSend(xLogQueue, (void *)&str1, portMAX_DELAY); */
    /* xQueueSend(xLogQueue, (void *)&str2, portMAX_DELAY); */
    /* xQueueSend(xLogQueue, (void *)&str3, portMAX_DELAY); */
    /* xQueueSend(xLogQueue, (void *)&str4, portMAX_DELAY); */
    xQueueSend(xLogQueue, (void *)&str5, portMAX_DELAY);
    //}
    //    xSemaphoreTake(testSemaphore, (TickType_t)100000);
    // osDelay(10000);
    HAL_Delay(1000);
    // vTaskDelete(NULL);
  }
  /* USER CODE END StartTask04 */
}

/* USER CODE BEGIN Header_StartTask05 */
/**
 * @brief Function implementing the watchdog thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask05 */
void StartTask05(void *argument) {
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

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state
   */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
