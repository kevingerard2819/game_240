#include "joystick_buttons.h"
#include "acceleration.h"
#include <stdbool.h>

void joystick_buttons__joystick_and_button_init(void) {
  // Initialize the accelerometer instead of the joystick
  if (acceleration__init()) {
    printf("Accelerometer initialized successfully!\n");
  } else {
    printf("Failed to initialize accelerometer.\n");
  }
}

joystick_position_e joystick_buttons__get_joystick_position(void) {
  joystick_position_e joystick_position = NONE;

  // Read accelerometer data
  acceleration__axis_data_s accel_data = acceleration__get_data();

  // Thresholds for determining movement direction
  const int16_t threshold_left = -200; // Adjust based on sensitivity
  const int16_t threshold_right = 200;

  if (accel_data.x < threshold_left) {
    joystick_position = JOYSTICK_LEFT;
  } else if (accel_data.x > threshold_right) {
    joystick_position = JOYSTICK_RIGHT;
  } else {
    joystick_position = NONE;
  }

  return joystick_position;
}
