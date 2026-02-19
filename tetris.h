#ifndef TETRIS_H
#define TETRIS_H

#include <stdbool.h>
#include "config.h"

void init_game(void);
bool check_collision(int n_y, int n_x, int type, int rotation);
void freeze_block(int y, int x, int type, int rotation);
int clear_lines(void);
void update_game(void);

#endif /* TETRIS_H */
