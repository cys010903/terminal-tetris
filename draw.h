#ifndef DRAW_H
#define DRAW_H

#include "config.h"

void init_colors(void);
void draw_block(int y, int x, int type, int rotation);
void draw_board(int board[BOARD_HEIGHT][BOARD_WIDTH]);
void draw_mino_preview(int top, int left, int type, int rotation);
void draw_ghost_block(int y, int x, int type, int rotation);
/* (구현이 있다면) 타이틀 렌더용 */
void draw_title(void);

#endif /* DRAW_H */
