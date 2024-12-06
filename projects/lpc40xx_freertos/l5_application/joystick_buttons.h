#pragma once

typedef enum {
  JOYSTICK_LEFT = 0,
  JOYSTICK_RIGHT,
  NONE,
} joystick_position_e;

void joystick_buttons__joystick_and_button_init(void);
joystick_position_e joystick_buttons__get_joystick_position(void);

