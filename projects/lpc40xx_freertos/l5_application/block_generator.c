#include "block_generator.h"
#include "led_driver.h"
#include <stdio.h>
#include <stdlib.h>

static const uint8_t top_of_game_area = 7;
static const uint8_t block_length = 8;
static const uint8_t block_height = 3;
static const color_code_s color_array[5] = {RED, GREEN, YELLOW, BLUE, PURPLE};
static const uint8_t beginning_of_block = 0;
static const uint8_t end_of_block = 7;

static const uint8_t number_of_blocks_per_row = 64 / block_length;

static void block_generator__generate_block(uint8_t row, uint8_t col, color_code_s color) {
  for (int i = 0; i < block_height; i++) {
    for (int j = 0; j < block_length; j++) {
      if (i == 1) {
        if (j == beginning_of_block || j == end_of_block) {
          led_driver__set_pixel(row + i, col + j, color);
        }
      } else {
        led_driver__set_pixel(row + i, col + j, color);
      }
    }
  }
}

static void block_generator__generate_row_of_blocks(uint8_t row, color_code_s color) {
  uint8_t col = 0;
  uint8_t color_of_block = color;
  while (col < 64) {
    block_generator__generate_block(row, col, color_array[color_of_block]);
    col += block_length;
    /* TODO: read ultrasonic and mod to get random value for block color somehow */
    if (color_of_block == 4) {
      color_of_block = 0;
    } else {
      color_of_block++;
    }
  }
}

void block_generator__generate_blocks_for_level(uint8_t number_of_rows) {
  // red - 1, green - 2, yellow - 3, blue - 4, purple - 5,
  uint8_t row = top_of_game_area + 1;
  uint8_t color = (rand() % 5);
  for (int i = 0; i < number_of_rows; i++) {
    block_generator__generate_row_of_blocks(row, color);
    row += block_height;
    color = rand() % 5;
  }
}

void block_generator__delete_block(uint8_t row, uint8_t col) {
  uint8_t start_of_block_col = (col / number_of_blocks_per_row) * number_of_blocks_per_row;
  uint8_t row_offset = 8;
  uint8_t block_row = (row - row_offset) / block_height;
  uint8_t start_of_block_row = (block_row * block_height) + row_offset;

  for (int i = 0; i < block_height; i++) {
    for (int j = 0; j < block_length; j++) {
      led_driver__set_pixel(start_of_block_row + i, start_of_block_col + j, BLACK);
    }
  }
}
