#ifndef DRAW_H
#define DRAW_H

#include "config.h"

void init_colors(void);
void draw_block(int y, int x, int type, int rotation);
void draw_board(int board[BOARD_HEIGHT][BOARD_WIDTH]);
void draw_mino_preview(int top, int left, int type, int rotation);
void draw_ghost_block(int y, int x, int type, int rotation);
void draw_line_clear_anim(const int rows[], int count, int frame);

#endif /* DRAW_H */
