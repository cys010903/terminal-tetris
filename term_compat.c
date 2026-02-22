#include "term_compat.h"

static Gui* s_gui;

void term_bind_gui(Gui* gui) { s_gui = gui; }
Gui* term_get_gui(void) { return s_gui; }

/* 임시값: 타일 픽셀 크기. 나중에 폰트/스케일에 맞게 조절 */
void term_get_cell_px(int* cw, int* ch) { if(cw)*cw=24; if(ch)*ch=24; }

int check_terminal_size(void) { return 1; }     // SDL에선 항상 OK 처리
void render_resize_prompt(void) { }             // SDL에선 안 씀