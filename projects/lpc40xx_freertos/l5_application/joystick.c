#include "joystick.h"
#include "gpio.h"
#include <stdbool.h>

// GPIO pin definitions for joystick
static gpio_s joystick_left;
static gpio_s joystick_right;

void joystick_buttons__joystick_and_button_init(void) {
  /* Initialize GPIOs for joystick:
   * P0_15 -> Joystick Left
   * P0_18 -> Joystick Right
   */
  joystick_left = gpio__construct_as_input(GPIO__PORT_0, 15);
  joystick_right = gpio__construct_as_input(GPIO__PORT_0, 18);

  // Enable pull-down resistors to ensure no floating values
  gpio__enable_pull_down_resistors(joystick_left);
  gpio__enable_pull_down_resistors(joystick_right);
}

joystick_position_e joystick_buttons__get_joystick_position(void) {
  // Read the current state of the joystick GPIOs
  bool joystick_left_value = gpio__get(joystick_left);
  bool joystick_right_value = gpio__get(joystick_right);

  // Determine the joystick position based on the GPIO values
  if (joystick_left_value) {
    return JOYSTICK_LEFT;
  } else if (joystick_right_value) {
    return JOYSTICK_RIGHT;
  } else {
    return NONE;
  }
}
