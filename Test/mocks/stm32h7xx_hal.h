#ifndef MOCK_STM32H7XX_HAL_H
#define MOCK_STM32H7XX_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* HAL Status */
typedef enum {
    HAL_OK       = 0x00U,
    HAL_ERROR    = 0x01U,
    HAL_BUSY     = 0x02U,
    HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

/* GPIO Definitions */
typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET = 1
} GPIO_PinState;

#define GPIO_PIN_0   ((uint16_t)0x0001)
#define GPIO_PIN_1   ((uint16_t)0x0002)
#define GPIO_PIN_2   ((uint16_t)0x0004)
#define GPIO_PIN_3   ((uint16_t)0x0008)
#define GPIO_PIN_4   ((uint16_t)0x0010)
#define GPIO_PIN_5   ((uint16_t)0x0020)
#define GPIO_PIN_6   ((uint16_t)0x0040)
#define GPIO_PIN_7   ((uint16_t)0x0080)
#define GPIO_PIN_8   ((uint16_t)0x0100)
#define GPIO_PIN_9   ((uint16_t)0x0200)
#define GPIO_PIN_10  ((uint16_t)0x0400)
#define GPIO_PIN_11  ((uint16_t)0x0800)
#define GPIO_PIN_12  ((uint16_t)0x1000)
#define GPIO_PIN_13  ((uint16_t)0x2000)
#define GPIO_PIN_14  ((uint16_t)0x4000)
#define GPIO_PIN_15  ((uint16_t)0x8000)

typedef struct {
    uint32_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;
} GPIO_InitTypeDef;

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

/* UART / USART Definitions */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
    volatile uint32_t PRESC;
} USART_TypeDef;

#define USART_ISR_IDLE (1U << 4)

typedef struct {
    USART_TypeDef *Instance;
    uint32_t gState;
    uint32_t ErrorCode;
} UART_HandleTypeDef;

/* IWDG Definitions */
typedef struct {
    volatile uint32_t KR;
    volatile uint32_t PR;
    volatile uint32_t RLR;
    volatile uint32_t SR;
    volatile uint32_t WINR;
} IWDG_TypeDef;

typedef struct {
    IWDG_TypeDef *Instance;
} IWDG_HandleTypeDef;

/* TIM Definitions */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
} TIM_TypeDef;

typedef struct {
    TIM_TypeDef *Instance;
} TIM_HandleTypeDef;

/* ADC Definitions */
typedef struct {
    volatile uint32_t ISR;
    volatile uint32_t IER;
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CFGR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t PCSEL;
    volatile uint32_t LTR1;
    volatile uint32_t HTR1;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t SQR4;
    volatile uint32_t DR;
} ADC_TypeDef;

typedef struct {
    ADC_TypeDef *Instance;
} ADC_HandleTypeDef;

/* DMA Definitions */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t NDTR;
    volatile uint32_t PAR;
    volatile uint32_t M0AR;
    volatile uint32_t M1AR;
    volatile uint32_t FCR;
} DMA_Stream_TypeDef;

typedef struct {
    volatile uint32_t ISR[2];
    volatile uint32_t IFCR[2];
    volatile uint32_t LIFCR;
    volatile uint32_t HIFCR;
} DMA_TypeDef;

#define DMA_SxCR_EN_Msk       (1U << 0)
#define DMA_SxCR_DIR          (3U << 6)
#define DMA_SxCR_DIR_0        (1U << 6)
#define DMA_SxCR_MINC         (1U << 9)
#define DMA_SxCR_MSIZE_0      (1U << 13)
#define DMA_SxCR_MSIZE_1      (1U << 14)
#define DMA_SxCR_PL           (3U << 16)
#define DMA_SxCR_HTIE         (1U << 3)
#define DMA_SxCR_TCIE         (1U << 4)
#define DMA_SxCR_DBM_Pos      (18U)

#define DMA_LIFCR_CHTIF1      (1U << 10)
#define DMA_LIFCR_CTCIF0      (1U << 5)

/* FDCAN Definitions */
typedef struct {
    volatile uint32_t CREL;
    volatile uint32_t ENDN;
    volatile uint32_t DBTP;
    volatile uint32_t TEST;
    volatile uint32_t RCON;
    volatile uint32_t CCCR;
} FDCAN_GlobalTypeDef;

typedef struct {
    FDCAN_GlobalTypeDef *Instance;
} FDCAN_HandleTypeDef;

