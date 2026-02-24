#include "bg_crt.h"
#include <math.h>

static float clampf(float x, float a, float b)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

void crt_bg_init(CrtBg* b)
{
    if (!b) return;
    b->scan_y = -60.0f;
    b->scan_speed = 120.0f;
    b->noise_t = 0.0f;
    b->enabled = 1;
}

void crt_bg_update(CrtBg* b, int dt_ms)
{
    if (!b || !b->enabled) return;

    float dt = (dt_ms <= 0) ? 0.016f : (float)dt_ms / 1000.0f;
    b->scan_y += b->scan_speed * dt;
    b->noise_t += dt;
}

void crt_bg_render(Gui* gui, const CrtBg* b)
{
    if (!gui || !b || !b->enabled) return;

    int w = 0, h = 0;
    gui_get_size(gui, &w, &h);

    float scan_y = b->scan_y;
    if (h > 0) {
        float wrap = (float)h + 120.0f;
        while (scan_y > (float)h + 60.0f) scan_y -= wrap;
        while (scan_y < -60.0f)          scan_y += wrap;
    }

    // base
    gui_fill_rect(gui, (GuiRect){0,0,w,h}, (GuiColor){10,10,14,255});

    // subtle gradient
    for (int i = 0; i < 6; i++) {
        int y0 = (h * i) / 6;
        int y1 = (h * (i + 1)) / 6;
        int add = i * 3;
        gui_fill_rect(gui, (GuiRect){0,y0,w,y1 - y0}, (GuiColor){12 + add,12 + add,16 + add,255});
    }

    // scanlines
    GuiColor line = (GuiColor){0,0,0,12};
    for (int y = 0; y < h; y += 2) {
        gui_fill_rect(gui, (GuiRect){0,y,w,1}, line);
    }

    // moving scan band
    int band_half = 28;
    for (int dy = -band_half; dy <= band_half; dy++) {
        float y = scan_y + (float)dy;
        if (y < 0 || y >= h) continue;

        float t = 1.0f - fabsf((float)dy) / (float)band_half;
        t = clampf(t, 0.0f, 1.0f);

        int a = (int)(6 + 10 * t);;
        gui_fill_rect(gui, (GuiRect){0,(int)y,w,1},(GuiColor){220,230,255,(unsigned char)a});
    }

    // noise dots (very subtle)
    unsigned int seed = (unsigned int)(b->noise_t * 1000.0f) ^ 0xA53C9E1u;
    int dots = (w * h) / 30000;
    for (int i = 0; i < dots; i++) {
        seed = 1664525u * seed + 1013904223u;
        int x = (int)(seed % (unsigned int)w);
        seed = 1664525u * seed + 1013904223u;
        int y = (int)(seed % (unsigned int)h);
        gui_fill_rect(gui, (GuiRect){x,y,1,1}, (GuiColor){255,255,255,8});
    }

    // vignette
    int pad = (w < h ? w : h) / 18;
    if (pad < 10) pad = 10;

    gui_fill_rect(gui, (GuiRect){0,0,w,pad}, (GuiColor){0,0,0,40});
    gui_fill_rect(gui, (GuiRect){0,h - pad,w,pad}, (GuiColor){0,0,0,40});
    gui_fill_rect(gui, (GuiRect){0,0,pad,h}, (GuiColor){0,0,0,40});
    gui_fill_rect(gui, (GuiRect){w - pad,0,pad,h}, (GuiColor){0,0,0,40});
}