#ifndef LAYOUT_H
#define LAYOUT_H

#include "config.h"

// HUD 배치값: scene가 아니라 draw에서만 사용
typedef struct {
    // "cell" 좌표(타일 단위)
    int ui_left_cell;   // NEXT 시작 x (cell)
    int next_top_cell;  // NEXT 시작 y (cell)
    int info_x_cell;    // INFO 시작 x (cell)
    int info_y_cell;    // INFO 시작 y (cell)

    // HOLD (cell)
    int hold_x_cell;
    int hold_y_cell;
} HudLayout;

void layout_build(HudLayout* out);

#endif
