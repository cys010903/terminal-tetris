#include "titlefall.h"


#include "tetris.h"   // BLOCK_KIND
#include <string.h>
#include <math.h>

static unsigned int rng_next(unsigned int* s)
{
    *s = (*s * 1664525u) + 1013904223u;
    return *s;
}

static float frand01(unsigned int* s)
{
    return (rng_next(s) & 0xFFFFFFu) / (float)0x1000000u;
}

static int irand(unsigned int* s, int a, int b_inclusive)
{
    unsigned int r = rng_next(s);
    int n = b_inclusive - a + 1;
    return a + (int)(r % (unsigned int)n);
}

// 표준 7개 미노 4회전 (4x4)
static const unsigned char SHAPE[7][4][4][4] = {
    // I
    {
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}
    },
    // O
    {
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}
    },
    // T
    {
        {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}
    },
    // S
    {
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
        {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}
    },
    // Z
    {
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}}
    },
    // J
    {
        {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}
    },
    // L
    {
        {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
        {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}
    }
};

static void draw_cell(Gui* gui, int x, int y, int sz, GuiColor c)
{
    gui_fill_rect(gui, (GuiRect){x, y, sz, sz}, c);
    GuiColor bd = c;
    int a = (int)c.a + 18;
    bd.a = (unsigned char)(a > 255 ? 255 : a);
    gui_draw_rect(gui, (GuiRect){x, y, sz, sz}, bd);
}

static void spawn_one(TitleFall* t, TitleFallMino* m, int spread_top)
{
    m->type = irand(&t->rng, 0, BLOCK_KIND - 1);
    m->rot  = irand(&t->rng, 0, 3);

    float u = frand01(&t->rng);
    m->vy = t->min_vy + (t->max_vy - t->min_vy) * u;

    float margin = (float)(t->cell_px * 2);
    float span = (float)t->w - margin * 2.0f;
    if (span < 10.0f) span = 10.0f;

    m->x_px = margin + frand01(&t->rng) * span;

    if (spread_top) {
        m->y_px = -frand01(&t->rng) * (float)t->h;
    } else {
        m->y_px = -(float)(t->cell_px * 8);
    }

    m->spin_t = frand01(&t->rng) * (t->spin_interval > 0.0f ? t->spin_interval : 1.0f);
}

void titlefall_init(TitleFall* t, int w, int h, int cell_px)
{
    if (!t) return;
    memset(t, 0, sizeof(*t));

    t->enabled = 1;
    t->w = w;
    t->h = h;
    t->cell_px = (cell_px > 0) ? cell_px : 18;

    t->count = (TITLEFALL_MAX < 18) ? TITLEFALL_MAX : 18;
    if (t->count < 8) t->count = 8;

    // 기본 튜닝(촌스럽지 않게)
    t->min_vy = 22.0f;
    t->max_vy = 70.0f;
    t->spin_interval = 1.2f; // 0이면 회전 OFF
    t->alpha = 32;           // 은은

    t->rng = 0xC0FFEEu;

    for (int i = 0; i < t->count; i++) {
        spawn_one(t, &t->m[i], 1);
    }
}

void titlefall_resize(TitleFall* t, int w, int h)
{
    if (!t) return;
    t->w = w;
    t->h = h;
}

void titlefall_set_enabled(TitleFall* t, int on)
{
    if (!t) return;
    t->enabled = on ? 1 : 0;
}

void titlefall_update(TitleFall* t, int dt_ms)
{
    if (!t || !t->enabled) return;

    float dt = (dt_ms <= 0) ? 0.016f : (float)dt_ms / 1000.0f;

    for (int i = 0; i < t->count; i++) {
        TitleFallMino* m = &t->m[i];
        m->y_px += m->vy * dt;

        if (t->spin_interval > 0.0f) {
            m->spin_t += dt;
            if (m->spin_t >= t->spin_interval) {
                m->spin_t = 0.0f;
                m->rot = (m->rot + 1) & 3;
            }
        }

        if (m->y_px > (float)t->h + (float)(t->cell_px * 6)) {
            spawn_one(t, m, 0);
        }
    }
}

void titlefall_render(Gui* gui, const TitleFall* t)
{
    if (!gui || !t || !t->enabled) return;

    int sz = t->cell_px;
    GuiColor c = (GuiColor){220, 230, 255, (unsigned char)t->alpha};

    for (int i = 0; i < t->count; i++) {
        const TitleFallMino* m = &t->m[i];
        int base_x = (int)m->x_px;
        int base_y = (int)m->y_px;

        int type = m->type;
        int rot  = m->rot;

        for (int yy = 0; yy < 4; yy++) {
            for (int xx = 0; xx < 4; xx++) {
                if (!SHAPE[type][rot][yy][xx]) continue;

                int px = base_x + xx * sz;
                int py = base_y + yy * sz;

                if (px + sz < 0 || px >= t->w || py + sz < 0 || py >= t->h) continue;
                draw_cell(gui, px, py, sz, c);
            }
        }
    }
}
