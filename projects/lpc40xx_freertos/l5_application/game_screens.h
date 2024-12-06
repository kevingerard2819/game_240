#pragma once

typedef enum {
  MAINMENU,
  HISCORE,
  OPTIONS,
  GAME,
} game_screens_e;

void game_screens__set_matrix_to(game_screens_e screen);
