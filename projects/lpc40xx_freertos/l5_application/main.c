/*#include "FreeRTOS.h"
#include "led_driver.h"
#include "task.h"
void color_task(void *params) {
  static int row_counter = 0;
  static color_code_s current_color = RED;

  while (1) {
    // Set the current row to a color and clear the previous row
    led_driver__set_row(row_counter, current_color);

    // Cycle to the next row
    row_counter = (row_counter + 1) % 32;

    // Change color every time the rows cycle
    if (row_counter == 0) {
      if (current_color == RED)
        current_color = GREEN;
      else if (current_color == GREEN)
        current_color = BLUE;
      else
        current_color = RED;
    }

    vTaskDelay(pdMS_TO_TICKS(100)); // Adjust the speed as needed
  }
}

void clock_task(void *params) {
  while (1) {
    led_driver__update_display();
    vTaskDelay(3);
  }
}
int main(void) {
  // Initialize the LED matrix driver
  led_driver__init_gpio_pins_for_matrix();

  // Create tasks for updating the display and changing colors
  xTaskCreate(clock_task, "clock_task", 1024 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(color_task, "color_task", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  puts("Starting RTOS scheduler...\n");
  vTaskStartScheduler(); // Start the scheduler

  // The program should never reach this point
  return 0;
}
*/
/*
#include "FreeRTOS.h"
#include "gpio.h"
#include "joystick.h"
#include "task.h"
#include <stdio.h>

// Task to monitor the joystick input
void joystick_monitor_task(void *params) {
  while (1) {
    // Get the joystick position
    joystick_position_e position = joystick_buttons__get_joystick_position();

    // Print the joystick position to the console
    switch (position) {
    case JOYSTICK_LEFT:
      printf("Joystick Position: LEFT\n");
      break;
    case JOYSTICK_RIGHT:
      printf("Joystick Position: RIGHT\n");
      break;
    case NONE:
    default:
      printf("Joystick Position: NONE\n");
      break;
    }

    // Delay for 200ms before checking again
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

int main(void) {
  // Initialize joystick GPIO pins
  joystick_buttons__joystick_and_button_init();

  // Create a FreeRTOS task to monitor joystick input
  xTaskCreate(joystick_monitor_task, "Joystick Monitor", 1024, NULL, 1, NULL);

  // Start the FreeRTOS scheduler
  vTaskStartScheduler();

  // The program should never reach this point
  return 0;
}
*/
/*
#include "FreeRTOS.h"
#include "joystick.h"
#include "led_driver.h"
#include "task.h"
#include <stdio.h>

// Simulates paddle movement by lighting up LEDs on the matrix
static void simulate_paddle_movement(joystick_position_e joystick_position) {
  static int paddle_position = 16; // Start in the middle of the 32-column width

  // Clear the previous paddle position
  for (int col = 0; col < 32; col++) {
    led_driver__set_pixel(30, col, BLACK);
  }

  // Adjust the paddle position based on joystick input
  if (joystick_position == JOYSTICK_LEFT && paddle_position > 0) {
    paddle_position--;
  } else if (joystick_position == JOYSTICK_RIGHT && paddle_position < 31) {
    paddle_position++;
  }

  // Light up the paddle position (2 pixels wide for better visibility)
  led_driver__set_pixel(30, paddle_position, BLUE);
  if (paddle_position + 1 < 32) {
    led_driver__set_pixel(30, paddle_position + 1, BLUE);
  }
}

// Task to update paddle position based on joystick input
static void update_paddle_position(void *pv) {
  joystick_position_e joystick_position = NONE;

  for (;;) {
    // Get joystick position
    joystick_position = joystick_buttons__get_joystick_position();

    // Print joystick position to the console
    switch (joystick_position) {
    case NONE:
      printf("Joystick Position: NONE\n");
      break;
    case JOYSTICK_LEFT:
      printf("Joystick Position: LEFT\n");
      break;
    case JOYSTICK_RIGHT:
      printf("Joystick Position: RIGHT\n");
      break;
    }

    // Simulate paddle movement on the LED matrix
    simulate_paddle_movement(joystick_position);

    // Small delay to allow task switching
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// Task to refresh the LED display
static void update_display_task(void *pv) {
  for (;;) {
    led_driver__update_display();
    vTaskDelay(pdMS_TO_TICKS(3)); // Refresh the display at high frequency
  }
}

int main(void) {
  // Initialize peripherals
  led_driver__init_gpio_pins_for_matrix();
  joystick_buttons__joystick_and_button_init();

  // Clear the LED matrix initially
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 64; col++) {
      led_driver__set_pixel(row, col, BLACK);
    }
  }

  // Create FreeRTOS tasks
  xTaskCreate(update_paddle_position, "update_paddle_position", 4096 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(update_display_task, "update_display_task", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  // Start the FreeRTOS scheduler
  vTaskStartScheduler();

  // Should never reach here
  return 0;
}
*/
/**
#include "FreeRTOS.h"
#include "led_driver.h"
#include "task.h"

// Define the "HELLO WORLD" bitmap
static const uint8_t hello_world_bitmap[11][7] = {
    {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}, // H
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}, // E
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // L
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // L
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // O
    {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}, // Space
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}, // W
    {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110}, // O
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // R
    {0b01111, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b10001}, // L
    {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110}, // D
};

void draw_hello_world_fixed(void) {
  int char_width = 6;  // Character width including spacing
  int char_height = 7; // Character height
  int x_start = 5;     // Starting x-coordinate for centering horizontally
  int y_start = 8;     // Starting y-coordinate for centering vertically

  // Clear the matrix
  for (int row = 0; row < 32; row++) {
    for (int col = 0; col < 64; col++) {
      led_driver__set_pixel(row, col, BLACK);
    }
  }

  // Draw each character
  for (int char_idx = 0; char_idx < 11; char_idx++) { // Loop through each character
    for (int row = 0; row < char_height; row++) {     // Loop through bitmap rows
      for (int col = 0; col < 5; col++) {             // Loop through bitmap columns
        if (hello_world_bitmap[char_idx][row] & (1 << (4 - col))) {
          // Adjust the pixel position for each character
          int x_pos = x_start + char_idx * char_width + col;
          int y_pos = y_start + row;

          // Ensure we stay within matrix bounds
          if (x_pos < 64 && y_pos < 32) {
            led_driver__set_pixel(y_pos, x_pos, GREEN);
          }
        }
      }
    }
  }
}

// Task to draw the "HELLO WORLD" text
void draw_hello_world_task(void *params) {
  draw_hello_world_fixed();
  while (1) {
    led_driver__update_display(); // Refresh display
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// Task to handle display updates
void update_display_task(void *params) {
  while (1) {
    led_driver__update_display();
    vTaskDelay(pdMS_TO_TICKS(3)); // Refresh rate
  }
}

int main(void) {
  // Initialize GPIO pins for LED matrix
  led_driver__init_gpio_pins_for_matrix();

  // Create tasks
  xTaskCreate(draw_hello_world_task, "Draw Hello World", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  // Start the RTOS scheduler
  vTaskStartScheduler();

  // Code should never reach here
  return 0;
}
*/

