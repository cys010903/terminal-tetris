#ifndef TETRIS_H
#define TETRIS_H

#pragma once
#include <stdbool.h>
#include "config.h"

#define BLOCK_KIND 7
#define BLOCK_SIZE 4
#define EMPTY 0

extern int board[BOARD_HEIGHT][BOARD_WIDTH];

bool check_collision(int n_y, int n_x, int type, int rotation);
void freeze_block(int y, int x, int type, int rotation);


// ===== line clear (애니메이션/연출용으로 탐지/적용 분리) =====
// out_rows: 가득 찬 줄의 y 인덱스를 아래->위 순서로 최대 4개 채움
int  tetris_find_full_lines(int out_rows[4]);
void tetris_remove_lines(const int rows[], int count);
int clear_lines(void);


#endif /* TETRIS_H */
