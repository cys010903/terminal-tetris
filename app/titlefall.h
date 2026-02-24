#pragma once

#include "gui.h"

#ifndef TITLEFALL_MAX
#define TITLEFALL_MAX 18
#endif

typedef struct TitleFallMino {
    int type;      // 0..BLOCK_KIND-1
    int rot;       // 0..3
    float x_px;
    float y_px;
    float vy;      // px/sec
    float spin_t;  // sec accumulator
} TitleFallMino;

typedef struct TitleFall {
    TitleFallMino m[TITLEFALL_MAX];
    int count;

    int w, h;       // window px
    int cell_px;    // block cell px

    float min_vy;
    float max_vy;
    float spin_interval; // sec (0이면 회전 없음)
    int alpha;           // 0..255
    int enabled;

    unsigned int rng;
} TitleFall;

void titlefall_init(TitleFall* t, int w, int h, int cell_px);
void titlefall_resize(TitleFall* t, int w, int h);
void titlefall_set_enabled(TitleFall* t, int on);
void titlefall_update(TitleFall* t, int dt_ms);
void titlefall_render(Gui* gui, const TitleFall* t);

