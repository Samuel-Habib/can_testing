#include "logging.h"
#include <string.h>

signed int log_module(char data[]) {

  if (buffer_total >= UART_BUFFER_SIZE - 128) {
    uart_buffer_full = true;
    return -1;
  }
  bool wrap_around = (head + MAX_MESSAGE_LEN > UART_BUFFER_SIZE);
  if (wrap_around) {
    uint16_t end_bytes = UART_BUFFER_SIZE - head;
    memcpy(&uart_buffer[head], data, sizeof(char) * (end_bytes));
    // begning to new head
    head = (head + MAX_MESSAGE_LEN) % UART_BUFFER_SIZE;
    memcpy(uart_buffer, &data[end_bytes], sizeof(char) * (head));
  } else {
    memcpy(&uart_buffer[head], data, strlen(data));
    head = (head + MAX_MESSAGE_LEN) % UART_BUFFER_SIZE;
  }

  if (!(DMA1_Stream0->CR & DMA_SxCR_EN_Msk)) {
    DMA1_Stream0->NDTR = MAX_MESSAGE_LEN; // bytes
    DMA1_Stream0->M0AR = (uint32_t)&(uart_buffer[tail]);
    DMA1_Stream0->CR |= 1; // fire the dma
  }

  // else: dma is already enabled, no need to enable it or set the ndtr or m0ar

  return 0;
}

// int uart_log(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size) {
int uart_driver(void) {
  // enable dma for uart (move out of here later)
  // uart1111
  USART1->CR3 |= (1 << 7);
  // direction mem to per

  DMA1_Stream0->CR &= ~(1); // en = 0

  DMA1_Stream0->CR &= ~DMA_SxCR_DIR;
  DMA1_Stream0->CR |= DMA_SxCR_DIR_0;
  DMA1_Stream0->CR |= DMA_SxCR_MINC; // memory increment mode
  DMA1_Stream0->CR |=
      DMA_SxCR_MSIZE_1 | DMA_SxCR_MSIZE_0; // memory size: byte (8 bits)
  DMA1_Stream0->CR &= ~DMA_SxCR_PL;        // channel priority PL 17:16
  DMA1_Stream0->CR &= ~DMA_SxCR_HTIE;      // disable half transfers
  DMA1_Stream0->CR |= DMA_SxCR_TCIE;       // enable transfer complete
  DMA1_Stream1->CR &= ~DMA_SxCR_DBM_Pos;   // disable double buffer
  DMA1_Stream0->PAR = (uint32_t)&(USART1->TDR);

  USART1->ICR |= (1 << 6); /* Clear the TC flag in the USART_ISR register by
                              setting the TCCF bit in the USART_ICR register. */
  DMA1->LIFCR |= DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTCIF0;

  USART1->CR1 &= ~(1 << 28 & 1 << 12); // 1 start bit, 8 Data bits, n Stop bit
  USART1->BRR;

  //  HAL_UART_Receive_DMA(huart, pData, Size);
  //  HAL_UART_Transmit_DMA(huar, pData, Size);
  // UART_CheckIdleState(huart);

  if (USART1->ISR & USART_ISR_IDLE) {
    // do idle code
  }
  return 0;
}