#define FDCAN_STANDARD_ID     (0x00000000U)
#define FDCAN_EXTENDED_ID     (0x00000001U)
#define FDCAN_DATA_FRAME      (0x00000000U)
#define FDCAN_REMOTE_FRAME    (0x00000001U)
#define FDCAN_DLC_BYTES_0     (0x00000000U)
#define FDCAN_DLC_BYTES_1     (0x00000001U)
#define FDCAN_DLC_BYTES_2     (0x00000002U)
#define FDCAN_DLC_BYTES_3     (0x00000003U)
#define FDCAN_DLC_BYTES_4     (0x00000004U)
#define FDCAN_DLC_BYTES_5     (0x00000005U)
#define FDCAN_DLC_BYTES_6     (0x00000006U)
#define FDCAN_DLC_BYTES_7     (0x00000007U)
#define FDCAN_DLC_BYTES_8     (0x00000008U)
#define FDCAN_ESI_ACTIVE      (0x00000000U)
#define FDCAN_ESI_PASSIVE     (0x80000000U)
#define FDCAN_BRS_OFF         (0x00000000U)
#define FDCAN_BRS_ON          (0x00100000U)
#define FDCAN_CLASSIC_CAN     (0x00000000U)
#define FDCAN_FD_CAN          (0x00200000U)
#define FDCAN_NO_TX_EVENTS    (0x00000000U)
#define FDCAN_STORE_TX_EVENTS (0x00800000U)
#define FDCAN_RX_FIFO0        (0x00000000U)
#define FDCAN_RX_FIFO1        (0x00000001U)

typedef struct {
    uint32_t Identifier;
    uint32_t IdType;
    uint32_t TxFrameType;
    uint32_t DataLength;
    uint32_t ErrorStateIndicator;
    uint32_t BitRateSwitch;
    uint32_t FDFormat;
    uint32_t TxEventFifoControl;
    uint32_t MessageMarker;
} FDCAN_TxHeaderTypeDef;

typedef struct {
    uint32_t Identifier;
    uint32_t IdType;
    uint32_t RxFrameType;
    uint32_t DataLength;
    uint32_t ErrorStateIndicator;
    uint32_t BitRateSwitch;
    uint32_t FDFormat;
    uint32_t RxTimestamp;
    uint32_t FilterIndex;
    uint32_t IsFilterMatchingFrame;
} FDCAN_RxHeaderTypeDef;

/* Peripheral instance mock pointers */
extern GPIO_TypeDef mock_GPIOG;
extern USART_TypeDef mock_USART1;
extern ADC_TypeDef mock_ADC1;
extern DMA_Stream_TypeDef mock_DMA1_Stream0;
extern DMA_Stream_TypeDef mock_DMA1_Stream1;
extern DMA_TypeDef mock_DMA1;
extern TIM_TypeDef mock_TIM1;
extern TIM_TypeDef mock_TIM2;
extern FDCAN_GlobalTypeDef mock_FDCAN1;
extern IWDG_TypeDef mock_IWDG1;

#define GPIOG         (&mock_GPIOG)
#define USART1        (&mock_USART1)
#define ADC1          (&mock_ADC1)
#define DMA1_Stream0  (&mock_DMA1_Stream0)
#define DMA1_Stream1  (&mock_DMA1_Stream1)
#define DMA1          (&mock_DMA1)
#define TIM1          (&mock_TIM1)
#define TIM2          (&mock_TIM2)
#define FDCAN1        (&mock_FDCAN1)
#define IWDG1         (&mock_IWDG1)

/* HAL Function Prototypes */
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, const GPIO_InitTypeDef *GPIO_Init);

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);

HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg);

HAL_StatusTypeDef HAL_FDCAN_AddMessageToTxFifoQ(FDCAN_HandleTypeDef *hfdcan, const FDCAN_TxHeaderTypeDef *pTxHeader, const uint8_t *pTxData);
uint32_t HAL_FDCAN_GetRxFifoFillLevel(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo);
HAL_StatusTypeDef HAL_FDCAN_GetRxMessage(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo, FDCAN_RxHeaderTypeDef *pRxHeader, uint8_t *pRxData);
HAL_StatusTypeDef HAL_FDCAN_ConfigInterruptLines(FDCAN_HandleTypeDef *hfdcan, uint32_t ITList, uint32_t InterruptLine);
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);

void HAL_Delay(uint32_t Delay);
uint32_t HAL_GetTick(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_STM32H7XX_HAL_H */
