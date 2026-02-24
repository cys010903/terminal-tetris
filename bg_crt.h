#pragma once
#include "gui.h"

typedef struct CrtBg {
    float scan_y;
    float scan_speed;
    float noise_t;
    int enabled;
} CrtBg;

void crt_bg_init(CrtBg* b);
void crt_bg_update(CrtBg* b, int dt_ms);
void crt_bg_render(Gui* gui, const CrtBg* b);
