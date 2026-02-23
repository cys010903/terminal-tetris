#include "layout.h"
#include "tetris.h"      // BLOCK_SIZE
#include "term_compat.h" // term_get_cell_px (SDL)

#ifndef BUILD_SDL
// ncurses는 "가로 2칸"로 그리므로 col 계산이 필요
#endif

void layout_build(HudLayout* out)
{
    if (!out) return;

#ifdef BUILD_SDL
    // SDL: cell 단위로만 배치
    out->ui_left_cell  = BOARD_WIDTH + 4;
    out->next_top_cell = 3;

    // NEXT 폭 = BLOCK_SIZE cell, 여백 6 cell
    out->info_x_cell = out->ui_left_cell + BLOCK_SIZE + 6;
    out->info_y_cell = 2;

    // HOLD: INFO 아래
    out->hold_x_cell = out->ui_left_cell - (BLOCK_SIZE + 6) + 14;
    out->hold_y_cell = out->info_y_cell + 6;

#else
    // ncurses: 보드가 (x*2)로 그려지므로 UI도 col 기준
    out->ui_left_col  = (BOARD_WIDTH + 4) * 2;
    out->next_top_row = 3;

    // NEXT 폭 = BLOCK_SIZE*2 col, 여백 6 col
    out->info_x_col = out->ui_left_col + (BLOCK_SIZE * 2) + 6;
    out->info_y_row = 2;

    out->hold_x_col = out->info_x_col;
    out->hold_y_row = out->info_y_row + 6;
#endif
}
