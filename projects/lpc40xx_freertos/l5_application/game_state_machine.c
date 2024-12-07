#include "game_state_machine.h"
#include "block_generator.h"
#include "game_screens.h"
#include "joystick_buttons.h"
#include "led_driver.h"

#include "stdbool.h"

#include "lpc40xx.h"

#define TEST

#include "delay.h"
#include <stdio.h>

typedef enum {
  MAINMENU_s = 0,
  OPTIONS_s = 1,
  HISCORE_s = 2,
  GAME_s = 3,
  GAMEOVER_s = 4,
} game_states_e;

typedef enum {
  TOP_RIGHT = 0,
  TOP_LEFT = 1,
  BOTTOM_LEFT = 2,
  BOTTOM_RIGHT = 3,
} ball_direction_e;

typedef enum {
  NEGATIVE_THREE = -3,
  NEGATIVE_TWO = -2,
  NEGATIVE_ONE = -1,
  POSITIVE_ONE = 1,
  POSITIVE_TWO = 2,
  POSITIVE_THREE = 3,
} ball_slope_e;

typedef struct {
  uint8_t ball_row;
  uint8_t ball_col;
  ball_direction_e ball_dir;
} ball_position_s;

static const uint8_t top_of_game_area = 7;
static const uint8_t bottom_of_game_area = 63;
static const uint8_t right_wall_of_game_area = 63;
static const uint8_t left_wall_of_game_area = 0;

static volatile game_states_e current_state = MAINMENU_s;
static volatile ball_slope_e ball_slope = POSITIVE_ONE;
static volatile ball_position_s ball_pos;
static volatile ball_position_s next_pos;
static volatile uint8_t paddle_length = 0;
static volatile uint8_t paddle_start_col = 0;
static volatile uint8_t paddle_end_col = 0;

static void game_state_machine__generate_paddle_and_ball(void) {
  const uint8_t initial_paddle_length = 5;
  const uint8_t last_row = 63;
  const uint8_t middle_of_paddle = 2;

  for (int col = 0; col < initial_paddle_length; col++) {
    led_driver__set_pixel(63, 30 + col, WHITE);
    if (col == middle_of_paddle) {
      led_driver__set_pixel(62, 30 + col, CYAN);
      ball_pos.ball_col = 30 + col;
      ball_pos.ball_row = 62;
      ball_pos.ball_dir = TOP_RIGHT;
      next_pos.ball_col = ball_pos.ball_col;
      next_pos.ball_row = ball_pos.ball_row;
      paddle_length = initial_paddle_length;
    }
  }
  paddle_start_col = 30;
  paddle_end_col = paddle_start_col + paddle_length - 1;
}

static void game_state_machine__negate_ball_slope(void) {
  switch (ball_slope) {
  case NEGATIVE_THREE:
    ball_slope = POSITIVE_THREE;
    break;
  case NEGATIVE_TWO:
    ball_slope = POSITIVE_TWO;
    break;
  case NEGATIVE_ONE:
    ball_slope = POSITIVE_ONE;
    break;
  case POSITIVE_ONE:
    ball_slope = NEGATIVE_ONE;
    break;
  case POSITIVE_TWO:
    ball_slope = NEGATIVE_TWO;
    break;
  case POSITIVE_THREE:
    ball_slope = NEGATIVE_THREE;
    break;
  }
}

