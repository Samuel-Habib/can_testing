#include "logging.h"
#include "main.h"
#include "stm32h7xx_hal_fdcan.h"
#include <stdint.h>
#include <time.h>

#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "stm32h723xx.h"
#include "stm32h7xx_it.h"

#define NODE_A_ID 0x00000001
#define NODE_B_ID 0x00000002

extern const osThreadAttr_t can_attributes;
void Can_Handler_Task(void *argument);
int can_tx(FDCAN_HandleTypeDef *hfdcan1, const uint8_t *DataBuffer);
int can_poll_rx(FDCAN_HandleTypeDef *hfdcan1, uint8_t *DataBuffer);

// --

// int can_rx(FDCAN_HandleTypeDef *hfdcan1, uint32_t rx_location);
// uint8_t *can_poll(FDCAN_HandleTypeDef *hfdcan1, uint8_t DataBuffer, uint8_t
// *extra); int can_int(FDCAN_HandleTypeDef hfdcan1);

typedef struct {
  uint16_t param_id;
  uint32_t value;
  uint32_t last_sent_value;
  uint32_t safe_value;
  uint32_t tx_rx_time_stamp;
  uint8_t ttl;
  uint8_t Internal_Flags;

} CAN_Paramter;

typedef enum {
  Paramter,
  Trigger,
  Diagnostics,
  Console_Text,
  Heartbeat
} Message_Type;

int heartbeat();

extern uint8_t can_buffer[8];
