#ifndef TERM_COMPAT_H
#define TERM_COMPAT_H

#include "gui.h"
void term_bind_gui(Gui* gui);
Gui* term_get_gui(void);
void term_get_cell_px(int* cw, int* ch);

int  check_terminal_size(void);
void render_resize_prompt(void);

#endif