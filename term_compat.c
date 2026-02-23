#include "term_compat.h"

static Gui* s_gui;

// ===== SDL layout knobs =====
static int s_cw = 28;   // 셀(블록) 픽셀 크기
static int s_ch = 28;
static int s_ox = 440;   // 보드 원점(픽셀)
static int s_oy = 56;

void term_bind_gui(Gui* gui) { s_gui = gui; }
Gui* term_get_gui(void) { return s_gui; }

void term_set_layout_px(int cw, int ch, int ox, int oy)
{
#ifdef BUILD_SDL
    if (cw > 0) s_cw = cw;
    if (ch > 0) s_ch = ch;
    s_ox = ox;
    s_oy = oy;
#else
    (void)cw; (void)ch; (void)ox; (void)oy;
#endif
}

void term_get_cell_px(int* cw, int* ch)
{
#ifdef BUILD_SDL
    if (cw) *cw = s_cw;
    if (ch) *ch = s_ch;
#else
    if (cw) *cw = 12;
    if (ch) *ch = 18;
#endif
}

void term_get_board_origin_px(int* ox, int* oy)
{
#ifdef BUILD_SDL
    if (ox) *ox = s_ox;
    if (oy) *oy = s_oy;
#else
    if (ox) *ox = 0;
    if (oy) *oy = 0;
#endif
}

#ifdef BUILD_SDL
int check_terminal_size(void) { return 1; }
void render_resize_prompt(void) { }
#endif