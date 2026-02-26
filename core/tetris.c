#include <stdbool.h>
#include <stdlib.h>

#include "config.h"
#include "tetris.h"

static const int blocks[BLOCK_KIND][4][BLOCK_SIZE][BLOCK_SIZE] = {
    {
        {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}},
        {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}
    },
    {
        {{2,0,0,0}, {2,2,2,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,2,2,0}, {0,2,0,0}, {0,2,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {2,2,2,0}, {0,0,2,0}, {0,0,0,0}},
        {{0,2,0,0}, {0,2,0,0}, {2,2,0,0}, {0,0,0,0}}
    },
    {
        {{0,0,3,0}, {3,3,3,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,3,0,0}, {0,3,0,0}, {0,3,3,0}, {0,0,0,0}},
        {{0,0,0,0}, {3,3,3,0}, {3,0,0,0}, {0,0,0,0}},
        {{3,3,0,0}, {0,3,0,0}, {0,3,0,0}, {0,0,0,0}}
    },
    {
        {{0,4,4,0}, {0,4,4,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,4,4,0}, {0,4,4,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,4,4,0}, {0,4,4,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,4,4,0}, {0,4,4,0}, {0,0,0,0}, {0,0,0,0}}
    },
    {
        {{0,5,5,0}, {5,5,0,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,5,0,0}, {0,5,5,0}, {0,0,5,0}, {0,0,0,0}},
        {{0,0,0,0}, {0,5,5,0}, {5,5,0,0}, {0,0,0,0}},
        {{5,0,0,0}, {5,5,0,0}, {0,5,0,0}, {0,0,0,0}}
    },
    {
        {{0,6,0,0}, {6,6,6,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,6,0,0}, {0,6,6,0}, {0,6,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {6,6,6,0}, {0,6,0,0}, {0,0,0,0}},
        {{0,6,0,0}, {6,6,0,0}, {0,6,0,0}, {0,0,0,0}}
    },
    {
        {{7,7,0,0}, {0,7,7,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,7,0}, {0,7,7,0}, {0,7,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {7,7,0,0}, {0,7,7,0}, {0,0,0,0}},
        {{0,7,0,0}, {7,7,0,0}, {7,0,0,0}, {0,0,0,0}}
    }
};

static int board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

// ==== Step2: RNG/NEXT/HOLD (Core 소유) ====
static TetrisRandomizer g_rng_mode = TETRIS_RNG_PURE;

static int g_next[TETRIS_NEXT_COUNT];
static int g_bag[BLOCK_KIND];
static int g_bag_pos = BLOCK_KIND;

static int g_hold_type = -1;
static bool g_hold_used = false;

void tetris_randomizer_set(TetrisRandomizer mode)
{
    g_rng_mode = mode;
}

static void bag_shuffle(void)
{
    for (int i = 0; i < BLOCK_KIND; i++) g_bag[i] = i;
    for (int i = BLOCK_KIND - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = g_bag[i]; g_bag[i] = g_bag[j]; g_bag[j] = tmp;
    }
    g_bag_pos = 0;
}

static void bag_reset(void) { g_bag_pos = BLOCK_KIND; }
static int bag_draw(void) { if (g_bag_pos >= BLOCK_KIND) bag_shuffle(); return g_bag[g_bag_pos++]; }

static int random_draw(void)
{
    if (g_rng_mode == TETRIS_RNG_7BAG) return bag_draw();
    return rand() % BLOCK_KIND;
}

void tetris_next_reset(void)
{
    if (g_rng_mode == TETRIS_RNG_7BAG) bag_reset();
    for (int i = 0; i < TETRIS_NEXT_COUNT; i++) g_next[i] = random_draw();
}

int tetris_next_pop(void)
{
    int t = g_next[0];
    for (int i = 0; i < TETRIS_NEXT_COUNT - 1; i++) g_next[i] = g_next[i + 1];
    g_next[TETRIS_NEXT_COUNT - 1] = random_draw();
    return t;
}

void tetris_next_peek(int out[TETRIS_NEXT_COUNT])
{
    for (int i = 0; i < TETRIS_NEXT_COUNT; i++) out[i] = g_next[i];
}

void tetris_hold_reset(void)
{
    g_hold_type = -1;
    g_hold_used = false;
}

void tetris_hold_new_turn(void)
{
    g_hold_used = false;
}

int tetris_hold_type(void)
{
    return g_hold_type;
}

bool tetris_hold_swap(int* io_type, int* io_rot)
{
    (void)io_rot; // 현재 rot는 Scene에서 0으로 리셋하므로 여기선 관여 안 함
    if (!io_type) return false;
    if (g_hold_used) return false;

    g_hold_used = true;

    if (g_hold_type < 0) {
        g_hold_type = *io_type;
        *io_type = tetris_next_pop();
        return true;
    }

    int tmp = *io_type;
    *io_type = g_hold_type;
    g_hold_type = tmp;
    return true;
}

// ==== board read-only ====
int tetris_cell(int y, int x)
{
    if (y < 0 || y >= BOARD_HEIGHT) return 0;
    if (x < 0 || x >= BOARD_WIDTH)  return 0;
    return board[y][x];
}

// ==== internal collision (Scene 금지) ====
bool check_collision(int n_y, int n_x, int type, int rotation)
{
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (blocks[type][rotation][i][j] != 0) {
                int board_y = n_y + i;
                int board_x = n_x + j;

                if (board_x < 0 || board_x >= BOARD_WIDTH || board_y >= BOARD_HEIGHT) return true;
                if (board_y >= 0 && board[board_y][board_x] != EMPTY) return true;
            }
        }
    }
    return false;
}

