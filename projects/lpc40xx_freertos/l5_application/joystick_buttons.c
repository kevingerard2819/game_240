#include "joystick_buttons.h"
#include "gpio.h"

#include <stdbool.h>

static gpio_s joystick_left;
static gpio_s joystick_right;

void joystick_buttons__joystick_and_button_init(void) {
  /* P0_15 - Joystick Left, P0_18 - Joystick Right */
  joystick_left = gpio__construct_as_input(GPIO__PORT_0, 15);
  joystick_right = gpio__construct_as_input(GPIO__PORT_0, 18);
  gpio__enable_pull_down_resistors(joystick_left);
  gpio__enable_pull_down_resistors(joystick_right);
}

joystick_position_e joystick_buttons__get_joystick_position(void) {
  joystick_position_e joystick_position = NONE;
  bool joystick_left_value = gpio__get(joystick_left);
  bool joystick_right_value = gpio__get(joystick_right);
  if (joystick_left_value) {
    joystick_position = JOYSTICK_LEFT;
  } else if (joystick_right_value) {
    joystick_position = JOYSTICK_RIGHT;
  } else {
    joystick_position = NONE;
  }
  return joystick_position;
}
