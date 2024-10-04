#include "gpio_lab_4.h"
#include "lpc40xx.h"
#include <stdio.h>

static void gpio1__modify_pin(uint8_t pin_num, bool set_high) {
  if (set_high) {
    LPC_GPIO1->SET = (1U << pin_num);
  } else {
    LPC_GPIO1->CLR = (1U << pin_num);
  }
}

void gpio1__set_as_input(uint8_t pin_num) {
  LPC_GPIO1->DIR &= ~(1U << pin_num);             // Clear bit to set as input
  printf("GPIO1 pin %d set as input\n", pin_num); // For debugging
}

void gpio1__set_as_output(uint8_t pin_num) {
  LPC_GPIO1->DIR |= (1U << pin_num); // Setting bit to set as output
  printf("GPIO1 pin %d set as output\n", pin_num);
}

void gpio1__set_high(uint8_t pin_num) {
  gpio1__modify_pin(pin_num, true);
  printf("GPIO1 pin %d set HIGH\n", pin_num);
}

// Should alter the hardware registers to set the pin as low
void gpio1__set_low(uint8_t pin_num) {
  gpio1__modify_pin(pin_num, false);
  printf("GPIO1 pin %d set LOW\n", pin_num);
}

// Should set or clear the pin depending on the boolean value
void gpio1__set(uint8_t pin_num, bool high) {
  gpio1__modify_pin(pin_num, high);
  printf("GPIO1 pin %d set to %s\n", pin_num, high ? "HIGH" : "LOW");
}

bool gpio1__get_level(uint8_t pin_num) {
  bool pin_state = (LPC_GPIO1->PIN & (1U << pin_num)) != 0;
  printf("GPIO1 pin %d level is %s\n", pin_num, pin_state ? "HIGH" : "LOW");
  return pin_state;
}
