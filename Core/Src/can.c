/**
 * how do i do the clock?
 */

#include "can.h"

int can_tx(FDCAN_HandleTypeDef hfdcan1) {
  // start
  const FDCAN_TxHeaderTypeDef pTxHeader;
  const uint8_t Data;
  uint32_t BufferIndex;
  // add message
  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &pTxHeader, &Data);
  // HAL_FDCAN_AddMessageToTxBuffer(&hfdcan1, &pTxHeader, &Data, BufferIndex);

  //[Q<<] is enabletxbufferrequest nesseary?
  HAL_FDCAN_EnableTxBufferRequest(&hfdcan1, BufferIndex);

  // get location
  uint32_t location = HAL_FDCAN_GetLatestTxFifoQRequestBuffer(&hfdcan1);

  return location;
}

// is there a way to do this without needing the locatin?
// do i need to stop can?
int can_rx(FDCAN_HandleTypeDef hfdcan1, uint32_t rx_location) {
  // start
  FDCAN_RxHeaderTypeDef pRxHeader;
  uint8_t Data;
  uint32_t BufferIndex;
  // add message
  //  HAL_FDCAN_GetRxMessage(, rx_location, &pRxHeader, &Data);
  // HAL_FDCAN_AddMessageToTxBuffer(&hfdcan1, &pTxHeader, &Data, BufferIndex);

  //[Q<<] is enabletxbufferrequest nesseary?
  HAL_FDCAN_EnableTxBufferRequest(&hfdcan1, BufferIndex);
}

// polling

int can_poll(FDCAN_HandleTypeDef hfdcan1, uint8_t DataBuffer) {
  // how do i set these?
  FDCAN_TxHeaderTypeDef pTxHeader;
  pTxHeader.IdType = FDCAN_STANDARD_ID;
  pTxHeader.TxFrameType =
      FDCAN_DATA_FRAME; // what is this? why does it go up to 64B?
  pTxHeader.DataLength = FDCAN_DLC_BYTES_8;
  pTxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  pTxHeader.Identifier = NODE_A_ID;
  pTxHeader.BitRateSwitch = FDCAN_BRS_ON;
  pTxHeader.FDFormat = FDCAN_FD_CAN;
  pTxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;

  FDCAN_RxHeaderTypeDef pRxHeader;
  pRxHeader.Identifier = NODE_A_ID;
  pRxHeader.DataLength = FDCAN_DLC_BYTES_8;
  pRxHeader.IdType = FDCAN_STANDARD_ID;
  pRxHeader.RxFrameType = FDCAN_DATA_FRAME;
  pRxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  pRxHeader.BitRateSwitch = FDCAN_BRS_ON;
  pRxHeader.FDFormat = FDCAN_FD_CAN;
  //  pRxHeader.FilterIndex what is the Rx acceptance filter element?

  uint8_t dataBuffer;
  uint32_t BufferIndex;

  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &pTxHeader, &DataBuffer);

  if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0)) {
    HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &pRxHeader, &DataBuffer);
  }

  // if (HAL_FDCAN_IsRxBufferMessageAvailable(&hfdcan1, BufferIndex)) {
  //   // so i can check if a message is available but how do i actually read
  //   that
  //   // message without knowing it's location?
  // }
}

// interrupt

int can_int(FDCAN_HandleTypeDef hfdcan1) {
  // what is the ITList? is it like the ITs bit shifted or anded together???
  HAL_FDCAN_ConfigInterruptLines(&hfdcan1, 0, 0);
  HAL_FDCAN_RxFifo0Callback(&hfdcan1, 1);
}

int heartbeat() {}
