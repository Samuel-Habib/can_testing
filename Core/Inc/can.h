#include "main.h"
#include <stdint.h>

#define NODE_A_ID 0x00000001
#define NODE_B_ID 0x00000002

int can_tx(FDCAN_HandleTypeDef hfdcan1);
int can_rx(FDCAN_HandleTypeDef hfdcan1, uint32_t rx_location);
uint8_t *can_poll(FDCAN_HandleTypeDef hfdcan1, uint8_t DataBuffer,
                  uint8_t *extra);
int can_int(FDCAN_HandleTypeDef hfdcan1);
int heartbeat();
