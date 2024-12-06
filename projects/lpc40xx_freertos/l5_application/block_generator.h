#pragma once

#include <stdint.h>

void block_generator__generate_blocks_for_level(uint8_t number_of_rows);
void block_generator__delete_block(uint8_t row, uint8_t col);