static void game_state_machine__calculate_next_ball_position(void) {
  switch (ball_pos.ball_dir) {
  case TOP_RIGHT:
    /* column math */
    if (ball_pos.ball_col + ball_slope > right_wall_of_game_area) {
      next_pos.ball_col = right_wall_of_game_area;
    } else {
      next_pos.ball_col += ball_slope;
    }
    /* row math */
    if (ball_pos.ball_row - 1 == top_of_game_area) {
      next_pos.ball_row = top_of_game_area + 1;
    } else {
      next_pos.ball_row = ball_pos.ball_row - 1;
    }
    break;
  case TOP_LEFT:
    if (ball_pos.ball_col + ball_slope < left_wall_of_game_area) {
      next_pos.ball_col = left_wall_of_game_area;
    } else {
      next_pos.ball_col += ball_slope;
    }
    if (ball_pos.ball_row - 1 == top_of_game_area) {

      next_pos.ball_row = top_of_game_area + 1;
    } else {
      next_pos.ball_row = ball_pos.ball_row - 1;
    }
    break;
  case BOTTOM_RIGHT:
    if (ball_pos.ball_col + ball_slope > right_wall_of_game_area) {
      next_pos.ball_col = right_wall_of_game_area;
    } else {
      if (ball_pos.ball_col + ball_slope <= 0) {
        next_pos.ball_col = left_wall_of_game_area;
      } else {
        next_pos.ball_col += ball_slope;
      }
    }
    next_pos.ball_row = ball_pos.ball_row + 1;
    break;
  case BOTTOM_LEFT:
    if (ball_pos.ball_col + ball_slope < left_wall_of_game_area) {
      next_pos.ball_col = left_wall_of_game_area;
    } else {
      next_pos.ball_col += ball_slope;
    }
    next_pos.ball_row = ball_pos.ball_row + 1;
    break;
  }
}

static uint8_t game_state_machine__get_color_of_next_pos(void) {
  uint8_t color = led_driver__get_led_matix_value(next_pos.ball_row, next_pos.ball_col);
  return color;
}

static void game_state_machine__update_ball_pos_with_next_pos(void) {
  ball_pos.ball_col = next_pos.ball_col;
  ball_pos.ball_row = next_pos.ball_row;
}

static bool game_state_machine__check_for_block_collision(uint8_t row, uint8_t col) {
  bool collision_detected = false;
  uint8_t number_of_blocks_per_row = 8;
  uint8_t block_height = 3;

  uint8_t start_of_block_col = (col / number_of_blocks_per_row) * number_of_blocks_per_row;
  uint8_t offset_val = row % block_height;
  uint8_t start_of_block_row = row - (offset_val + 1);

  color_code_s color_of_next_pixel = led_driver__get_led_matix_value(row, col);

  if (color_of_next_pixel == BLACK || color_of_next_pixel == CYAN || color_of_next_pixel == WHITE) {
    collision_detected = false;
  } else {
    collision_detected = true;
  }
  return collision_detected;
}

static bool game_state_machine__check_for_wall_collision(uint8_t row, uint8_t col) {
  bool wall_detected = false;
  if (col == right_wall_of_game_area || col == left_wall_of_game_area) {
    wall_detected = true;
  }
  return wall_detected;
}

static bool game_state_machine__check_for_collision_with_paddle(uint8_t row, uint8_t col) {
  bool paddle_detected = false;
  uint8_t color_of_next_pixel = led_driver__get_led_matix_value(row, col);
  if (color_of_next_pixel == WHITE) {
    fprintf(stderr, "paddle hit!\n");

    paddle_detected = true;
  }
  return paddle_detected;
}

static bool game_state_machine__check_for_collision_with_top(uint8_t row, uint8_t col) {
  bool collision_with_top = false;
  if (row == top_of_game_area + 1) {
    collision_with_top = true;
  }
  return collision_with_top;
}

