#include "button.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdio.h>

// Define the GPIO pin for the button
static const uint8_t button_pin = 10; // P0.10

// Semaphore handle for button press signaling
SemaphoreHandle_t button_semaphore = NULL;

// Function to configure the button pin as input and enable pull-down resistor
static void configure_button_pin(void) {
  LPC_GPIO0->DIR &= ~(1 << button_pin); // Set pin as input
  LPC_IOCON->P0_10 &= ~0x07;            // Clear function bits
  LPC_IOCON->P0_10 |= (1 << 3);         // Enable pull-down resistor
}

// Button interrupt handler
void GPIO_IRQHandler(void) {
  if (LPC_GPIOINT->IO0IntStatF & (1 << button_pin)) {
    xSemaphoreGiveFromISR(button_semaphore, NULL);
    LPC_GPIOINT->IO0IntClr |= (1 << button_pin); // Clear interrupt flag
    fprintf(stderr, "Button press detected\n");
  }
}

// Initialize the button hardware and interrupt
void button_init(void) {
  configure_button_pin();

  // Initialize semaphore
  button_semaphore = xSemaphoreCreateBinary();

  // Setup interrupt for button press
  LPC_GPIOINT->IO0IntEnF |= (1 << button_pin); // Enable falling edge interrupt on P0.10
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, GPIO_IRQHandler, "GPIO ISR");
}
