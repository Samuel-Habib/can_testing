/**
 * @file test_example.c
 * @brief Example unit test demonstrating how to use Fake Function Framework (fff)
 *        and Unity for host-based C unit testing with GCC.
 */

#include "unity.h"
#include "fff.h"
#include "fake_stm32_hal.h"

/* Application headers to test */
#include "can.h"
#include "logging.h"
#include "watchdog.h"

/* Optional: Define test-specific fakes or custom fake callbacks here */
FAKE_VALUE_FUNC1(int, custom_callback_example, int);

void setUp(void) {
    /* Always reset all global HAL and RTOS fakes before each test */
    fake_stm32_hal_reset_all();
    RESET_FAKE(custom_callback_example);
}

void tearDown(void) {
    /* Cleanup after each test if needed */
}

/* =========================================================================
 * Example Test 1: Testing CAN Transmission with FFF
 * ========================================================================= */
void test_can_tx_should_populate_header_and_call_hal_tx(void) {
    uint8_t payload[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

    /* Configure fake return value */
    HAL_FDCAN_AddMessageToTxFifoQ_fake.return_val = HAL_OK;

    int result = can_tx(&hfdcan1, payload);

    /* Assert function return value */
    TEST_ASSERT_EQUAL_INT(0, result);

    /* Verify HAL function was called exactly once */
    TEST_ASSERT_EQUAL_INT(1, HAL_FDCAN_AddMessageToTxFifoQ_fake.call_count);

    /* Verify arguments passed to the fake */
    TEST_ASSERT_EQUAL_PTR(&hfdcan1, HAL_FDCAN_AddMessageToTxFifoQ_fake.arg0_val);
    TEST_ASSERT_NOT_NULL(HAL_FDCAN_AddMessageToTxFifoQ_fake.arg1_val);
    TEST_ASSERT_EQUAL_HEX32(NODE_A_ID, HAL_FDCAN_AddMessageToTxFifoQ_fake.arg1_val->Identifier);
    TEST_ASSERT_EQUAL_HEX32(FDCAN_DLC_BYTES_8, HAL_FDCAN_AddMessageToTxFifoQ_fake.arg1_val->DataLength);
    TEST_ASSERT_EQUAL_PTR(payload, HAL_FDCAN_AddMessageToTxFifoQ_fake.arg2_val);
}

/* =========================================================================
 * Example Test 2: Testing CAN Poll RX when FIFO is empty vs when FIFO has data
 * ========================================================================= */
void test_can_poll_rx_should_not_read_when_fifo_empty(void) {
    uint8_t rx_buffer[8] = {0};

    /* Simulate FIFO fill level = 0 */
    HAL_FDCAN_GetRxFifoFillLevel_fake.return_val = 0;

    int result = can_poll_rx(&hfdcan1, rx_buffer);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, HAL_FDCAN_GetRxFifoFillLevel_fake.call_count);
    TEST_ASSERT_EQUAL_INT(0, HAL_FDCAN_GetRxMessage_fake.call_count);
}

/* =========================================================================
 * Example Test 3: Testing Logging Buffer using FFF and Register Mocks
 * ========================================================================= */
void test_log_module_should_store_data_and_trigger_dma(void) {
    char test_msg[] = "Hello Host Unit Test!";

    /* Verify initial state */
    TEST_ASSERT_EQUAL_UINT16(0, head);
    TEST_ASSERT_EQUAL_HEX32(0, DMA1_Stream0->CR);

    signed int ret = log_module(test_msg);

    TEST_ASSERT_EQUAL_INT(0, ret);
    /* Verify data was written to buffer */
    TEST_ASSERT_EQUAL_STRING_LEN(test_msg, (char*)uart_buffer, strlen(test_msg));
    /* Verify DMA stream was enabled (CR bit 0 set) */
    TEST_ASSERT_TRUE(DMA1_Stream0->CR & DMA_SxCR_EN_Msk);
    /* Verify DMA memory address points to uart_buffer */
    TEST_ASSERT_EQUAL_PTR(&uart_buffer[0], (void*)DMA1_Stream0->M0AR);
}

/* =========================================================================
 * Main Test Runner
 * ========================================================================= */
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_can_tx_should_populate_header_and_call_hal_tx);
    RUN_TEST(test_can_poll_rx_should_not_read_when_fifo_empty);
    RUN_TEST(test_log_module_should_store_data_and_trigger_dma);

    return UNITY_END();
}
