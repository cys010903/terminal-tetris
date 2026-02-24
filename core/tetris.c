#include <stdbool.h>

#include "config.h"       // BOARD_WIDTH, BOARD_HEIGHT
#include "tetris.h"       // BLOCK_KIND, BLOCK_SIZE, EMPTY

static const int blocks[BLOCK_KIND][4][BLOCK_SIZE][BLOCK_SIZE] = {
    // 1. MINO_I (하늘색)
    {
        {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}},
        {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}
    },
    // 2. MINO_J (파란색)
    {
        {{2,0,0,0}, {2,2,2,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,2,2,0}, {0,2,0,0}, {0,2,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {2,2,2,0}, {0,0,2,0}, {0,0,0,0}},
        {{0,2,0,0}, {0,2,0,0}, {2,2,0,0}, {0,0,0,0}}
    },
    // 3. MINO_L (주황색)
    {
        {{0,0,3,0}, {3,3,3,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,3,0,0}, {0,3,0,0}, {0,3,3,0}, {0,0,0,0}},
        {{0,0,0,0}, {3,3,3,0}, {3,0,0,0}, {0,0,0,0}},
        {{3,3,0,0}, {0,3,0,0}, {0,3,0,0}, {0,0,0,0}}
    },
    // 4. MINO_O (노란색)
    {
        {{0,4,4,0}, {0,4,4,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,4,4,0}, {0,4,4,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,4,4,0}, {0,4,4,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,4,4,0}, {0,4,4,0}, {0,0,0,0}, {0,0,0,0}}
    },
    // 5. MINO_S (초록색)
    {
        {{0,5,5,0}, {5,5,0,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,5,0,0}, {0,5,5,0}, {0,0,5,0}, {0,0,0,0}},
        {{0,0,0,0}, {0,5,5,0}, {5,5,0,0}, {0,0,0,0}},
        {{5,0,0,0}, {5,5,0,0}, {0,5,0,0}, {0,0,0,0}}
    },
    // 6. MINO_T (보라색)
    {
        {{0,6,0,0}, {6,6,6,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,6,0,0}, {0,6,6,0}, {0,6,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {6,6,6,0}, {0,6,0,0}, {0,0,0,0}},
        {{0,6,0,0}, {6,6,0,0}, {0,6,0,0}, {0,0,0,0}}
    },
    // 7. MINO_Z (빨간색)
    {
        {{7,7,0,0}, {0,7,7,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,7,0}, {0,7,7,0}, {0,7,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {7,7,0,0}, {0,7,7,0}, {0,0,0,0}},
        {{0,7,0,0}, {7,7,0,0}, {7,0,0,0}, {0,0,0,0}}
    }
};

// 게임판 데이터 (0: 빈칸, 1~7: 쌓인 블록)
static  int board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

/**
 * @param n_y 이동하려는 목표 y좌표
 * @param n_x 이동하려는 목표 x좌표
 * @param type 블록 종류
 * @param rotation 회전 상태
 * @return true(충돌 있음, 이동불가), false(충돌 없음, 이동가능)
 */
int tetris_cell(int y, int x)
{
    if (y < 0 || y >= BOARD_HEIGHT) return 0;
    if (x < 0 || x >= BOARD_WIDTH)  return 0;
    return board[y][x];
}

bool check_collision(int n_y, int n_x, int type, int rotation) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            // 1. 블록 배열에서 실제 칸이 있는 부분(1~7)만 검사
            if (blocks[type][rotation][i][j] != 0) {
                int board_y = n_y + i;
                int board_x = n_x + j;

                // 2. 범위를 벗어나는지 검사 (좌측, 우측, 바닥 벽)
                if (board_x < 0 || board_x >= BOARD_WIDTH || board_y >= BOARD_HEIGHT) {
                    return true;
                }
                
                // 3. 이미 쌓여있는 블록과 겹치는지 검사
                // (board_y가 0보다 작을 때는 천장이므로 무시해도 됨)
                if (board_y >= 0 && board[board_y][board_x] != EMPTY) {
                    return true;
                }
            }
        }
    }
    // 4. 모든 칸을 검사했는데 걸리는 게 없으면 안전!
    return false;
}

void freeze_block(int y, int x, int type, int rotation) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (blocks[type][rotation][i][j] != 0) {
                // board 배열은 1~7 사이의 숫자로 블록 색상을 기억합니다.
                board[y + i][x + j] = type + 1; 
            }
        }
    }
}

int clear_lines() {
    int lines_cleared = 0;

    for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
        bool full = true;
        
        // 1. 해당 줄이 가득 찼는지 확인
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j] == EMPTY) {
                full = false;
                break;
            }
        }

        // 2. 줄이 가득 찼다면 지우고 내리기
        if (full) {
            lines_cleared++;
            
            // i번째 줄 위쪽의 모든 줄을 한 칸씩 아래로 복사
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < BOARD_WIDTH; j++) {
                    board[k][j] = board[k-1][j];
                }
            }
            
            // 맨 윗줄은 빈칸으로 채우기
            for (int j = 0; j < BOARD_WIDTH; j++) {
                board[0][j] = EMPTY;
            }

            // 줄이 내려왔으므로 현재 위치(i)를 다시 검사해야 함
            i++; 
        }
    }
    return lines_cleared;
}

int tetris_find_full_lines(int out_rows[4]) {
    int cnt = 0;
    for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
        bool full = true;
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j] == EMPTY) { full = false; break; }
        }
        if (full) {
            out_rows[cnt++] = i;
            if (cnt >= 4) break; // 한 번에 최대 4줄
        }
    }
    return cnt;
}

void tetris_remove_lines(const int rows[], int count) {
    if (count <= 0) return;

    bool remove_row[BOARD_HEIGHT] = {false};
    for (int i = 0; i < count; i++) {
        int r = rows[i];
        if (r >= 0 && r < BOARD_HEIGHT) remove_row[r] = true;
    }

    int dst = BOARD_HEIGHT - 1;
    for (int src = BOARD_HEIGHT - 1; src >= 0; src--) {
        if (remove_row[src]) continue;
        if (dst != src) {
            for (int j = 0; j < BOARD_WIDTH; j++) {
                board[dst][j] = board[src][j];
            }
        }
        dst--;
    }

    // 남은 윗부분은 EMPTY로
    for (int i = dst; i >= 0; i--) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            board[i][j] = EMPTY;
        }
    }

    
}

void tetris_clear_board(void)
{
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            board[y][x] = 0;
        }
    }
}

int tetris_mino_cell(int type, int rotation, int r, int c)
{
    if (type < 0 || type >= BLOCK_KIND) return 0;
    if (rotation < 0 || rotation >= 4)  return 0;
    if (r < 0 || r >= BLOCK_SIZE)       return 0;
    if (c < 0 || c >= BLOCK_SIZE)       return 0;

    return blocks[type][rotation][r][c];
}