/*#include "FreeRTOS.h"
#include "led_driver.h"
#include "task.h"
#include <stdio.h>

// Define character bitmaps for "HELLO"
static const uint8_t hello_bitmap[5][7] = {
    {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}, // H
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}, // E
    {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}, // L
    {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}, // L
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}  // O
};

// Task to display "HELLO" on the LED matrix with improved brightness
void display_hello_task(void *params) {
  while (1) {
    // Clear the matrix first
    for (int row = 0; row < 32; row++) {
      for (int col = 0; col < 64; col++) {
        led_driver__set_pixel(row, col, BLACK);
      }
    }

    // Draw "HELLO" on the matrix
    int char_width = 6; // Character width including spacing
    int x_start = 6;    // Adjusted x-coordinate for centering "HELLO"
    int y_start = 8;    // Starting y-coordinate for vertical alignment

    for (int char_idx = 0; char_idx < 5; char_idx++) { // Loop through each character
      for (int row = 0; row < 7; row++) {              // Loop through bitmap rows
        for (int col = 0; col < 5; col++) {            // Loop through bitmap columns
          if (hello_bitmap[char_idx][row] & (1 << (4 - col))) {
            // Set pixels for "HELLO" centered in the matrix
            led_driver__set_pixel(y_start + row, x_start + char_idx * char_width + col, WHITE);
          }
        }
      }
    }

    led_driver__update_display();   // Refresh the display
    vTaskDelay(pdMS_TO_TICKS(500)); // Delay for smooth rendering
  }
}

// Task to periodically refresh the display with optimized brightness
void refresh_display_task(void *params) {
  while (1) {
    led_driver__update_display();
    vTaskDelay(pdMS_TO_TICKS(1)); // Minimal delay for high refresh rate
  }
}

// Main function
int main(void) {
  // Initialize GPIO pins for the LED matrix
  printf("Initializing LED driver GPIO pins...\n");
  led_driver__init_gpio_pins_for_matrix();

  printf("Creating tasks...\n");
  // Create tasks
  xTaskCreate(refresh_display_task, "refresh_display_task", 1024 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(display_hello_task, "display_hello_task", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  puts("Starting RTOS scheduler...\n");

  // Start the FreeRTOS scheduler
  vTaskStartScheduler();

  // Should never reach here
  return 0;
}
*/
/*
#include "FreeRTOS.h"
#include "game_screens.h"
#include "game_state_machine.h"
#include "led_driver.h"
#include "menu.h"
#include "task.h"
// #include "periodic_scheduler.h"

static void update_display_task(void *pv) {
  for (;;) {
    led_driver__update_display();
    vTaskDelay(3);
  }
}

static void game_state_machine_task(void *pv) {
  for (;;) {
    game_state_machine__run_game();
    vTaskDelay(10);
  }
}
int main(void) {
  led_driver__init_gpio_pins_for_matrix();
  game_screens__set_matrix_to(GAME);
  game_state_machine__test_setup();
  // for (;;) {
  //   led_driver__update_display();
  //   delay__ms(3);
  // }
  // return 0;
  // xTaskCreate(clock_task, "clock_task", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(color_task, "color_task", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  xTaskCreate(update_display_task, "update_display_task", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(game_state_machine_task, "game_state_machine_task", 10000 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  puts("Starting RTOS scheduler.\n");
  vTaskStartScheduler();
}
*/

