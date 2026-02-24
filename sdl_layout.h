#pragma once

// SDL 전용 레이아웃 메트릭(픽셀)
// - 셀(타일) 픽셀 크기
// - 보드 원점(픽셀)

void sdl_layout_set_px(int cell_w, int cell_h, int board_ox, int board_oy);
void sdl_layout_get_cell_px(int* cell_w, int* cell_h);
void sdl_layout_get_board_origin_px(int* board_ox, int* board_oy);