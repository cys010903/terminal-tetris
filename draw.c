#include <ncursesw/ncurses.h>

#include "config.h"       // BOARD_WIDTH, BOARD_HEIGHT
#include "tetris.h"       // BLOCK_KIND, BLOCK_SIZE, EMPTY
#include "tetris_data.h"  // blocks 배열
#include "draw.h"

#define COLOR_ORANGE 8

// tetris.c에 정의된 4차원 배열을 가져옵니다.
extern int blocks[BLOCK_KIND][4][BLOCK_SIZE][BLOCK_SIZE];

// ncurses 색상 초기화 (main에서 한 번만 호출하면 됩니다)
void init_colors() {
    start_color();
    
    //use_default_colors(); //디폴트 컬러로 변경 일단 기본 색상인 검정색으로
    if (can_change_color()) {
        init_color(COLOR_ORANGE, 1000, 500, 0);
    }
    // COLOR_PAIR(번호, 글자색, 배경색)
    init_pair(1, COLOR_CYAN, COLOR_BLACK);   // I
    init_pair(2, COLOR_BLUE, COLOR_BLACK);   // J
    init_pair(3, COLOR_ORANGE, COLOR_BLACK);  // L 
    init_pair(4, COLOR_YELLOW, COLOR_BLACK); // O
    init_pair(5, COLOR_GREEN, COLOR_BLACK);  // S
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);// T
    init_pair(7, COLOR_RED, COLOR_BLACK);    // Z
    init_pair(8, COLOR_WHITE, COLOR_BLACK); //WALL
}

// 특정 위치에 블록 하나를 그리는 함수
void draw_block(int y, int x, int type, int rotation) {
    attron(COLOR_PAIR(type + 1));
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (blocks[type][rotation][i][j] != 0) {
                // y+1, (x+1)*2 좌표로 벽 내부와 일치시킴
                mvaddstr(y + i + 1, (x + j + 1) * 2, "■");
            }
        }
    }
    attroff(COLOR_PAIR(type + 1));
}

// - A_DIM(흐리게) + 빈 사각형 문자로 실제 블록과 구분
void draw_ghost_block(int y, int x, int type, int rotation) {
    attron(COLOR_PAIR(type + 1));
    attron(A_DIM);
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (blocks[type][rotation][i][j] != 0) {
                mvaddstr(y + i + 1, (x + j + 1) * 2, "□");
            }
        }
    }
    attroff(A_DIM);
    attroff(COLOR_PAIR(type + 1));
}

// 게임판 전체(이미 쌓인 블록들)를 그리는 함수 (나중에 사용)
void draw_board(int board[BOARD_HEIGHT][BOARD_WIDTH]) {
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        // 1. 왼쪽 벽 (3번 페어 - 흰색)
        attron(COLOR_PAIR(8)); 
        mvaddstr(i + 1, 0, "■"); 
        attroff(COLOR_PAIR(8));

        // 2. 보드 내부
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j] != EMPTY) {
                attron(COLOR_PAIR(board[i][j]));
                mvaddstr(i + 1, (j + 1) * 2, "■");
                attroff(COLOR_PAIR(board[i][j]));
            } else {
                mvaddstr(i + 1, (j + 1) * 2, "  "); 
            }
        }

        // 3. 오른쪽 벽 (3번 페어 - 흰색)
        attron(COLOR_PAIR(8));
        mvaddstr(i + 1, (BOARD_WIDTH + 1) * 2, "■");
        attroff(COLOR_PAIR(8));
    }

    // 4. 바닥 벽 (3번 페어 - 흰색)
    attron(COLOR_PAIR(8));
    for (int j = 0; j <= BOARD_WIDTH + 1; j++) {
        mvaddstr(BOARD_HEIGHT + 1, j * 2, "■");
    }
    attroff(COLOR_PAIR(8));
}

void draw_mino_preview(int top, int left, int type, int rotation) {
    attron(COLOR_PAIR(type + 1));
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (blocks[type][rotation][i][j]) {
                mvaddstr(top + i, left + j * 2, "■");
            } else {
                mvaddstr(top + i, left + j * 2, "  "); // 잔상 제거
            }
        }
    }
    attroff(COLOR_PAIR(type + 1));
}

void draw_line_clear_anim(const int rows[], int count, int frame) {
    int left = frame;
    int right = (BOARD_WIDTH - 1) - frame;

    attron(COLOR_PAIR(8));
    attron(A_BOLD);

    for (int i = 0; i < count; i++) {
        int r = rows[i];
        if (r < 0 || r >= BOARD_HEIGHT) continue;

        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (left <= j && j <= right) {
                mvaddstr(r + 1, (j + 1) * 2, "□");
            } else {
                mvaddstr(r + 1, (j + 1) * 2, "  ");
            }
        }
    }

    attroff(A_BOLD);
    attroff(COLOR_PAIR(8));
}