#pragma once
#include <stdbool.h>  
#include "config.h"
#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_KIND 7
#define BLOCK_SIZE 4
#define EMPTY 0

// === Step2: NEXT/HOLD는 Core 소유 ===
#define TETRIS_NEXT_COUNT 5

typedef enum {
    TETRIS_RNG_PURE = 0,
    TETRIS_RNG_7BAG = 1
} TetrisRandomizer;

#ifndef BOARD_WIDTH
#define BOARD_WIDTH 10
#endif

#ifndef BOARD_HEIGHT
#define BOARD_HEIGHT 20
#endif

void tetris_randomizer_set(TetrisRandomizer mode);
void tetris_next_reset(void);
int  tetris_next_pop(void);
void tetris_next_peek(int out[TETRIS_NEXT_COUNT]);

void tetris_hold_reset(void);
void tetris_hold_new_turn(void);                 // 미노 고정 후 호출(= hold 재허용)
bool tetris_hold_swap(int* io_type, int* io_rot); // 이번 턴에 1회만 성공
int  tetris_hold_type(void);

// === board read-only / core ops ===
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

// rotate (기존 오타 함수 유지 + 정식 이름 alias)
bool tetris_try_rotatre(int* io_y, int* io_x, int type, int* io_rot, int dir);
static inline bool tetris_try_rotate(int* io_y, int* io_x, int type, int* io_rot, int dir)
{
    return tetris_try_rotatre(io_y, io_x, type, io_rot, dir);
}

void freeze_block(int y, int x, int type, int rotation);

int clear_lines(void);

#ifdef __cplusplus
}
#endif