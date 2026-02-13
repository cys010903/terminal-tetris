#ifndef COMMON_H
#define COMMON_H

/* 1. 표준 라이브러리 및 로캘 설정 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>      // 특수 문자(■) 출력을 위해 필수
#include <ncurses.h>

/* 2. 게임 설정 (매크로) */
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define BLOCK_KIND 7
#define BLOCK_SIZE 4

#define COLOR_ORANGE 8

/* 3. 미노 타입 및 상태 정의 */
#define EMPTY 0
typedef enum {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_EXIT
} GameState;

/* 4. 공유 데이터 (extern) */
extern int blocks[BLOCK_KIND][4][BLOCK_SIZE][BLOCK_SIZE];
extern int board[BOARD_HEIGHT][BOARD_WIDTH];

/* 5. 함수 프로토타입 */

// [draw.c] - 화면 출력 관련
void init_colors();
void draw_block(int y, int x, int type, int rotation);
void draw_board(int board[BOARD_HEIGHT][BOARD_WIDTH]);
void freeze_block(int y, int x, int type, int rotation);
void draw_title();

// [tetris.c] - 게임 로직 관련
void init_game();
bool check_collision(int n_y, int n_x, int type, int rotation);
void update_game(); // 블록 하강 로직 등
int clear_lines();

// [input.c] - 사용자 입력 관련
int handle_input();

#endif
