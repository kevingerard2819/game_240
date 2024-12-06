#pragma once

#include <stdint.h>

typedef enum {
  BLACK = 0,
  RED = 1,
  GREEN = 2,
  YELLOW = 3,
  BLUE = 4,
  PURPLE = 5,
  CYAN = 6,
  WHITE = 7,
} color_code_s;

uint8_t led_driver__get_led_matix_value(uint8_t row, uint8_t col);
void led_driver__disable_latch(void);

void led_driver__enable_latch(void);

void led_driver__disable_output(void);

void led_driver__enable_output(void);

void led_driver__clock_toggle(void);

void led_driver__init_gpio_pins_for_matrix(void);

void led_driver__map_color_code_to_color_select_pins_top(color_code_s color);

void led_driver__map_color_code_to_color_select_pins_bottom(color_code_s color);

void led_driver__set_pixel(uint8_t row, uint8_t col, color_code_s color);

void led_driver__set_row(uint8_t row, color_code_s color);

;
void led_driver__update_display(void);

void led_driver__set_display_matrix(uint8_t matrix[64][64]);

/*#pragma once

#include <stdint.h>

typedef enum {
  BLACK = 0,
  RED = 1,
  GREEN = 2,
  YELLOW = 3,
  BLUE = 4,
  PURPLE = 5,
  CYAN = 6,
  WHITE = 7,
} color_code_s;

uint8_t led_driver__get_led_matix_value(uint8_t row, uint8_t col);

void led_driver__disable_latch(void);

void led_driver__enable_latch(void);

void led_driver__disable_output(void);

void led_driver__enable_output(void);

void led_driver__clock_toggle(void);

void led_driver__init_gpio_pins_for_matrix(void);

void led_driver__map_color_code_to_color_select_pins_top(color_code_s color);

void led_driver__map_color_code_to_color_select_pins_bottom(color_code_s color);

void led_driver__set_pixel(uint8_t row, uint8_t col, color_code_s color);

void led_driver__set_row(uint8_t row, color_code_s color);

void led_driver__update_display(void);

void led_driver__set_display_matrix(uint8_t matrix[64][64]);
*/