static bool game_state_machine__check_for_corner_of_blocks() {
  bool ball_in_corner = false;
  uint8_t pixel_above_or_below_ball = BLACK;
  uint8_t pixel_to_side_of_ball = BLACK;
  switch (ball_pos.ball_dir) {
  case TOP_LEFT:
    if (ball_pos.ball_row == top_of_game_area + 1) {
      ball_in_corner = false;
    } else {
      pixel_above_or_below_ball = led_driver__get_led_matix_value(ball_pos.ball_row - 1, ball_pos.ball_col);
      pixel_to_side_of_ball = led_driver__get_led_matix_value(ball_pos.ball_row, ball_pos.ball_col - 1);
      if (pixel_to_side_of_ball != BLACK && pixel_above_or_below_ball != BLACK) {
        /* Ball is in the corner of two blocks */
        ball_in_corner = true;
      }
    }
    break;
  case TOP_RIGHT:
    if (ball_pos.ball_row == top_of_game_area + 1) {
      ball_in_corner = false;
    } else {
      pixel_above_or_below_ball = led_driver__get_led_matix_value(ball_pos.ball_row - 1, ball_pos.ball_col);
      pixel_to_side_of_ball = led_driver__get_led_matix_value(ball_pos.ball_row, ball_pos.ball_col + 1);
      if (pixel_to_side_of_ball != BLACK && pixel_above_or_below_ball != BLACK) {
        ball_in_corner = true;
      }
    }
    break;
  case BOTTOM_RIGHT:
    if (ball_pos.ball_row > 32) {
      ball_in_corner = false;
    } else {
      pixel_above_or_below_ball = led_driver__get_led_matix_value(ball_pos.ball_row + 1, ball_pos.ball_col);
      pixel_to_side_of_ball = led_driver__get_led_matix_value(ball_pos.ball_row, ball_pos.ball_col + 1);
      if (pixel_to_side_of_ball != BLACK && pixel_above_or_below_ball != BLACK) {
        ball_in_corner = true;
      }
    }
    break;
  case BOTTOM_LEFT:
    if (ball_pos.ball_row > 32) {
      ball_in_corner = false;
    } else {
      pixel_above_or_below_ball = led_driver__get_led_matix_value(ball_pos.ball_row + 1, ball_pos.ball_col);
      pixel_to_side_of_ball = led_driver__get_led_matix_value(ball_pos.ball_row, ball_pos.ball_col - 1);
      if (pixel_to_side_of_ball != BLACK && pixel_above_or_below_ball != BLACK) {
        ball_in_corner = true;
      }
    }
    break;
  }
  return ball_in_corner;
}