/*#include "FreeRTOS.h"
#include "game_screens.h"
#include "game_state_machine.h"
#include "led_driver.h"
#include "task.h"

// #include "periodic_scheduler.h"

static void update_display_task(void *pv) {
  for (;;) {
    led_driver__update_display();
    vTaskDelay(3);
  }
}

static void game_state_machine_task(void *pv) {
  for (;;) {
    game_state_machine__run_game();
    vTaskDelay(10);
  }
}

int main(void) {
  led_driver__init_gpio_pins_for_matrix();
  game_screens__set_matrix_to(GAME);
  game_state_machine__test_setup();
  // for (;;) {
  //   led_driver__update_display();
  //   delay__ms(3);
  // }
  // return 0;
  // xTaskCreate(clock_task, "clock_task", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  // xTaskCreate(color_task, "color_task", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  xTaskCreate(update_display_task, "update_display_task", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(game_state_machine_task, "game_state_machine_task", 10000 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  puts("Starting RTOS scheduler.\n");
  vTaskStartScheduler();
}
*/
/*
#include "FreeRTOS.h"
#include "button.h"
#include "task.h"

// Task that waits for a button press
void button_task(void *params) {
  while (1) {
    if (xSemaphoreTake(button_semaphore, portMAX_DELAY)) {
      printf("Button was pressed!\n");
    }
  }
}

int main(void) {
  // Initialize the button
  button_init();

  // Create a task to handle button press
  xTaskCreate(button_task, "Button Task", 1024, NULL, 1, NULL);

  // Start the scheduler
  vTaskStartScheduler();

  // Should never reach here
  return 0;
}
*/

/*#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS.h"
#include "acceleration.h"
#include "board_io.h"
#include "common_macros.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"
#include "task.h"
#include <stdio.h>

// 'static' to make these functions 'private' to this file

void accelerometer_task(void *pvParameters) {
  if (acceleration__init()) {
    printf("Accelerometer initialized successfully!\n");
  } else {
    printf("Failed to initialize accelerometer.\n");
  }

  while (1) {
    acceleration__axis_data_s accel_data = acceleration__get_data();

    printf("X: %d, Y: %d, Z: %d\n", accel_data.x, accel_data.y, accel_data.z);

    vTaskDelay(100);
  }
}

int main(void) {

  // If you have the ESP32 wifi module soldered on the board, you can try uncommenting this code
  // See esp32/README.md for more details
  // uart3_init();                                                                     // Also include:  uart3_init.h
  // xTaskCreate(esp32_tcp_hello_world_task, "uart3", 1000, NULL, PRIORITY_LOW, NULL); // Include esp32_task.h

  xTaskCreate(accelerometer_task, "Accelerometer Task", 2048 / sizeof(void *), NULL, 1, NULL);

  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}
*/

#include "FreeRTOS.h"
#include "game_screens.h"
#include "game_state_machine.h"
#include "led_driver.h"
#include "task.h"
#include <stdio.h>

static void update_display_task(void *pv) {
  for (;;) {
    led_driver__update_display();
    vTaskDelay(3);
  }
}

static void game_state_machine_task(void *pv) {
  for (;;) {
    game_state_machine__run_game();
    vTaskDelay(36);
  }
}

static void update_paddle_position(void *pv) {
  static joystick_position_e joystick_position = NONE;
  for (;;) {
    joystick_position = joystick_buttons__get_joystick_position();
    // switch (joystick_position) {
    // case NONE:
    //   fprintf(stderr, "NONE\n");
    //   break;
    // case JOYSTICK_LEFT:
    //   fprintf(stderr, "LEFT\n");
    //   break;
    // case JOYSTICK_RIGHT:
    //   fprintf(stderr, "RIGHT\n");
    //   break;
    // }
    game_state_machine__update_paddle_position(joystick_position);
    vTaskDelay(12);
  }
}

int main(void) {
  led_driver__init_gpio_pins_for_matrix();
  game_screens__set_matrix_to(GAME);
  game_state_machine__test_setup();
  joystick_buttons__joystick_and_button_init();

  xTaskCreate(update_paddle_position, "update_paddle_position", 4096 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(update_display_task, "update_display_task", 4096 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(game_state_machine_task, "game_state_machine_task", 10000 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  puts("Starting RTOS scheduler.\n");
  vTaskStartScheduler();
}
