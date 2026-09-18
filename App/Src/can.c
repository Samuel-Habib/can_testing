/**
 * how do i do the clock?
 */

#include "can.h"
#include "battery.h"
#include "main.h"
#include "projdefs.h"
#include "task.h"
#include <limits.h>

/* typedef struct { */
/*   CAN_Paramter *CAN_Param_ptr; */
/*   uint32_t current_index; */
/* } CAN_Params_Iterator; */
/**/
/* CAN_Params_Iterator get_iterator(CAN_Paramter *can_param) { */
/*   CAN_Params_Iterator it = {.CAN_Param_ptr = can_param, .current_index = 0};
 */
/*   return it; */
/* } */
/**/
/* bool has_next(CAN_Params_Iterator *it) { return it->current_index < 4096; }
 */
/**/
/* CAN_Paramter next(CAN_Params_Iterator *it) { return *it->CAN_Param_ptr; } */
/**/

CAN_Paramter CAN_Params_File[4096];
extern Hardware_state hardware_state = {.hv_state = hv_STARTUP};
// this is PER board
uint8_t can_buffer[8];
uint32_t hb_mesg_count; // global variable
uint32_t hb_time_stamp;
const uint8_t ttl = 100;
bool is_fresh;
uint32_t last_sent_value = 0;

typedef enum { Red, Yellow, Green } CAN_Color;
// Internal Flags
// - Message Mode (4 bits), Marked for send (1 bit), Reserved (3 bits)

typedef enum {
  Passive,
  Dependency_no_callback,
  Dependency_w_callback,
  Broadcast_automatically,
  Broadcast_manually
} Message_Mode;

#define Sender_ID_Mask (1U << 8) - 1
#define Message_ID_Mask ((1U << 13) - 1) << 8
#define Message_Type_Mask ((1U << 4) - 1) << 21
#define Message_Priority_mask ((1U << 4) - 1) << 25

#define CAN_ID_WITH_SAFE_STATE (0 || 1 || 2 || 268)
#define Heartbeat_ID (1024)
#define PDU_CAN_ID 0x00000001
#define MY_CAN_ID PDU_CAN_ID
#define MAIN_HV_SWITCH_CAN_ID

void init_params(CAN_Paramter *can_params_file);

