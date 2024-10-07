#include "gpio_intr.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "stdint.h"
#include <stdio.h>

// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpio0_callbacks[32];

void gpio0__attach_interrupt(uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {

  // 1) Store the callback based on the pin at gpio0_callbacks
  // 2) Configure GPIO 0 pin for rising or falling edge
  gpio0_callbacks[pin] = callback;
  uint32_t mask = (1 << pin);
  if (interrupt_type == GPIO_INTR__RISING_EDGE) {
    LPC_GPIOINT->IO0IntEnR |= mask;
  } else {
    LPC_GPIOINT->IO0IntEnF |= mask;
    // printf("inside falling");
    // printf("%d statusFalling",LPC_GPIOINT->IO0IntStatF);
  }
}
// We wrote some of the implementation for you
void gpio0__interrupt_dispatcher(void) {

  uint32_t interrupt_status = LPC_GPIOINT->IO0IntStatF | LPC_GPIOINT->IO0IntStatR;
  //   printf("%d", LPC_GPIOINT->IO0IntStatF);
  uint8_t pin_num = 0;
  while (pin_num < 32) // i: pin that generated interrupt:Run through all pins to check interrupt
  {
    if (interrupt_status & (1 << pin_num)) {

      function_pointer_t attached_user_handler =
          gpio0_callbacks[pin_num]; // Function pointer to the pin that generated interrupt
      attached_user_handler();      // Call api using pointer
      clear_pin_interrupt(pin_num); // clear interrupt
    }
  }
  // const int pin_that_generated_interrupt = logic_that_you_will_write();
  // function_pointer_t attached_user_handler = gpio0_callbacks[i];
  // Invoke the user registered callback, and then clear the interrupt
}
void clear_pin_interrupt(uint32_t pin) { LPC_GPIOINT->IO0IntClr |= (1 << pin); }
