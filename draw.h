#pragma once
#include "tetris.h"   // BOARD_HEIGHT, BOARD_WIDTH

void init_colors(void);

// 렌더는 보드를 수정하지 않으므로 const로 고정
void draw_board(const int board[BOARD_HEIGHT][BOARD_WIDTH]);

void draw_block(int y, int x, int type, int rotation);
void draw_ghost_block(int y, int x, int type, int rotation);
void draw_mino_preview(int top, int left, int type, int rotation);
void draw_line_clear_anim(const int rows[], int count, int frame);