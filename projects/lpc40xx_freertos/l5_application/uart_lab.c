#include <uart_lab.h>

#include "FreeRTOS.h"
#include "lpc_peripherals.h"
#include "queue.h"
#include <lpc40xx.h>
#include <stdbool.h>
#include <stdint.h>

static QueueHandle_t your_uart_rx_queue;

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  int uart2_power_bit = 24;
  int dlab_mask_bit = 7;
  LPC_SC->PCONP |= (1 << uart2_power_bit);

  LPC_UART2->LCR |= (1 << dlab_mask_bit); // Setting DLAB

  uint16_t divider = peripheral_clock / (baud_rate * 16);

  LPC_UART2->DLL |= (divider & 0xFF); // Setting LSB

  LPC_UART2->DLM |= (divider >> 8 & 0xFF); // Setting MSB

  LPC_UART2->FDR &= ~(0b1111); // Ensuring DivAddVal 0

  LPC_UART2->FDR |= (1 << 4); // Making MulVal non-zero

  LPC_UART2->LCR |= (0b11); // 8-bit character length

  LPC_UART2->LCR &= ~(1 << dlab_mask_bit); // Resetting DLAB
};

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  if ((LPC_UART2->LSR & (1 << 0))) {
    *input_byte = LPC_UART2->RBR & (0xFF);
    return true;
  }

  return false;
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  if (LPC_UART2->LSR & (1 << 5)) {
    LPC_UART2->THR = output_byte;
    return true;
  }

  return false;
}

// Private function of our uart_lab.c
static void uart2_receive_interrupt(void) {
  // printf("A");

  // TODO: Read the IIR register to figure out why you got interrupted
  if (!(LPC_UART2->IIR & (1 << 0))) {
    int id = (LPC_UART2->IIR >> 1) & (0b111);

    if (id == 2) {
      if (LPC_UART2->LSR & (1 << 0)) {
        char data = LPC_UART2->RBR & (0xFF);
        xQueueSendFromISR(your_uart_rx_queue, &data, NULL);
      }
    }
  };
  // TODO: Based on IIR status, read the LSR register to confirm if there is data to be read

  // TODO: Based on LSR status, read the RBR register and input the data to the RX Queue
}

// Public function to enable UART interrupt
// TODO Declare this at the header file
void uart__enable_receive_interrupt(uart_number_e uart_number) {
  // TODO: Use lpc_peripherals.h to attach your interrupt
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, uart2_receive_interrupt, "Interrupt");

  int RDA_interrupt_mask = 1 << 0;
  // TODO: Enable UART receive interrupt by reading the LPC User manual
  // Hint: Read about the IER register
  LPC_UART2->IER |= (RDA_interrupt_mask);
  printf("Helloooooooooooo!!!!!!");
  // TODO: Create your RX queue
  your_uart_rx_queue = xQueueCreate(8, sizeof(char));
}

// Public function to get a char from the queue (this function should work without modification)
// TODO: Declare this at the header file
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}