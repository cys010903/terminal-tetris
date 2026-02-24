#include "layout.h"
#include "tetris.h"      // BLOCK_SIZE
#include "term_compat.h" // term_get_cell_px (SDL)

void layout_build(HudLayout* out)
{
    if (!out) return;

    // SDL: cell 단위로만 배치
    out->ui_left_cell  = BOARD_WIDTH + 4;
    out->next_top_cell = 3;

    // NEXT 폭 = BLOCK_SIZE cell, 여백 6 cell
    out->info_x_cell = out->ui_left_cell + BLOCK_SIZE + 6;
    out->info_y_cell = 2;

    // HOLD: INFO 아래
    out->hold_x_cell = out->ui_left_cell - (BLOCK_SIZE + 6) + 14;
    out->hold_y_cell = out->info_y_cell + 6;
}