static void game_state_machine__update_ball_position(void) {
  bool colission_with_block = false;
  bool colission_with_wall = false;
  bool colission_with_paddle = false;
  bool colission_with_top = false;
  uint8_t color_of_next_pixel = BLACK;
  led_driver__set_pixel(ball_pos.ball_row, ball_pos.ball_col, BLACK);
  game_state_machine__calculate_next_ball_position();
  color_of_next_pixel = game_state_machine__get_color_of_next_pos();

  colission_with_block = game_state_machine__check_for_block_collision(next_pos.ball_row, next_pos.ball_col);
  colission_with_wall = game_state_machine__check_for_wall_collision(next_pos.ball_row, next_pos.ball_col);
  colission_with_paddle = game_state_machine__check_for_collision_with_paddle(next_pos.ball_row, next_pos.ball_col);
  colission_with_top = game_state_machine__check_for_collision_with_top(next_pos.ball_row, next_pos.ball_col);

  /* The consider wall hits */

  if (!colission_with_block) {
    /* Did not hit block */
    switch (ball_pos.ball_dir) {
    case TOP_RIGHT:
      if (colission_with_top) {
        ball_pos.ball_dir = BOTTOM_RIGHT;
      } else if (colission_with_wall) {

        ball_pos.ball_dir = TOP_LEFT;
        game_state_machine__negate_ball_slope();
      }

      break;
    case TOP_LEFT:
      if (colission_with_top) {
        ball_pos.ball_dir = BOTTOM_LEFT;
      } else if (colission_with_wall) {

        ball_pos.ball_dir = TOP_RIGHT;
        game_state_machine__negate_ball_slope();
      }

      break;
    case BOTTOM_RIGHT:
      if (colission_with_paddle) {
        next_pos.ball_row--;
        ball_pos.ball_dir = TOP_RIGHT;
        if (colission_with_wall) {
          ball_pos.ball_dir = TOP_LEFT;
          game_state_machine__negate_ball_slope();
        }
      } else if (colission_with_wall) {
        ball_pos.ball_dir = BOTTOM_LEFT;
        game_state_machine__negate_ball_slope();
      } else if (next_pos.ball_row == bottom_of_game_area) {
        /* TEST bounce on bottom */
        ball_pos.ball_dir = TOP_RIGHT;
      }

      break;
    case BOTTOM_LEFT:
      if (colission_with_paddle) {
        next_pos.ball_row--;
        ball_pos.ball_dir = TOP_LEFT;
        if (colission_with_wall) {
          ball_pos.ball_dir = TOP_RIGHT;
          game_state_machine__negate_ball_slope();
        }
      } else if (colission_with_wall) {
        ball_pos.ball_dir = BOTTOM_RIGHT;
        game_state_machine__negate_ball_slope();
      } else if (next_pos.ball_row == bottom_of_game_area) {
        /* Test bounce on bottom */
        ball_pos.ball_dir = TOP_LEFT;
      }
      break;
    }

  } else {
    /* Block Hit */
    bool ball_in_corner = game_state_machine__check_for_corner_of_blocks(next_pos.ball_row, next_pos.ball_col);
    switch (ball_pos.ball_dir) {
    case TOP_RIGHT:
      if (ball_in_corner) {
        fprintf(stderr, "TR: in corner\n");
        if (ball_slope == POSITIVE_ONE) {
          /* Break all three */
          fprintf(stderr, "TR: breaking 3 blocks\n");
          block_generator__delete_block(next_pos.ball_row, next_pos.ball_col);
        }
        /* Break top and right blocks */
        block_generator__delete_block(ball_pos.ball_row - 1, ball_pos.ball_col);
        block_generator__delete_block(ball_pos.ball_row, ball_pos.ball_col + 1);
        ball_pos.ball_dir = BOTTOM_LEFT;
        game_state_machine__negate_ball_slope();

        next_pos.ball_row = ball_pos.ball_row;
        next_pos.ball_col = ball_pos.ball_col;
      } else {
        /* Delete block hit */
        block_generator__delete_block(next_pos.ball_row, next_pos.ball_col);
        if (next_pos.ball_col % 8 == 0) {
          ball_pos.ball_dir = TOP_LEFT;
          game_state_machine__negate_ball_slope();
        } else {
          ball_pos.ball_dir = BOTTOM_RIGHT;
        }
        if (next_pos.ball_col == right_wall_of_game_area) {
          ball_pos.ball_dir = BOTTOM_LEFT;
          game_state_machine__negate_ball_slope();
        }
      }

      break;
    case TOP_LEFT:
      if (ball_in_corner) {
        if (ball_slope == NEGATIVE_ONE) {
          block_generator__delete_block(next_pos.ball_row, next_pos.ball_col);
        } else {
          /* break top and left blocks */
          block_generator__delete_block(ball_pos.ball_row - 1, ball_pos.ball_col);
          block_generator__delete_block(ball_pos.ball_row, ball_pos.ball_col - 1);
          ball_pos.ball_dir = BOTTOM_RIGHT;
          game_state_machine__negate_ball_slope();
        }
        next_pos.ball_row = ball_pos.ball_row;
        next_pos.ball_col = ball_pos.ball_col;
      } else {
        block_generator__delete_block(next_pos.ball_row, next_pos.ball_col);
        if (next_pos.ball_col % 8 == 7) {
          ball_pos.ball_dir = TOP_RIGHT;
          game_state_machine__negate_ball_slope();
        } else {
          ball_pos.ball_dir = BOTTOM_LEFT;
        }
        if (next_pos.ball_col == left_wall_of_game_area) {
          ball_pos.ball_dir = BOTTOM_RIGHT;
          game_state_machine__negate_ball_slope();
        }
      }

      break;
    case BOTTOM_RIGHT:
      if (ball_in_corner) {
        if (ball_slope == POSITIVE_ONE) {
          block_generator__delete_block(next_pos.ball_row, next_pos.ball_col);
        }
        /* break top and left blocks */
        block_generator__delete_block(ball_pos.ball_row - 1, ball_pos.ball_col);
        block_generator__delete_block(ball_pos.ball_row, ball_pos.ball_col - 1);
        ball_pos.ball_dir = BOTTOM_RIGHT;
        game_state_machine__negate_ball_slope();

        next_pos.ball_row = ball_pos.ball_row;
        next_pos.ball_col = ball_pos.ball_col;
      } else {
        /* Block hit */
        block_generator__delete_block(next_pos.ball_row, next_pos.ball_col);

        if (next_pos.ball_col % 8 == 0) {
          ball_pos.ball_dir = BOTTOM_LEFT;
          game_state_machine__negate_ball_slope();
        } else {
          ball_pos.ball_dir = TOP_RIGHT;
        }

        if (next_pos.ball_col == right_wall_of_game_area) {
          ball_pos.ball_dir = TOP_LEFT;
          game_state_machine__negate_ball_slope();
        }
      }

      break;
    case BOTTOM_LEFT:
      if (ball_in_corner) {

        if (ball_slope == NEGATIVE_ONE) {
          block_generator__delete_block(next_pos.ball_row, next_pos.ball_col);
        } else {
          block_generator__delete_block(ball_pos.ball_row + 1, ball_pos.ball_col);
          block_generator__delete_block(ball_pos.ball_row, ball_pos.ball_col - 1);
          ball_pos.ball_dir = TOP_RIGHT;
          game_state_machine__negate_ball_slope();
        }
        next_pos.ball_row = ball_pos.ball_row;
        next_pos.ball_col = ball_pos.ball_col;
      } else {

        block_generator__delete_block(next_pos.ball_row, next_pos.ball_col);

        if (next_pos.ball_col % 8 == 7) {

          ball_pos.ball_dir = BOTTOM_RIGHT;
          game_state_machine__negate_ball_slope();
        } else {

          ball_pos.ball_dir = TOP_LEFT;
        }

        if (next_pos.ball_col == left_wall_of_game_area) {

          ball_pos.ball_dir = TOP_RIGHT;
          game_state_machine__negate_ball_slope();
        }
      }
      break;
    }
  }
  game_state_machine__update_ball_pos_with_next_pos();
  led_driver__set_pixel(ball_pos.ball_row, ball_pos.ball_col, CYAN);
}

