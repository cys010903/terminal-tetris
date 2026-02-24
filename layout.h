#ifndef LAYOUT_H
#define LAYOUT_H

#include "config.h"

// HUD 배치값: scene가 아니라 draw에서만 사용
typedef struct {
#ifdef BUILD_SDL
    // "cell" 좌표(타일 단위)
    int ui_left_cell;   // NEXT 시작 x (cell)
    int next_top_cell;  // NEXT 시작 y (cell)
    int info_x_cell;    // INFO 시작 x (cell)
    int info_y_cell;    // INFO 시작 y (cell)

    // HOLD (cell)
    int hold_x_cell;
    int hold_y_cell;
#else
    // 터미널은 col/row (ncurses)
    int ui_left_col;    // NEXT 시작 x (col)
    int next_top_row;   // NEXT 시작 y (row)
    int info_x_col;     // INFO 시작 x (col)
    int info_y_row;     // INFO 시작 y (row)

    // HOLD (row/col)
    int hold_x_col;
    int hold_y_row;
#endif
} HudLayout;

void layout_build(HudLayout* out);

#endif