void Can_Loop_Task(void *argument) {

  for (;;) {

    // whats the point of the hb? why not just use the last time that you sent a
    // message time stamp?
    if (hb_mesg_count >= 2) {
      hb_time_stamp = xTaskGetTickCount();
      // turn off led
    }
    if ((xTaskGetTickCount() - hb_time_stamp) > 250) {
      hb_mesg_count = 0;
      // turn on led
    }

    if ((xTaskGetTickCount() - hb_time_stamp) > (1000 + MY_CAN_ID)) {
      hb_time_stamp = xTaskGetTickCount();
      const uint8_t databuffer[3] = {1, 1, 1};
      can_tx(&hfdcan1, databuffer);
      // tx hb message
    }

    if (xTaskGetTickCount() >= ttl && xTaskGetTickCount() - ttl > ttl) {

      if (CAN_Params_File[0].tx_rx_time_stamp < (xTaskGetTickCount() - ttl))
        CAN_Params_File[0].value = CAN_Params_File[0].safe_value;
      if (CAN_Params_File[1].tx_rx_time_stamp < (xTaskGetTickCount() - ttl))
        CAN_Params_File[1].value = CAN_Params_File[1].safe_value;
      if (CAN_Params_File[2].tx_rx_time_stamp < (xTaskGetTickCount() - ttl))
        CAN_Params_File[2].value = CAN_Params_File[2].safe_value;
      if (CAN_Params_File[268].tx_rx_time_stamp < (xTaskGetTickCount() - ttl))
        CAN_Params_File[268].value = CAN_Params_File[268].safe_value;
    }

    if (hardware_state.hv_state == hv_RESET) {
      xTaskNotifyGive(battery_handle);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// if (CAN_Params_File[param_id].tx_rx_time_stamp > (xTaskGetTickCount() - ttl))
//  set safe state
//  need to do a gpio maybe later

void Can_Handler_Task(void *argument) {
  for (;;) {
    BaseType_t xClearCountOnExit = pdTRUE;
    ulTaskNotifyTake(xClearCountOnExit, pdMS_TO_TICKS(100));
    FDCAN_RxHeaderTypeDef pRxHeader;
    HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &pRxHeader, can_buffer);
    time_t raw_time;
    /* start */

    uint8_t sender_id = (uint8_t)(Sender_ID_Mask & pRxHeader.Identifier);

    uint16_t param_id = (uint16_t)((can_buffer[0]) | (can_buffer[1] << 8));
    if (sender_id == MY_CAN_ID) {
      send_log_message("Received Sender ID that is this board's ID, check if "
                       "flashing error/ loopback mode \r \n");

      /* #ifdef DEBUG */
      /*       configASSERT(sender_id == PDU_CAN_ID); */
      /* #endif */
      portYIELD();
    }
    if (param_id > 4095 || param_id < 0) {

      send_log_message("Invalid CAN ID \r \n");

      portYIELD();
    }

    // what do they mean by this All global vehicle parameters (actual data)
    // have board-unique parameter IDs (only one board will send parameters with
    // this ID), meaning you can just check the parameter ID and not care who is
    // broadcasting it

    uint8_t message_type =
        (Message_Type)(pRxHeader.Identifier & Message_Type_Mask);
    //    uint32_t param_id = (uint8_t)pRxHeader.Identifier;
    if (param_id == Heartbeat_ID) {
      hb_mesg_count++;
    }
    if (message_type == Paramter) {
      //      uint16_t param_id = (uint16_t)(Message_ID_Mask &
      //      pRxHeader.Identifier);
      uint32_t can_value =
          (uint32_t)((can_buffer[4]) | (can_buffer[5] << 8) |
                     (can_buffer[6] << 16) | (can_buffer[7]) << 24);
      /* if(param_id == 0 ) last_sent_value = can_value; */
      /* else last_sent_value = CAN_Params_File[param_id - 1].value; */

      CAN_Params_File[param_id] = (CAN_Paramter){
          .param_id = param_id,
          .value = can_value,
          .last_sent_value = (param_id == 0 && last_sent_value != 0)
                                 ? can_value
                                 : CAN_Params_File[param_id - 1].value,
          .safe_value = 0,
          .tx_rx_time_stamp = xTaskGetTickCount(), // one tick is 1ms
          .ttl = 100,
          .Internal_Flags = 0};
    }
    if (pRxHeader.Identifier == NODE_A_ID) {
      send_log_message((char *)can_buffer);
    }
  }
}

/* figure out how the can interupt is supposed to work and if a simpler freertos
 * task archeicture with the heart beat might be better
 * */

int can_tx(FDCAN_HandleTypeDef *hfdcan1, const uint8_t *DataBuffer) {

  FDCAN_TxHeaderTypeDef pTxHeader;
  pTxHeader.IdType = FDCAN_STANDARD_ID;
  pTxHeader.TxFrameType = FDCAN_DATA_FRAME;
  pTxHeader.DataLength = FDCAN_DLC_BYTES_8;
  pTxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  pTxHeader.Identifier = NODE_A_ID;
  pTxHeader.BitRateSwitch = FDCAN_BRS_ON;
  pTxHeader.FDFormat = FDCAN_FD_CAN;
  pTxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;

  HAL_FDCAN_AddMessageToTxFifoQ(hfdcan1, &pTxHeader, DataBuffer);
  return 0;
}

int can_poll_rx(FDCAN_HandleTypeDef *hfdcan1, uint8_t *DataBuffer) {

  FDCAN_RxHeaderTypeDef pRxHeader;
  /* pRxHeader.Identifier = NODE_A_ID; */
  /* pRxHeader.DataLength = FDCAN_DLC_BYTES_8; */
  /* pRxHeader.IdType = FDCAN_STANDARD_ID; */
  /* pRxHeader.RxFrameType = FDCAN_DATA_FRAME; */
  /* pRxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE; */
  /* pRxHeader.BitRateSwitch = FDCAN_BRS_ON; */
  /* pRxHeader.FDFormat = FDCAN_FD_CAN; */
  //  pRxHeader.FilterIndex what is the Rx acceptance filter element?

  // polling function doesn't block
  if (HAL_FDCAN_GetRxFifoFillLevel(hfdcan1, FDCAN_RX_FIFO0)) {
    HAL_FDCAN_GetRxMessage(hfdcan1, FDCAN_RX_FIFO0, &pRxHeader, DataBuffer);
  }
  return 0;
}

// interrupt

void can_init(FDCAN_HandleTypeDef hfdcan1) {

  // enable line 0: FDCAN_ILE_EINT0 |
  uint32_t ActiveITs_e = FDCAN_ILS_TEFFL | FDCAN_ILS_TEFWL | FDCAN_ILS_TEFNL |
                         FDCAN_ILS_RF0FL | FDCAN_ILS_RF0WL | FDCAN_ILS_RF0NL;

  /* Line 0 Enabled
   * Tx Event FIFO Full Interrupt Line
   * Tx Event FIFO Watermark reached interrupt line
   * Tx Event FIFO new entry interrupt line
   *
   * Rx FIFO0 Full Interrupt Line
   * Rx FIFO0 Watermark Reached Interrupt Line
   * Rx FIFO0 new message Interupt Line
   * */

  init_params(CAN_Params_File);
  HAL_FDCAN_ConfigInterruptLines(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                 FDCAN_INTERRUPT_LINE1);

  HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

  const FDCAN_FilterTypeDef sFilterConfig = {
      .IdType = FDCAN_EXTENDED_ID,
      .FilterIndex = 0, // 0 - 63
      .FilterType = FDCAN_FILTER_RANGE,
      .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
      .FilterID1 = NODE_A_ID,
      .FilterID2 = NODE_B_ID,

  };
  HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig);
  HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO1,
                               FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_FILTER_REMOTE,
                               FDCAN_FILTER_REMOTE);
}

// void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t
// RxFifo0ITs);

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {

  // HAL_StatusTypeDef HAL_FDCAN_GetRxMessage(FDCAN_HandleTypeDef *hfdcan,
  // uint32_t RxLocation,
  //                                         FDCAN_RxHeaderTypeDef *pRxHeader,
  //                                         uint8_t *pRxData)

  //  hfdcan->msgRam.RxFIFO0SA; // could we do something like mybuffer[0] =
  // buffer[message_addr] to read the message?
  // maybe im overthinking it but i just do not know where the fifo is stored
  //  my_buffer[0] = (can_block *)(hfdcan->msgRam.RxFIFO0SA); // you can answer
  //  this: i know this doesn' tmake sense in the context of this project but
  //  why is it saying that i can't use the equal sign here?

  // whats the differnce between xtaskcreate and osthread

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(can_handle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