bool tetris_can_place(int y, int x, int type, int rot)
{
    if (type < 0 || type >= BLOCK_KIND) return false;
    rot &= 3;
    return !check_collision(y, x, type, rot);
}

bool tetris_try_move(int* io_y, int* io_x, int type, int rot, int dx, int dy)
{
    if (!io_y || !io_x) return false;
    if (type < 0 || type >= BLOCK_KIND) return false;

    int ny = *io_y + dy;
    int nx = *io_x + dx;
    rot &= 3;

    if (check_collision(ny, nx, type, rot)) return false;
    *io_y = ny;
    *io_x = nx;
    return true;
}

int tetris_ghost_y(int y, int x, int type, int rot)
{
    if (type < 0 || type >= BLOCK_KIND) return y;
    rot &= 3;

    int gy = y;
    while (!check_collision(gy + 1, x, type, rot)) gy++;
    return gy;
}

int tetris_hard_drop_y(int y, int x, int type, int rot)
{
    return tetris_ghost_y(y, x, type, rot);
}

typedef struct { int dx; int dy_up; } Kick;

static const Kick KICK_JLSTZ[8][5] = {
    {{ 0, 0},{-1, 0},{-1, 1},{ 0,-2},{-1,-2}},
    {{ 0, 0},{ 1, 0},{ 1,-1},{ 0, 2},{ 1, 2}},
    {{ 0, 0},{ 1, 0},{ 1,-1},{ 0, 2},{ 1, 2}},
    {{ 0, 0},{-1, 0},{-1, 1},{ 0,-2},{-1,-2}},
    {{ 0, 0},{ 1, 0},{ 1, 1},{ 0,-2},{ 1,-2}},
    {{ 0, 0},{-1, 0},{-1,-1},{ 0, 2},{-1, 2}},
    {{ 0, 0},{-1, 0},{-1,-1},{ 0, 2},{-1, 2}},
    {{ 0, 0},{ 1, 0},{ 1, 1},{ 0,-2},{ 1,-2}},
};

static const Kick KICK_I[8][5] = {
    {{ 0, 0},{-2, 0},{ 1, 0},{-2,-1},{ 1, 2}},
    {{ 0, 0},{ 2, 0},{-1, 0},{ 2, 1},{-1,-2}},
    {{ 0, 0},{-1, 0},{ 2, 0},{-1, 2},{ 2,-1}},
    {{ 0, 0},{ 1, 0},{-2, 0},{ 1,-2},{-2, 1}},
    {{ 0, 0},{ 2, 0},{-1, 0},{ 2, 1},{-1,-2}},
    {{ 0, 0},{-2, 0},{ 1, 0},{-2,-1},{ 1, 2}},
    {{ 0, 0},{ 1, 0},{-2, 0},{ 1,-2},{-2, 1}},
    {{ 0, 0},{-1, 0},{ 2, 0},{-1, 2},{ 2,-1}},
};

static int srs_index(int from, int to)
{
    if (from == 0 && to == 1) return 0;
    if (from == 1 && to == 0) return 1;
    if (from == 1 && to == 2) return 2;
    if (from == 2 && to == 1) return 3;
    if (from == 2 && to == 3) return 4;
    if (from == 3 && to == 2) return 5;
    if (from == 3 && to == 0) return 6;
    if (from == 0 && to == 3) return 7;
    return -1;
}

bool tetris_try_rotatre(int* io_y, int* io_x, int type, int* io_rot, int dir)
{
    if (!io_y || !io_x || !io_rot) return false;
    if (type < 0 || type >= BLOCK_KIND) return false;

    int from = (*io_rot) & 3;
    int to   = (from + (dir >= 0 ? 1 : 3)) & 3;

    if (type == 3) {
        if (!check_collision(*io_y, *io_x, type, to)) { *io_rot = to; return true; }
        return false;
    }

    const Kick (*table)[5] = (type == 0) ? KICK_I : KICK_JLSTZ;
    int idx = srs_index(from, to);
    if (idx < 0) return false;

    for (int i = 0; i < 5; i++) {
        int nx = *io_x + table[idx][i].dx;
        int ny = *io_y - table[idx][i].dy_up;
        if (!check_collision(ny, nx, type, to)) {
            *io_x = nx; *io_y = ny; *io_rot = to;
            return true;
        }
    }
    return false;
}

void freeze_block(int y, int x, int type, int rotation)
{
    for (int i = 0; i < BLOCK_SIZE; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (blocks[type][rotation][i][j] != 0) {
                board[y + i][x + j] = type + 1;
            }
        }
    }
}

int clear_lines(void)
{
    int lines_cleared = 0;
    for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
        bool full = true;
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j] == EMPTY) { full = false; break; }
        }
        if (full) {
            lines_cleared++;
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < BOARD_WIDTH; j++) board[k][j] = board[k-1][j];
            }
            for (int j = 0; j < BOARD_WIDTH; j++) board[0][j] = EMPTY;
            i++;
        }
    }
    return lines_cleared;
}

int tetris_find_full_lines(int out_rows[4])
{
    int cnt = 0;
    for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
        bool full = true;
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j] == EMPTY) { full = false; break; }
        }
        if (full) {
            out_rows[cnt++] = i;
            if (cnt >= 4) break;
        }
    }
    return cnt;
}

void tetris_remove_lines(const int rows[], int count)
{
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
            for (int j = 0; j < BOARD_WIDTH; j++) board[dst][j] = board[src][j];
        }
        dst--;
    }

    for (int i = dst; i >= 0; i--) {
        for (int j = 0; j < BOARD_WIDTH; j++) board[i][j] = EMPTY;
    }
}

void tetris_clear_board(void)
{
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) board[y][x] = 0;
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