#ifdef TEST
// Test code
#include "gpio.h"
#include "lpc_peripherals.h"
#include "stdio.h"

static volatile int counter = 0;

static void game_state_machine__get_next_state(void) {
  if (counter == 0) {
    current_state = MAINMENU_s;
  } else if (counter == 1) {
    current_state = OPTIONS_s;
  } else if (counter == 2) {
    current_state = HISCORE_s;
  } else if (counter == 3) {
    current_state = GAME_s;
  }
}

static void gpio0_29_isr(void) {
  fprintf(stderr, "0_29\n");
  if (counter < 3) {
    counter++;
  }
  LPC_GPIOINT->IO0IntClr |= (1 << 29);
}

void game_state_machine__test_setup(void) {
  gpio_s switch0_29 = gpio__construct_as_input(GPIO__PORT_0, 29);
  LPC_GPIOINT->IO0IntEnR |= (1 << 29);
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0_29_isr, "gpio 0_29 isr");
}

#endif

static void game_state_machine__move_paddle(uint8_t starting_col, uint8_t ending_col) {
  for (int i = 0; i < paddle_length; i++) {
    led_driver__set_pixel(bottom_of_game_area, paddle_start_col + i, BLACK);
  }
  paddle_start_col = starting_col;
  paddle_end_col = ending_col;
  for (int j = 0; j < paddle_length; j++) {
    led_driver__set_pixel(bottom_of_game_area, paddle_start_col + j, WHITE);
  }
}

void game_state_machine__update_paddle_position(joystick_position_e position) {
  if (position == JOYSTICK_LEFT) {
    if (paddle_start_col == left_wall_of_game_area) {
      // dont move paddle, will go out of bounds
    } else {
      game_state_machine__move_paddle(paddle_start_col - 1, paddle_end_col - 1);
    }
  } else if (position == JOYSTICK_RIGHT) {
    if (paddle_end_col == right_wall_of_game_area) {
      // dont move paddle, will go out of bounds
    } else {
      game_state_machine__move_paddle(paddle_start_col + 1, paddle_end_col + 1);
    }
  } else if (position == NONE) {
    // NONE - paddle shouldn't move
  }
}

void game_state_machine__run_game(void) {

  static bool stage_init = true;
  switch (current_state) {
  case MAINMENU_s:
    game_screens__set_matrix_to(MAINMENU);
    break;
  case OPTIONS_s:
    game_screens__set_matrix_to(OPTIONS);
    break;
  case HISCORE_s:
    game_screens__set_matrix_to(HISCORE);
    break;
  case GAME_s:

    if (stage_init == true) {
      game_screens__set_matrix_to(GAME);
      game_state_machine__generate_paddle_and_ball();
      block_generator__generate_blocks_for_level(8);

      stage_init = false;
    }
    joystick_position_e position = joystick_buttons__get_joystick_position();
    game_state_machine__update_ball_position();
    // current_state = GAME_s;
    break;
  case GAMEOVER_s:
    // make gameover screen
    break;
  }
  game_state_machine__get_next_state();
}
