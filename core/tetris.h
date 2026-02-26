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