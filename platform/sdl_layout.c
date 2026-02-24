#include "sdl_layout.h"

// 기본값(윈도우 크기/타일 크기 바뀌면 main_sdl에서 갱신)
static int s_cw = 28;
static int s_ch = 28;
static int s_ox = 440;
static int s_oy = 56;

void sdl_layout_set_px(int cell_w, int cell_h, int board_ox, int board_oy)
{
    if (cell_w > 0) s_cw = cell_w;
    if (cell_h > 0) s_ch = cell_h;
    s_ox = board_ox;
    s_oy = board_oy;
}

void sdl_layout_get_cell_px(int* cell_w, int* cell_h)
{
    if (cell_w) *cell_w = s_cw;
    if (cell_h) *cell_h = s_ch;
}

void sdl_layout_get_board_origin_px(int* board_ox, int* board_oy)
{
    if (board_ox) *board_ox = s_ox;
    if (board_oy) *board_oy = s_oy;
}