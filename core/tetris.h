#pragma once
#include <stdbool.h>  
#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_KIND 7
#define BLOCK_SIZE 4
#define EMPTY 0

#ifndef BOARD_WIDTH
#define BOARD_WIDTH 10
#endif

#ifndef BOARD_HEIGHT
#define BOARD_HEIGHT 20
#endif


int tetris_cell(int y, int x);
int tetris_mino_cell(int type, int rotation, int r, int c);
void tetris_clear_board(void); // 보드 초기화

// line utils
int  tetris_find_full_lines(int out_rows[4]);
void tetris_remove_lines(const int rows[], int count);

// placement / movement (Scene가 충돌판단 직접 금지)
bool tetris_can_place(int y, int x, int type, int rot);
bool tetris_try_move(int* io_y, int* io_x, int type, int rot, int dx, int dy);

// ghost / hard drop helper
int  tetris_ghost_y(int y, int x, int type, int rot);
int  tetris_hard_drop_y(int y, int x, int type, int rot);

//충돌 검사용
bool check_collision(int n_y, int n_x, int type, int rotation);
void freeze_block(int y, int x, int type, int rotation);

//
bool tetris_try_rotatre(int* io_y, int* io_x, int type, int* io_rot, int dir);

//애니매이션 연출용
int  tetris_find_full_lines(int out_rows[4]);
void tetris_remove_lines(const int rows[], int count);
int clear_lines(void);

#ifdef __cplusplus
}
#endif