#include "titlefx.h"

#include "titlefall.h"
#include "term_compat.h"   // term_get_gui()
#include "gui.h"

static int s_enabled = 0;

static TitleFall s_fall;
static int s_inited = 0;
static int s_last_w = 0;
static int s_last_h = 0;

void titlefx_set_enabled(int on)
{
    s_enabled = (on != 0);
}

static void ensure_fall(Gui* gui)
{
    int w = 0, h = 0;
    gui_get_size(gui, &w, &h);

    const int cell_px = 20; // 프로젝트 셀 크기(필요하면 16~24로 조절)

    if (!s_inited) {
        titlefall_init(&s_fall, w, h, cell_px);
        titlefall_set_enabled(&s_fall, 1);
        s_inited = 1;
        s_last_w = w;
        s_last_h = h;
        return;
    }

    if (w != s_last_w || h != s_last_h) {
        titlefall_resize(&s_fall, w, h);
        s_last_w = w;
        s_last_h = h;
    }
}

void titlefx_update(int dt_ms)
{
    if (!s_enabled) return;

    Gui* gui = term_get_gui();
    if (!gui) return;

    ensure_fall(gui);
    titlefall_set_enabled(&s_fall, 1);
    titlefall_update(&s_fall, dt_ms);
}

void titlefx_render(void)
{
    if (!s_enabled) return;

    Gui* gui = term_get_gui();
    if (!gui) return;

    ensure_fall(gui);
    titlefall_set_enabled(&s_fall, 1);

    // ✅ 씬 그리기 전에 호출되면 자동으로 “뒤에 깔림”
    titlefall_render(gui, &s_fall);
}