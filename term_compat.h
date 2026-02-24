#ifndef TERM_COMPAT_H
#define TERM_COMPAT_H

#include "gui.h"

void term_bind_gui(Gui* gui);
Gui* term_get_gui(void);

// SDL 레이아웃(셀 크기/보드 원점) 브릿지
void term_set_layout_px(int cw, int ch, int ox, int oy);
void term_get_cell_px(int* cw, int* ch);
void term_get_board_origin_px(int* ox, int* oy);

int  check_terminal_size(void);
void render_resize_prompt(void);

#endif